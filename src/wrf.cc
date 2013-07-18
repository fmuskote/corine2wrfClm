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
    : NcFile (fileName.c_str (), fileMode),
      _errorBehavior (new NcError (NcError::silent_nonfatal))
#ifdef _OPENMP
      , _lock (new omp_lock_t)
#endif
{

    NcDim* dimension = get_dim ("west_east");
    if (dimension == NULL) throw UnknownLUTypeException ();
    _iSize = dimension->size ();

    dimension = get_dim ("south_north");
    if (dimension == NULL) throw UnknownLUTypeException ();
    _jSize = dimension->size ();

    // create projection for this WRF file //
    //-------------------------------------//
    stringstream ss;
    ss << "+proj=lcc"
       << " +lat_1="    << (*get_att ("TRUELAT1")).as_double (0)
       << " +lat_2="    << (*get_att ("TRUELAT2")).as_double (0)
       << " +lat_0="    << (*get_att ("CEN_LAT")).as_double (0)
       << " +lon_0="    << (*get_att ("CEN_LON")).as_double (0)
       << " +x_0="      << (((double) iSize ())/2.0 * getDx ())
       << " +y_0="      << (((double) jSize ())/2.0 * getDy ())
       << " +ellps=WGS84 +datum=WGS84";
    _coordinateSystem->importFromProj4 (ss.str ().c_str ());

    // get parameters for affine transformation //
    //------------------------------------------//
    _padfTransform[0] = getDx ()/2.0;
    _padfTransform[1] = getDx ();
    _padfTransform[2] = 0.0;
    _padfTransform[3] = getDy ()/2.0;
    _padfTransform[4] = 0.0;
    _padfTransform[5] = getDy ();

    // calculate parameters for inverse transformation //
    //-------------------------------------------------//
    _padfTransformInverse[0] = -_padfTransform[0]/_padfTransform[1];
    _padfTransformInverse[1] = 1.0/_padfTransform[1];
    _padfTransformInverse[2] = 0.0;
    _padfTransformInverse[3] = -_padfTransform[3]/_padfTransform[5];
    _padfTransformInverse[4] = 0.0;
    _padfTransformInverse[5] = 1.0/_padfTransform[5];

#ifdef _OPENMP
    omp_init_lock (_lock.get ());
#endif
}

File::~File ()
{
#ifdef _OPENMP
    omp_destroy_lock (_lock.get ());
#endif
    close ();
}

inline size_t File::iSize () const
{
    return _iSize;
}

inline size_t File::jSize () const
{
    return _jSize;
}

double File::getDx () const
{
    return get_att ("DX")->as_double (0);
}

double File::getDy () const
{
    return get_att ("DY")->as_double (0);
}

boost::shared_ptr<NotClmFractions> File::getLandUseFraction (size_t i, size_t j)
{
    // check indizes against domain size //
    //-----------------------------------//
    if (i > iSize () or j > jSize ())
        throw OutOfDomainException ();

    // allocate output temporary array //
    //---------------------------------//
    NcDim* landCatDim = get_dim ("land_cat_stag");
    if (landCatDim == NULL) throw UnknownLUTypeException ();
    size_t landCatStag = landCatDim->size ();
    boost::scoped_array<float> indexRate (new float[landCatStag]);

    // read from NetCDF //
    //------------------//
    NcVar* variable = get_var ("LANDUSEF");
    long offset[4] = {0, 0, (long) j, (long) i};
    long count[4] = {1, (long) landCatStag, 1, 1};
#ifdef _OPENMP
    lock ();
#endif
    variable->set_cur (offset);
    variable->get (indexRate.get (), count);
#ifdef _OPENMP
    unlock ();
#endif

    // create resulting vector //
    //-------------------------//
    boost::shared_ptr<NotClmFractions> result;
    if (isModisLUType ())     result = boost::shared_ptr<NotClmFractions> (new modis::ModisFractions);
    else if (isUsgsLUType ()) result = boost::shared_ptr<NotClmFractions> (new usgs::UsgsFractions);
    else throw std::exception ();

    // copy to result //
    //----------------//
    for (size_t i = 0; i < landCatStag; i++)
        result->set (i, indexRate[i]);

    return result;
}

void File::writeClmPftTypeFractions (size_t i, size_t j, const clm::ClmFractions& fractions)
{
    for (size_t type = 0; type < clm::typeCount - 1; type++)
    {
        stringstream stream;
        stream << clmPFTtypeFractionName;
        stream.fill ('0');
        stream.width (2);
        stream << type;
        string varName = stream.str ();

        long offset[3] = {0, (long) j, (long) i};
        long counts[3] = {1, 1, 1};

#ifdef _OPENMP
        lock ();
#endif

        NcVar* variable = get_var (varName.c_str ());
        if (!variable)
        {
            boost::scoped_array<const NcDim*> dims (
                    new const NcDim*[3]);
            dims[0] = get_dim ("Time");
            dims[1] = get_dim ("south_north");
            dims[2] = get_dim ("west_east");

            variable = add_var (varName.c_str (), ncFloat, 3, dims.get ());

            variable->add_att ("FieldType", 104);
            variable->add_att ("MemoryOrder", "XY");
            variable->add_att ("units", "category");
            variable->add_att ("description", "CLM plant functional types fractions");
            variable->add_att ("stagger", "M");
            variable->add_att ("sr_x", "1");
            variable->add_att ("sr_y", "1");
        }

        variable->set_cur (offset);
        variable->put (&fractions[type], counts);

#ifdef _OPENMP
        unlock ();
#endif

    }
}

bool File::isModisLUType () const
{
    char* luType = get_att ("MMINLU")->as_string (0);
    NcDim* landCatDim = get_dim ("land_cat_stag");
    if (landCatDim == NULL) throw UnknownLUTypeException ();
    bool result = false;
    if (    strcmp (luType, "MODIFIED_IGBP_MODIS_NOAH") == 0
        and landCatDim->size () == (long) modis::typeCount
        and get_att ("NUM_LAND_CAT")->as_int (0) == (long) modis::typeCount)
        result = true;

    delete[] luType;
    return result;
}

bool File::isUsgsLUType () const
{
    char* luType = get_att ("MMINLU")->as_string (0);
    NcDim* landCatDim = get_dim ("land_cat_stag");
    if (landCatDim == NULL) throw UnknownLUTypeException ();
    bool result = false;
    if (    strcmp (luType, "USGS") == 0
        and landCatDim->size () == (long) usgs::typeCount
        and get_att ("NUM_LAND_CAT")->as_int (0) == (long) usgs::typeCount)
        result = true;

    delete[] luType;
    return result;
}

boost::multi_array<float, 2> File::getClmType (size_t type)
{
    boost::array<long, 4> offset = {{0, (long) type, 0, 0}};
    boost::array<long, 4> count = {{1, 1, (long) jSize (), (long) iSize ()}};

    boost::multi_array<float, 3> data = read<float, 3> ("clm_landuse_fraction", offset, count);
    return data[boost::indices[0][boost::multi_array_types::index_range(0, jSize ())][boost::multi_array_types::index_range (0, iSize ())]];
}

void File::createMosaic (File& highResFile)
{

    float dx = getDx ();
    float dy = getDy ();
    float dxHigh = highResFile.getDx ();
    float dyHigh = highResFile.getDy ();

    if (fabs (fmod (dx, dxHigh)) > 0.0005 or fabs (fmod (dy, dyHigh)) > 0.0005)
        throw WrongMosaicGeometryException ();

    size_t dxFac = (size_t)(dx/dxHigh + 0.5);
    size_t dyFac = (size_t)(dy/dyHigh + 0.5);

    if (dxFac != dyFac)
        throw WrongMosaicGeometryException ();

    if (   iSize ()*dxFac != highResFile.iSize ()
        or jSize ()*dyFac != highResFile.jSize ())
        throw WrongMosaicGeometryException ();

    size_t mosaicCellCount = dxFac*dyFac;

    if (get_dim ("mosaic_cells")->size () != (int)mosaicCellCount)
        throw WrongMosaicGeometryException ();

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
            highResFile.read<float, 2> (highResVarName.str ());

        boost::multi_array<float, 3> data =
            mosaicArray (highResData, mosaicCellCount, dxFac, dyFac);

        stringstream stream;
        stream << "CLM_LANDUSE_FRACTION_MOSAIC_";
        stream.fill ('0');
        stream.width (2);
        stream << type;
        write (stream.str (), data);

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
void File::lock ()
{
    omp_set_lock (_lock.get ());
}

void File::unlock ()
{
    omp_unset_lock (_lock.get ());
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
#ifdef _OPENMP
        lock ();
#endif
    NcVar* variable = get_var (varName.c_str ());
    if (!variable) {
        boost::scoped_array<const NcDim*> dims (
                new const NcDim*[3]);
        dims[0] = get_dim ("Time");
        dims[1] = get_dim ("south_north");
        dims[2] = get_dim ("west_east");

        variable = add_var (varName.c_str (), ncFloat, 3, dims.get ());
        if (!variable)
            throw VariableNotExistException ();

        variable->add_att ("FieldType", 104);
        variable->add_att ("MemoryOrder", "XY");
        variable->add_att ("units", "");
        variable->add_att ("description", varName.c_str ());
        variable->add_att ("stagger", "M");
        variable->add_att ("sr_x", "1");
        variable->add_att ("sr_y", "1");
    }
#ifdef _OPENMP
        unlock ();
#endif

    long offset[3] = {0, (long) j, (long) i};
    long counts[3] = {1, 1, 1};

#ifdef _OPENMP
    lock ();
#endif
    variable->set_cur (offset);
    variable->put (&value, counts);
#ifdef _OPENMP
    unlock ();
#endif
}

boost::multi_array<float, 3> File::mosaicArray (
        const boost::multi_array<float, 2>& highResData,
        size_t mosaicCellCount, size_t dxFac, size_t dyFac) const
{

    boost::multi_array<float, 3> result (boost::extents[mosaicCellCount][jSize ()][iSize ()]);

    for (size_t i = 0; i < iSize (); i++)
        for (size_t j = 0; j < jSize (); j++)
            for (size_t im = 0; im < dxFac; im++)
                for (size_t jm = 0; jm < dyFac; jm++) {
                    size_t m = im*dyFac + jm;
                    result[m][j][i] = highResData[j*dyFac + jm][i*dxFac + im];
                }
    return result;
}
    
