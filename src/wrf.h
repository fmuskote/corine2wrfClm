#ifndef WRFFILE_H
#define WRFFILE_H

#include <netcdfcpp.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
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
    class NotUsgsLanduseExeption {};
    class WrongDimensionSizeException {};
    class UnknownLUTypeException {};
    class WrongMosaicGeometryException {};

    class File : public NcFile, public GeoRaster
    {
      private:
        size_t _iSize;
        size_t _jSize;
        boost::scoped_ptr<NcError> errorBehavior;

#ifdef _OPENMP
        boost::scoped_ptr<omp_lock_t> _lock;
#endif

        const double getDx () const;
        const double getDy () const;
        void write0Dto2D (std::string, size_t, size_t, double);

        boost::multi_array<float, 3> mosaicArray (
                const boost::multi_array<float, 2>&,
                size_t, size_t, size_t) const;

      public:
        File (std::string, FileMode = ReadOnly);
        ~File ();
        size_t iSize () const;
        size_t jSize () const;
        boost::shared_ptr<NotClmFractions> getLandUseFraction (size_t, size_t);
        void writeClmPftTypeFractions (size_t, size_t, const clm::ClmFractions&);
        bool isModisLUType () const;
        bool isUsgsLUType () const;
        boost::multi_array<float, 2> getClmType (size_t);
        void createMosaic (wrf::File&);
#ifdef _OPENMP
        void lock ();
        void unlock ();
#endif
        void writeWaterFraction (size_t, size_t, double);
        void writeUrbanFraction (size_t, size_t, double);
        void writeGlacierFraction (size_t, size_t, double);
        void writeWetlandFraction (size_t, size_t, double);

        template<typename T, size_t D>
        void write (
                std::string varName,
                const boost::multi_array<T, D>& data);
        template<typename T, size_t D>
        void write (
                std::string varName,
                const boost::multi_array<T, D>& data,
                const boost::array<long, D+1>& offset,
                const boost::array<long, D+1>& count);
        template<typename T, size_t D>
        boost::multi_array<T, D> read (std::string varName);
        template<typename T, size_t D>
        boost::multi_array<T, D> read (
                std::string varName,
                boost::array<long, D+1>& offset,
                boost::array<long, D+1>& count);

    };

    template<typename T, size_t D>
    void File::write (
            std::string varName,
            const boost::multi_array<T, D>& data) {
        NcVar* variable = get_var (varName.c_str ());
        if (!variable)
            throw VariableNotExistException ();

#ifdef _OPENMP
        lock ();
#endif

        variable->put (data.data ());

#ifdef _OPENMP
        unlock ();
#endif
    }
    template<typename T, size_t D>
    void File::write (
            std::string varName,
            const boost::multi_array<T, D>& data,
            const boost::array<long, D+1>& offset,
            const boost::array<long, D+1>& count) {
        NcVar* variable = get_var (varName.c_str ());
        if (!variable)
            throw VariableNotExistException ();

#ifdef _OPENMP
        lock ();
#endif

        variable->set_cur (offset.data ());
        variable->put (data.data (), count.data ());

#ifdef _OPENMP
        unlock ();
#endif
    }
    template<typename T, size_t D>
    boost::multi_array<T, D> File::read (
            std::string varName,
            boost::array<long, D+1>& offset,
            boost::array<long, D+1>& count) {
        NcVar* variable = get_var (varName.c_str ());
        if (!variable)
            throw VariableNotExistException ();

        int variableDimension = variable->num_dims ();
        if (variableDimension != D)
            throw VariableNotExistException ();

        boost::array<long, D> dims;
        for (size_t i = 0; i < D; ++i)
            dims[i] = count[i+1];
        boost::multi_array<T, D> data (dims);

#ifdef _OPENMP
        lock ();
#endif

        variable->set_cur (offset.data ());
        variable->get (data.data (), count.data ());

#ifdef _OPENMP
        unlock ();
#endif

        return data;
    }
    template<typename T, size_t D>
    boost::multi_array<T, D> File::read (std::string varName) {
        NcVar* variable = get_var (varName.c_str ());
        if (!variable)
            throw VariableNotExistException ();

        int variableDimension = variable->num_dims ();
        if (variableDimension != D)
            throw VariableNotExistException ();

        boost::array<long, D> dims;
        for (int i = 0; i < variableDimension; ++i)
            dims[i] = variable->get_dim (i)->size ();

        boost::multi_array<T, D> data (dims);

#ifdef _OPENMP
        lock ();
#endif

        variable->get (data.data ());

#ifdef _OPENMP
        unlock ();
#endif
        return data;

    }

}

#endif
