#ifndef WRFFILE_H
#define WRFFILE_H

#include <netcdf>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <vector>

#include "notClmFractions.h"
#include "geoRaster.h"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace wrf
{

    const std::string clmPFTtypeDimensionName = "clm_pfttypes";
    const std::string clmPFTtypeFractionName = "clm_landuse_fraction_";

    class VariableNotExistException {};
    class NotUsgsLanduseException {};
    class WrongDimensionSizeException {};
    class UnknownLUTypeException {};
    class WrongMosaicGeometryException {};

    class File : public netCDF::NcFile, public GeoRaster
    {
      private:
        size_t iSize_;
        size_t jSize_;

#ifdef _OPENMP
        boost::scoped_ptr<omp_lock_t> lock_;
#endif

        double getDx () const;
        double getDy () const;
        void write0Dto2D (std::string, size_t, size_t, double);

        boost::multi_array<float, 3> mosaicArray (
                const boost::multi_array<float, 2>&,
                size_t, size_t, size_t) const;

      public:
        File (std::string, FileMode = netCDF::NcFile::read);
        ~File();
        inline size_t iSize() const {return iSize_;}
        inline size_t jSize() const {return jSize_;}
        boost::shared_ptr<NotClmFractions> getLandUseFraction (size_t, size_t);
        void writeClmPftTypeFractions (size_t, size_t, const clm::ClmFractions&);
        bool isModisLUType() const;
        bool isUsgsLUType() const;
        boost::multi_array<float, 2> getClmType (size_t);
        void createMosaic (wrf::File&);
#ifdef _OPENMP
        void lock();
        void unlock();
#endif
        void writeWaterFraction (size_t, size_t, double);
        void writeUrbanFraction (size_t, size_t, double);
        void writeGlacierFraction (size_t, size_t, double);
        void writeWetlandFraction (size_t, size_t, double);

        template<typename T, size_t D>
        void write(
                std::string varName,
                const boost::multi_array<T, D>& data);
        template<typename T, size_t D>
        void write(
                std::string varName,
                const boost::multi_array<T, D>& data,
                const std::vector<size_t>& offset,
                const std::vector<size_t>& count);
        template<typename T, size_t D>
        boost::multi_array<T, D> read (std::string varName);
        template<typename T, size_t D>
        boost::multi_array<T, D> read(
                std::string varName,
                const std::vector<size_t>& offset,
                const std::vector<size_t>& count);

    };

    template<typename T, size_t D>
    void File::write(
            std::string varName,
            const boost::multi_array<T, D>& data)
    {
        netCDF::NcVar variable = getVar (varName);

#ifdef _OPENMP
        lock();
#endif

        variable.putVar (data.data());

#ifdef _OPENMP
        unlock();
#endif
    }

    template<typename T, size_t D>
    void File::write(
            std::string varName,
            const boost::multi_array<T, D>& data,
            const std::vector<size_t>& offset,
            const std::vector<size_t>& count)
    {
        netCDF::NcVar variable = getVar (varName);

#ifdef _OPENMP
        lock();
#endif

        variable.putVar (offset, count, data.data());

#ifdef _OPENMP
        unlock();
#endif
    }

    template<typename T, size_t D>
    boost::multi_array<T, D> File::read (std::string varName)
    {
        netCDF::NcVar variable = getVar (varName);

        std::vector<netCDF::NcDim> dimensions = variable.getDims();
        if (dimensions.size () != D) throw VariableNotExistException();

        boost::array<long, D> dimensionSizes;
        for (size_t i = 0; i < D; ++i)
            dimensionSizes[i] = dimensions[i].getSize();

        boost::multi_array<T, D> data (dimensionSizes);

#ifdef _OPENMP
        lock();
#endif

        variable.getVar (data.data());

#ifdef _OPENMP
        unlock();
#endif
        return data;

    }

    template<typename T, size_t D>
    boost::multi_array<T, D> File::read(
            std::string varName,
            const std::vector<size_t>& offset,
            const std::vector<size_t>& count)
    {
        netCDF::NcVar variable = getVar (varName);
        if (variable.getDims().size() != D) throw VariableNotExistException();

        boost::array<long, D> dimensionSizes;
        for (size_t i = 0; i < D; ++i)
            dimensionSizes[i] = count[i];

        boost::multi_array<T, D> data (dimensionSizes);

#ifdef _OPENMP
        lock();
#endif

        variable.getVar (offset, count, data.data());

#ifdef _OPENMP
        unlock();
#endif

        return data;
    }

}

#endif
