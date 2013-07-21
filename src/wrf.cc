#include <sstream>
#include <iomanip>
#include <gdal_alg.h>
#include <string>
#include <cstring>
#include <exception>
#include "wrf.h"
#include "clm.h"
#include "modis.h"
#include "usgs.h"

#ifdef _OPENMP
#include <omp.h>
#endif

using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::setfill;
using std::setw;
using std::uppercase;

using namespace wrf;

File::File (string fileName, FileMode fileMode)
    : NcFile (fileName.c_str(), fileMode)
#ifdef _OPENMP
      , lock_ (new omp_lock_t)
#endif
{

    iSize_ = getDim ("west_east").getSize();
    jSize_ = getDim ("south_north").getSize();

    // create projection for this WRF file //
    //-------------------------------------//
    double truelat1;
    getAtt ("TRUELAT1").getValues (&truelat1);
    double truelat2;
    getAtt ("TRUELAT2").getValues (&truelat2);
    double cen_lat;
    getAtt ("CEN_LAT").getValues (&cen_lat);
    double cen_lon;
    getAtt ("CEN_LON").getValues (&cen_lon);

    stringstream ss;
    ss << "+proj=lcc"
       << " +lat_1="    << truelat1
       << " +lat_2="    << truelat2
       << " +lat_0="    << cen_lat
       << " +lon_0="    << cen_lon
       << " +x_0="      << static_cast<double> (iSize()) / 2.0 * getDx()
       << " +y_0="      << static_cast<double> (jSize()) / 2.0 * getDy()
       << " +ellps=WGS84 +datum=WGS84";
    coordinateSystem_->importFromProj4 (ss.str().c_str());

    // get parameters for affine transformation //
    //------------------------------------------//
    padfTransform_[0] = getDx()/2.0;
    padfTransform_[1] = getDx();
    padfTransform_[2] = 0.0;
    padfTransform_[3] = getDy()/2.0;
    padfTransform_[4] = 0.0;
    padfTransform_[5] = getDy();

    // calculate parameters for inverse transformation //
    //-------------------------------------------------//
    padfTransformInverse_[0] = -padfTransform_[0]/padfTransform_[1];
    padfTransformInverse_[1] = 1.0/padfTransform_[1];
    padfTransformInverse_[2] = 0.0;
    padfTransformInverse_[3] = -padfTransform_[3]/padfTransform_[5];
    padfTransformInverse_[4] = 0.0;
    padfTransformInverse_[5] = 1.0/padfTransform_[5];

#ifdef _OPENMP
    omp_init_lock (lock_.get());
#endif
}

File::~File ()
{
#ifdef _OPENMP
    omp_destroy_lock (lock_.get ());
#endif
}

double File::getDx() const
{
    double result;
    getAtt ("DX").getValues (&result);
    return result;
}

double File::getDy() const
{
    double result;
    getAtt ("DY").getValues (&result);
    return result;
}

boost::shared_ptr<NotClmFractions> File::getLandUseFraction (size_t i, size_t j)
{
    // check indizes against domain size //
    //-----------------------------------//
    if (i > iSize() or j > jSize())
        throw OutOfDomainException();

    // allocate output temporary array //
    //---------------------------------//
    netCDF::NcDim landCatDim = getDim ("land_cat_stag");

    size_t landCatStag = landCatDim.getSize ();
    boost::scoped_array<float> indexRate (new float[landCatStag]);

    // read from NetCDF //
    //------------------//
    netCDF::NcVar variable = getVar ("LANDUSEF");
    std::vector<size_t> offset;
    offset.push_back (0);
    offset.push_back (0);
    offset.push_back (j);
    offset.push_back (i);
    std::vector<size_t> count;
    count.push_back (1);
    count.push_back (landCatStag);
    count.push_back (1);
    count.push_back (1);

#ifdef _OPENMP
    lock();
#endif

    variable.getVar (offset, count, indexRate.get());

#ifdef _OPENMP
    unlock();
#endif

    // create resulting vector //
    //-------------------------//
    boost::shared_ptr<NotClmFractions> result;
    if (isModisLUType())     result = boost::shared_ptr<NotClmFractions> (new modis::ModisFractions);
    else if (isUsgsLUType()) result = boost::shared_ptr<NotClmFractions> (new usgs::UsgsFractions);
    else throw UnknownLUTypeException();

    // copy to result //
    //----------------//
    for (size_t i = 0; i < landCatStag; ++i)
        result->set (i, indexRate[i]);

    return result;
}

void File::writeClmPftTypeFractions(
        size_t i, size_t j, const clm::ClmFractions& fractions)
{
    for (size_t type = 0; type < clm::typeCount - 1; type++)
    {
        stringstream stream;
        stream << clmPFTtypeFractionName;
        stream.fill ('0');
        stream.width (2);
        stream << type;
        string varName = stream.str();

        std::vector<size_t> offset;
        offset.push_back (0);
        offset.push_back (j);
        offset.push_back (i);
        std::vector<size_t> count;
        count.push_back (1);
        count.push_back (1);
        count.push_back (1);

#ifdef _OPENMP
        lock();
#endif

        netCDF::NcVar variable = getVar (varName);
        if (variable.isNull())
        {
            std::vector<netCDF::NcDim> dims;
            dims.push_back (getDim ("Time"));
            dims.push_back (getDim ("south_north"));
            dims.push_back (getDim ("west_east"));

            variable = addVar (varName, netCDF::NcType::nc_FLOAT, dims);

            variable.putAtt ("FieldType", netCDF::NcType::nc_INT, 104);
            variable.putAtt ("MemoryOrder", "XY");
            variable.putAtt ("units", "category");
            variable.putAtt ("description", "CLM plant functional types fractions");
            variable.putAtt ("stagger", "M");
            variable.putAtt ("sr_x", "1");
            variable.putAtt ("sr_y", "1");
        }

        variable.putVar (offset, count, &fractions[type]);

#ifdef _OPENMP
        unlock();
#endif

    }
}

bool File::isModisLUType() const
{
    std::string luType;
    getAtt ("MMINLU").getValues (luType);
    int num_land_cat;
    getAtt ("NUM_LAND_CAT").getValues (&num_land_cat);

    netCDF::NcDim landCatDim = getDim ("land_cat_stag");

    bool result = false;
    if (    luType == "MODIFIED_IGBP_MODIS_NOAH"
        and landCatDim.getSize() == modis::typeCount
        and num_land_cat == static_cast<int> (modis::typeCount))
        result = true;

    return result;
}

bool File::isUsgsLUType () const
{
    std::string luType;
    getAtt ("MMINLU").getValues (luType);
    int num_land_cat;
    getAtt ("NUM_LAND_CAT").getValues (&num_land_cat);

    netCDF::NcDim landCatDim = getDim ("land_cat_stag");

    bool result = false;
    if (    luType == "USGS"
        and landCatDim.getSize () == usgs::typeCount
        and num_land_cat == static_cast<int> (usgs::typeCount))
        result = true;

    return result;
}

boost::multi_array<float, 2> File::getClmType (size_t type)
{
    std::vector<size_t> offset;
    offset.push_back (0);
    offset.push_back (type);
    offset.push_back (0);
    offset.push_back (0);
    std::vector<size_t> count;
    count.push_back (1);
    count.push_back (1);
    count.push_back (jSize ());
    count.push_back (iSize ());

    boost::multi_array<float, 3> data = read<float, 3> ("clm_landuse_fraction", offset, count);
    return data[boost::indices[0][boost::multi_array_types::index_range(0, jSize())][boost::multi_array_types::index_range (0, iSize())]];
}

void File::createMosaic (File& highResFile)
{

    float dx = getDx();
    float dy = getDy();
    float dxHigh = highResFile.getDx();
    float dyHigh = highResFile.getDy();

    if (fabs (fmod (dx, dxHigh)) > 0.0005 or fabs (fmod (dy, dyHigh)) > 0.0005)
        throw WrongMosaicGeometryException ();

    size_t dxFac = static_cast<size_t> (dx / dxHigh + 0.5);
    size_t dyFac = static_cast<size_t> (dy / dyHigh + 0.5);

    if (dxFac != dyFac)
        throw WrongMosaicGeometryException();

    if (   iSize() * dxFac != highResFile.iSize()
        or jSize() * dyFac != highResFile.jSize())
        throw WrongMosaicGeometryException();

    size_t mosaicCellCount = dxFac * dyFac;

    if (getDim ("mosaic_cells").getSize() != mosaicCellCount)
        throw WrongMosaicGeometryException();

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (size_t type = 0; type < clm::typeCount - 1; type++)
    {
        stringstream highResVarName;
        highResVarName << clmPFTtypeFractionName;
        highResVarName.fill ('0');
        highResVarName.width (2);
        highResVarName << type;

        boost::multi_array<float, 2> highResData =
            highResFile.read<float, 2> (highResVarName.str());

        boost::multi_array<float, 3> data =
            mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);

        stringstream stream;
        stream << "CLM_LANDUSE_FRACTION_MOSAIC_";
        stream.fill ('0');
        stream.width (2);
        stream << type;
        write (stream.str(), data);

    }

    boost::multi_array<float, 2> highResData;
    boost::multi_array<float, 3> data;

    highResData = highResFile.read<float, 2> ("waterFraction");
    data = mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);
    write ("WATERFRACTION_MOSAIC", data);

    highResData = highResFile.read<float, 2> ("urbanFraction");
    data = mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);
    write ("URBANFRACTION_MOSAIC", data);

    highResData = highResFile.read<float, 2> ("glacierFraction");
    data = mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);
    write ("GLACIERFRACTION_MOSAIC", data);

    highResData = highResFile.read<float, 2> ("wetlandFraction");
    data = mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);
    write ("WETLANDFRACTION_MOSAIC", data);

}

#ifdef _OPENMP
void File::lock()
{
    omp_set_lock (lock_.get());
}

void File::unlock()
{
    omp_unset_lock (lock_.get());
}
#endif

void File::writeWaterFraction (size_t i, size_t j, double fraction)
{
    write0Dto2D ("waterFraction", i, j, fraction);
}

void File::writeUrbanFraction (size_t i, size_t j, double fraction)
{
    write0Dto2D ("urbanFraction", i, j, fraction);
}

void File::writeGlacierFraction (size_t i, size_t j, double fraction)
{
    write0Dto2D ("glacierFraction", i, j, fraction);
}

void File::writeWetlandFraction (size_t i, size_t j, double fraction)
{
    write0Dto2D ("wetlandFraction", i, j, fraction);
}

void File::write0Dto2D (string varName, size_t i, size_t j, double value)
{
    std::vector<size_t> offset;
    offset.push_back (0);
    offset.push_back (j);
    offset.push_back (i);
    std::vector<size_t> count;
    count.push_back (1);
    count.push_back (1);
    count.push_back (1);

#ifdef _OPENMP
    lock();
#endif

    netCDF::NcVar variable = getVar (varName);
    if (variable.isNull())
    {
        std::vector<netCDF::NcDim> dims;
        dims.push_back (getDim ("Time"));
        dims.push_back (getDim ("south_north"));
        dims.push_back (getDim ("west_east"));

        variable = addVar (varName, netCDF::NcType::nc_FLOAT, dims);

        variable.putAtt ("FieldType", netCDF::NcType::nc_INT, 104);
        variable.putAtt ("MemoryOrder", "XY");
        variable.putAtt ("units", "");
        variable.putAtt ("description", varName);
        variable.putAtt ("stagger", "M");
        variable.putAtt ("sr_x", "1");
        variable.putAtt ("sr_y", "1");
    }

    variable.putVar (offset, count, &value);

#ifdef _OPENMP
    unlock();
#endif
}

boost::multi_array<float, 3> File::mosaicArray(
        const boost::multi_array<float, 2>& highResData,
        size_t mosaicCellCount, size_t dxFac, size_t dyFac) const
{

    boost::multi_array<float, 3> result (boost::extents[mosaicCellCount][jSize()][iSize()]);

    for (size_t i = 0; i < iSize(); i++)
        for (size_t j = 0; j < jSize(); j++)
            for (size_t im = 0; im < dxFac; im++)
                for (size_t jm = 0; jm < dyFac; jm++) {
                    size_t m = im * dyFac + jm;
                    result[m][j][i] = highResData[j * dyFac + jm][i * dxFac + im];
                }
    return result;
}
    
