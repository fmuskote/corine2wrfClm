#ifndef FRACTIONS_H
#define FRACTIONS_H

#include <boost/scoped_array.hpp>
#include <exception>
#include <ostream>

class Fractions
{
    private:
        boost::scoped_array<double> _data;
        size_t                      _missing;
    protected:
        size_t                      _typeCount;
    public:
        Fractions (const size_t& typeCount, const size_t& missing);
        Fractions (const Fractions&);
        void check () const;
        const double& operator[] (const size_t& index) const;
        void set (const size_t& index, const double& value);
        void add (const size_t& index, const double& value);
        friend std::ostream& operator<< (std::ostream& out, const Fractions& fractions);
};
class FractionInconsistent : public std::exception {};
class FractionOutOfRange   : public std::exception {};

#endif
