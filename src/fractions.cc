#include "fractions.h"

Fractions::Fractions (size_t typeCount)
    : _data (new double[typeCount])
{
    _typeCount = typeCount;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = 0.0;
}

Fractions::Fractions (const Fractions& fractions)
    : _data (new double[fractions._typeCount])
{
    _typeCount = fractions._typeCount;
    for (size_t i = 0; i < _typeCount; ++i)
        _data[i] = fractions[i];
}

void Fractions::check () const
{
    for (size_t i = 0; i < _typeCount; ++i)
        if (_data[i] < 0.0) throw FractionInconsistent ();

    if (missing () < -1e-6) throw FractionInconsistent ();
}
    
const double& Fractions::operator[] (size_t index) const
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    return _data[index];
}

void Fractions::set (size_t index, double value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[index]   =  value;
}

void Fractions::add (size_t index, double value)
{
    if (index >= _typeCount) throw FractionOutOfRange ();
    _data[index]    += value;
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
