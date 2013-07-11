#include "fractions.h"

Fractions::Fractions (const size_t& typeCount, const bool withCheck)
    : _data (new double[typeCount])
{
    _typeCount = typeCount;
    _withCheck = withCheck;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = 0.0;
}

Fractions::Fractions (const Fractions& fractions)
    : _data (new double[fractions._typeCount])
{
    _typeCount = fractions._typeCount;
    _withCheck = fractions._withCheck;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = fractions[i];
}

void Fractions::check () const
{
    for (size_t i = 0; i < _typeCount; ++i)
        if (_data[i] < 0.0) throw FractionInconsistent ();

    if (missing () < -1e-6) throw FractionInconsistent ();
}
    
const double& Fractions::operator[] (const size_t& index) const
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    return _data[index];
}

void Fractions::set (const size_t& index, const double& value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[index]   =  value;

    if (_withCheck) check ();
}

void Fractions::add (const size_t& index, const double& value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[index]    += value;

    if (_withCheck) check ();
}

std::ostream& operator<< (std::ostream& out, const Fractions& fractions)
{
    for (size_t i = 0; i < fractions._typeCount; ++i)
        out << i << "\t" << fractions[i] << std::endl;
    out << "miss" << "\t" << fractions.missing () << std::endl;
    return out;
}

double Fractions::missing () const
{
    double result = 1.0;
    for (size_t i = 0; i < _typeCount; ++i)
        result -= _data[i];

    return result;
}
