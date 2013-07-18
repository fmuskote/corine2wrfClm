#ifndef CLM_H
#define CLM_H

#include "fractions.h"

namespace clm
{

    const size_t typeCount = 17;

    enum Type
    {
        noPFT                            =  0,
        needleleafEvergreenTreeTemperate =  1,
        needleleafEvergreenTreeBoreal    =  2,
        needleleafDeciduousTreeBoreal    =  3,
        broadleafEvergreenTreeTropical   =  4,
        broadleafEvergreenTreeTemperate  =  5,
        broadleafDeciduousTreeTropical   =  6,
        broadleafDeciduousTreeTemperate  =  7,
        broadleafDeciduousTreeBoreal     =  8,
        broadleafEvergreenShrubTemperate =  9,
        broadleafDeciduousShrubTemperate = 10,
        broadleafDeciduousShrubBoreal    = 11,
        c3ArcticGrass                    = 12,
        c3Grass                          = 13,
        c4Grass                          = 14,
        crop                             = 15,
        crop2                            = 16
    };

    class ClmFractions : public Fractions
    {
        public:
            ClmFractions ();
    };

}

#endif
