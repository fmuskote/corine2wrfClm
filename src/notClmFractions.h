#ifndef NOTCLMFRACTIONS_H
#define NOTCLMFRACTIONS_H

#include "fractions.h"
#include "clm.h"

class NotClmFractions : public Fractions
{
    public:
        NotClmFractions (const size_t& typeCount, const size_t& missing);
        virtual clm::ClmFractions map2Clm () const = 0;
};

#endif
