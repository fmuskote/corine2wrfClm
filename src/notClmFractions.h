#ifndef NOTCLMFRACTIONS_H
#define NOTCLMFRACTIONS_H

#include "fractions.h"
#include "clm.h"

class NotClmFractions : public Fractions
{
    public:
        NotClmFractions (size_t typeCount);
        virtual clm::ClmFractions map2Clm() const = 0;
};

#endif
