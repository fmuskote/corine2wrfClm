#ifndef MODIS_H
#define MODIS_H

#include <boost/shared_ptr.hpp>
#include "notClmFractions.h"
#include "clm.h"
#include "usgs.h"

namespace modis {

    const size_t typeCount = 21;

    enum Type {
        evergreenNeedleleafForest          =  0,
        evergreenBroadleafForest           =  1,
        deciduousNeedleleafForest          =  2,
        deciduousBroadleafForest           =  3,
        mixedForest                        =  4,
        closedShrublands                   =  5,
        openShrublands                     =  6,
        woodySavannas                      =  7,
        savannas                           =  8,
        grasslands                         =  9,
        permanentWetlands                  = 10,
        croplands                          = 11,
        urbanAndBuiltUp                    = 12,
        croplandAndNaturalVegetationMosaic = 13,
        snowAndIce                         = 14,
        barrenOrSparselyVegetated          = 15,
        water                              = 16,
        woodedTundra                       = 17,
        mixedTundra                        = 18,
        barrenTundra                       = 19,
        missing                            = 20
    };

    class ModisFractions : public NotClmFractions
    {
        private:
            usgs::UsgsFractions map2Usgs () const;
        public:
            ModisFractions ();
            clm::ClmFractions map2Clm () const;
    };

}
#endif
