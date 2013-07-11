#include <sstream>
#include <iomanip>
#include <gdal_alg.h>
#include <string>
#include <cstring>
#include <exception>
#include <boost/scoped_ptr.hpp>
#include "wrfFile.h"
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

const std::string WrfFile::clmPFTtypeDimensionName = "clm_pfttypes";
const std::string WrfFile::clmPFTtypeFractionName = "clm_landuse_fraction_";

WrfFile::WrfFile (const string fileName, FileMode fileMode) : NcFile (fileName.c_str (), fileMode) {

    errorBehavior = new NcError (NcError::silent_nonfatal);

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
    _lock = new omp_lock_t;
    omp_init_lock (_lock);
#endif
}
WrfFile::~WrfFile () {
#ifdef _OPENMP
    omp_destroy_lock (_lock);
    delete _lock;
#endif
    close ();
    delete errorBehavior;
}
inline const size_t WrfFile::iSize () const {
    return _iSize;
}
inline const size_t WrfFile::jSize () const {
    return _jSize;
}
const double WrfFile::getDx () const {
    return get_att ("DX")->as_double (0);
}
const double WrfFile::getDy () const {
    return get_att ("DY")->as_double (0);
}
boost::shared_ptr<NotClmFractions> WrfFile::getLandUseFraction (const size_t i, const size_t j) {
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
void WrfFile::writeClmPftTypeFractions (const size_t& i, const size_t& j, const clm::ClmFractions& fractions) {
    for (size_t type = 0; type < clm::typeCount - 1; type++) {
        stringstream stream;
        stream << clmPFTtypeFractionName;
        stream.fill ('0');
        stream.width (2);
        stream << type;
        string varName = stream.str ();

        // get variable //
        //--------------//
#ifdef _OPENMP
        lock ();
#endif
        NcVar* variable = get_var (varName.c_str ());
        if (!variable) {
            const NcDim** dims = new const NcDim*[3];
            dims[0] = get_dim ("Time");
            dims[1] = get_dim ("south_north");
            dims[2] = get_dim ("west_east");

            variable = add_var (varName.c_str (), ncFloat, 3, dims);
            delete[] dims;

            variable->add_att ("FieldType", 104);
            variable->add_att ("MemoryOrder", "XY");
            variable->add_att ("units", "category");
            variable->add_att ("description", "CLM plant functional types fractions");
            variable->add_att ("stagger", "M");
            variable->add_att ("sr_x", "1");
            variable->add_att ("sr_y", "1");
        }
#ifdef _OPENMP
        unlock ();
#endif

        boost::scoped_ptr<double> tmp (new double);
        *tmp = fractions[type];

        long offset[3] = {0, (long) j, (long) i};
        long counts[3] = {1, 1, 1};
#ifdef _OPENMP
        lock ();
#endif
        variable->set_cur (offset);
        variable->put (tmp.get (), counts);
#ifdef _OPENMP
        unlock ();
#endif

    }
}
bool WrfFile::isModisLUType () const {
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
bool WrfFile::isUsgsLUType () const {
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
float* WrfFile::getClmType (const size_t type) {
    NcVar* variable = get_var ("clm_landuse_fraction");
    long offset[4] = {0, (long) type, 0, 0};
    long count[4] = {1, 1, (long) jSize (), (long) iSize ()};
    float* data = new float[iSize ()*jSize ()];

#ifdef _OPENMP
    lock ();
#endif
    variable->set_cur (offset);
    variable->get (data, count);
#ifdef _OPENMP
    unlock ();
#endif


    float* result = new float[iSize ()*jSize ()];

    for (size_t i = 0; i < iSize (); i++)
        for (size_t j = 0; j < jSize (); j++)
            result[i+j*iSize ()] = data[i+j*iSize ()];

    delete[] data;
    return result;
}
void WrfFile::createMosaic (WrfFile* highResFile) {

    float dx = getDx ();
    float dy = getDy ();
    float dxHigh = highResFile->getDx ();
    float dyHigh = highResFile->getDy ();

    if (fabs (fmod (dx, dxHigh)) > 0.0005 or fabs (fmod (dy, dyHigh)) > 0.0005)
        throw WrongMosaicGeometryException ();

    size_t dxFac = (size_t)(dx/dxHigh + 0.5);
    size_t dyFac = (size_t)(dy/dyHigh + 0.5);

    if (dxFac != dyFac)
        throw WrongMosaicGeometryException ();

    if (   iSize ()*dxFac != highResFile->iSize ()
        or jSize ()*dyFac != highResFile->jSize ())
        throw WrongMosaicGeometryException ();

    size_t mosaicCellCount = dxFac*dyFac;

    if (get_dim ("mosaic_cells")->size () != (int)mosaicCellCount)
        throw WrongMosaicGeometryException ();

#pragma omp parallel for
    for (size_t type = 0; type < clm::typeCount - 1; type++) {

        stringstream highResVarName;
        highResVarName << clmPFTtypeFractionName;
        highResVarName.fill ('0');
        highResVarName.width (2);
        highResVarName << type;

        float* highResData = new float[highResFile->jSize ()*highResFile->iSize ()];
        highResFile->read2D (highResVarName.str (), highResData);

        float* data = new float[mosaicCellCount*jSize ()*iSize ()];
        mosaicArray (highResData, highResFile->iSize (), highResFile->jSize (),
                data, mosaicCellCount, dxFac, dyFac);

        stringstream stream;
        stream << "CLM_LANDUSE_FRACTION_MOSAIC_";
        stream.fill ('0');
        stream.width (2);
        stream << type;
        write3D (stream.str (), data, mosaicCellCount);

        delete[] data;
        delete[] highResData;
    }

    float* highResData = new float[highResFile->jSize ()*highResFile->jSize ()];
    float* data = new float[mosaicCellCount*jSize ()*iSize ()];

    highResFile->read2D ("waterFraction", highResData);
    mosaicArray (highResData, highResFile->iSize (), highResFile->jSize (),
            data, mosaicCellCount, dxFac, dyFac);
    write3D ("WATERFRACTION_MOSAIC", data, mosaicCellCount);

    highResFile->read2D ("urbanFraction", highResData);
    mosaicArray (highResData, highResFile->iSize (), highResFile->jSize (),
            data, mosaicCellCount, dxFac, dyFac);
    write3D ("URBANFRACTION_MOSAIC", data, mosaicCellCount);

    highResFile->read2D ("glacierFraction", highResData);
    mosaicArray (highResData, highResFile->iSize (), highResFile->jSize (),
            data, mosaicCellCount, dxFac, dyFac);
    write3D ("GLACIERFRACTION_MOSAIC", data, mosaicCellCount);

    highResFile->read2D ("wetlandFraction", highResData);
    mosaicArray (highResData, highResFile->iSize (), highResFile->jSize (),
            data, mosaicCellCount, dxFac, dyFac);
    write3D ("WETLANDFRACTION_MOSAIC", data, mosaicCellCount);

    delete[] data;
    delete[] highResData;
    
}

#ifdef _OPENMP
void WrfFile::lock () {
    omp_set_lock (_lock);
}
void WrfFile::unlock () {
    omp_unset_lock (_lock);
}
#endif

void WrfFile::writeWaterFraction (const size_t& i, const size_t& j, const double& fraction) {
    write0Dto2D ("waterFraction", i, j, fraction);
}
void WrfFile::writeUrbanFraction (const size_t& i, const size_t& j, const double& fraction) {
    write0Dto2D ("urbanFraction", i, j, fraction);
}
void WrfFile::writeGlacierFraction (const size_t& i, const size_t& j, const double& fraction) {
    write0Dto2D ("glacierFraction", i, j, fraction);
}
void WrfFile::writeWetlandFraction (const size_t& i, const size_t& j, const double& fraction) {
    write0Dto2D ("wetlandFraction", i, j, fraction);
}
void WrfFile::write0Dto2D (const string& varName, const size_t& i, const size_t& j, const double& value) {
#ifdef _OPENMP
        lock ();
#endif
    NcVar* variable = get_var (varName.c_str ());
    if (!variable) {
        const NcDim** dims = new const NcDim*[3];
        dims[0] = get_dim ("Time");
        dims[1] = get_dim ("south_north");
        dims[2] = get_dim ("west_east");

        variable = add_var (varName.c_str (), ncFloat, 3, dims);
        if (!variable)
            throw VariableNotExistExeption ();
        delete[] dims;

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
void WrfFile::write3D (const string varName, const float* data, const size_t zCount) {
    NcVar* variable = get_var (varName.c_str ());
    if (!variable)
        throw VariableNotExistExeption ();

    long offset[4] = {0, 0, 0, 0};
    long count[4] = {1, (long) zCount, (long) jSize (), (long) iSize ()};

#ifdef _OPENMP
    lock ();
#endif
    variable->set_cur (offset);
    variable->put (data, count);
#ifdef _OPENMP
    unlock ();
#endif
}
void WrfFile::read2D (const string varName, float* data) {
    NcVar* variable = get_var (varName.c_str ());
    if (!variable)
        throw VariableNotExistExeption ();

    long offset[3] = {0, 0, 0};
    long count[3] = {1, (long) jSize (), (long) iSize ()};

#ifdef _OPENMP
    lock ();
#endif
    variable->set_cur (offset);
    variable->get (data, count);
#ifdef _OPENMP
    unlock ();
#endif
}
void WrfFile::mosaicArray (const float* highResData, const size_t highResISize,
        const size_t highResJSize, float* data, const size_t mosaicCellCount,
        const size_t dxFac, const size_t dyFac) {

    const float** highResData2D = new const float*[highResJSize];
    for (size_t j = 0; j < highResJSize; j++)
        highResData2D[j] = highResData + j*highResISize;

    float*** data3D = new float**[mosaicCellCount];
    for (size_t m = 0; m < mosaicCellCount; m++) {
        data3D[m] = new float*[jSize ()];
        for (size_t j = 0; j < jSize (); j++)
            data3D[m][j] = data + (m*jSize () + j)*iSize ();
    }

    for (size_t i = 0; i < iSize (); i++)
        for (size_t j = 0; j < jSize (); j++)
            for (size_t im = 0; im < dxFac; im++)
                for (size_t jm = 0; jm < dyFac; jm++) {
                    size_t m = im*dyFac + jm;
                    data3D[m][j][i] = highResData2D[j*dyFac + jm][i*dxFac + im];
                }
        
    for (size_t m = 0; m < mosaicCellCount; m++)
        delete[] data3D[m];
    delete[] data3D;
    delete[] highResData2D;
}
    
