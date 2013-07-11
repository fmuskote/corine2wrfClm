#ifndef CORINE_H
#define CORINE_H

#include <string>
#include "notClmFractions.h"
#include "clm.h"

namespace corine {

    const size_t typeCount = 44;

    enum Type {
        continuousUrbanFabric      =  0,
        discontinuousUrbanFabric   =  1,
        industrialCommercial       =  2,
        roadAndRail                =  3,
        port                       =  4,
        airport                    =  5,
        mineralExtractionSites     =  6,
        dumpSites                  =  7,
        constructionSites          =  8,
        greenUrbanAreas            =  9,
        sportLeisureFacilities     = 10,
        nonIrrigatedArableLand     = 11,
        permanentIrrigatedLand     = 12,
        riceFields                 = 13,
        vineyards                  = 14,
        fruitAndBerryPlantations   = 15,
        oliveGroves                = 16,
        pastures                   = 17,
        annualCropsAssociatedWithPermanentCrops = 18,
        complexCultivationPatterns = 19,
        landPrincipallyOccupiedByAgricultureWithSignificantAreasOfNaturalVegetation = 20,
        agroForestryAreas          = 21,
        broadLeavedForest          = 22,
        coniferousForest           = 23,
        mixedForest                = 24,
        naturalGrassland           = 25,
        moorsAndHeatland           = 26,
        sclerophyllousVegetation   = 27,
        transitionalWoodlandShrub  = 28,
        beachesDunesSands          = 29,
        bareRocks                  = 30,
        sparselyVegetatedAreas     = 31,
        burnedAreas                = 32,
        claciersAndPerpetualSnow   = 33,
        inlandMarshes              = 34,
        peatBogs                   = 35,
        saltMarshes                = 36,
        salines                    = 37,
        intertidalFlats            = 38,
        waterCourses               = 39,
        waterBodies                = 40,
        coastalLagoons             = 41,
        estuaries                  = 42,
        seaAndOcean                = 43,
    };

    class CorineFractions : public NotClmFractions
    {
        public:
            CorineFractions ();
            clm::ClmFractions map2Clm () const;
            double getArtificialFraction () const;
            double getWetlandFraction () const;
            double getWaterFraction () const;
            double getGlacierFraction () const;
    };

    const std::string getFileName (const std::string, const size_t);

}
                 
#endif
