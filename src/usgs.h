#ifndef USGS_H
#define USGS_H

#include "notClmFractions.h"
#include "clm.h"

namespace usgs {

    const size_t typeCount = 24;

    enum Type {
        urbanAndBuildUpLand              =  0,
        drylandCropAndPasture            =  1,
        irrigatedCroplandAndPasture      =  2,
        mixedDrylandAndIrrigatedCropland =  3,
        croplandAndGrasslandMosaic       =  4,
        croplandAndWoodlandMosaic        =  5,
        grassland                        =  6,
        shrubland                        =  7,
        mixedShrublandAndGrassland       =  8,
        savanna                          =  9,
        deciduousBroadleafForest         = 10,
        deciduousNeedleleafForest        = 11,
        evergreenBroadleafForest         = 12,
        evergreenNeedleleafForest        = 13,
        mixedForest                      = 14,
        waterBodies                      = 15,
        herbaceousWetland                = 16,
        woodedWetland                    = 17,
        barrenOrSparselyVegetated        = 18,
        herbaceousTundra                 = 19,
        woodenTundra                     = 20,
        mixedTundra                      = 21,
        bareGroundTundra                 = 22,
        snowOrIce                        = 23
    };

    class UsgsFractions : public NotClmFractions
    {
        public:
            UsgsFractions ();
            clm::ClmFractions map2Clm () const;
    };
}

#endif
