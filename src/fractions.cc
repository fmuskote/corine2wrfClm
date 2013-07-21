#include "fractions.h"

Fractions::Fractions (size_t size)
    : data_ (new double[size]),
      typeCount_ (size)
{
    for (size_t i = 0; i < typeCount(); ++i)
        data_[i] = 0.0;
}

Fractions::Fractions (const Fractions& fractions)
    : data_ (new double[fractions.typeCount()]),
      typeCount_ (fractions.typeCount())
{
    for (size_t i = 0; i < typeCount(); ++i)
        data_[i] = fractions[i];
}

void Fractions::check() const
{
    for (size_t i = 0; i < typeCount(); ++i)
        if (data_[i] < 0.0) throw FractionInconsistent ();

    if (missing() < -1e-6) throw FractionInconsistent ();
}
    
const double& Fractions::operator[] (size_t index) const
{
    if (index >= typeCount()) throw FractionOutOfRange ();
    return data_[index];
}

void Fractions::set (size_t index, double value)
{
    if (index >= typeCount()) throw FractionOutOfRange ();
    data_[index] =  value;
}

void Fractions::add (size_t index, double value)
{
    if (index >= typeCount()) throw FractionOutOfRange ();
    data_[index] += value;
}

std::ostream& operator<< (std::ostream& out, const Fractions& fractions)
{
    for (size_t i = 0; i < fractions.typeCount(); ++i)
        out << i << "\t" << fractions[i] << std::endl;
    out << "miss" << "\t" << fractions.missing () << std::endl;
    return out;
}

double Fractions::missing() const
{
    double result = 1.0;
    for (size_t i = 0; i < typeCount(); ++i)
        result -= data_[i];

    return result;
}
