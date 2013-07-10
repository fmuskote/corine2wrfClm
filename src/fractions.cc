#include "fractions.h"
#include <cmath>

Fractions::Fractions (const size_t& typeCount, const size_t& missing)
    : _data (new double[typeCount])
{
    _typeCount = typeCount;
    _missing = missing;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = 0.0;
    _data[_missing] = 1.0;
}

Fractions::Fractions (const Fractions& fractions)
    : _data (new double[fractions._typeCount])
{
    _typeCount = fractions._typeCount;
    _missing = fractions._missing;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = fractions[i];
}

void Fractions::check () const
{
    double sum = 0.0;
    for (size_t i = 0; i < _typeCount; ++i)
    {
        if (_data[i] < 0.0) throw FractionInconsistent ();
        sum += _data[i];
    }
    if (std::abs (sum - 1.0) > 1e-33) throw FractionInconsistent ();
}
    
const double& Fractions::operator[] (const size_t& index) const
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    return _data[index];
}

void Fractions::set (const size_t& index, const double& value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[_missing] -= value - _data[index];
    _data[index]   =  value;
}

void Fractions::add (const size_t& index, const double& value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[index]    += value;
    _data[_missing] -= value;
}

std::ostream& operator<< (std::ostream& out, const Fractions& fractions)
{
    for (size_t i = 0; i < fractions._typeCount; ++i)
        out << i << "\t" << fractions[i] << std::endl;
    return out;
}

