#ifndef WRFFILE_H
#define WRFFILE_H

#include <netcdfcpp.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "notClmFractions.h"
#include "geoRaster.h"

#ifdef _OPENMP
#include <omp.h>
#endif

class WrfFile : public NcFile, public GeoRaster
{
  private:
    static const double _rEarth = 6370000.0;
    static const std::string clmPFTtypeDimensionName;
    static const std::string clmPFTtypeFractionName;

#ifdef _OPENMP
    omp_lock_t * _lock;
#endif

    size_t _iSize;
    size_t _jSize;
    NcError* errorBehavior;

    const double getDx () const;
    const double getDy () const;
    void write0Dto2D (const std::string&, const size_t&, const size_t&, const double&);
    void write3D (const std::string, const float*, const size_t);
    void read2D (const std::string, float*);

    void mosaicArray (const float*, const size_t, const size_t, float*,
            const size_t, const size_t, const size_t);

  public:
    WrfFile (const std::string, FileMode = ReadOnly);
    ~WrfFile ();
    const size_t iSize () const;
    const size_t jSize () const;
    boost::shared_ptr<NotClmFractions> getLandUseFraction (const size_t, const size_t);
    void writeClmPftTypeFractions (const size_t&, const size_t&, const clm::ClmFractions&);
    bool isModisLUType () const;
    bool isUsgsLUType () const;
    float* getClmType (const size_t);
    void createMosaic (WrfFile*);
#ifdef _OPENMP
    void lock ();
    void unlock ();
#endif
    void writeWaterFraction (const size_t&, const size_t&, const double&);
    void writeUrbanFraction (const size_t&, const size_t&, const double&);
    void writeGlacierFraction (const size_t&, const size_t&, const double&);
    void writeWetlandFraction (const size_t&, const size_t&, const double&);
};

class VariableNotExistExeption {};
class NotUsgsLanduseExeption {};
class WrongDimensionSizeException {};
class UnknownLUTypeException {};
class WrongMosaicGeometryException {};

#endif
