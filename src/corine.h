#ifndef CORINE_H
#define CORINE_H

#include <string>
#include "notClmFractions.h"
#include "clm.h"

namespace corine {

    const size_t typeCount = 45;

    enum Type {
        missing                    =  0,
        continuousUrbanFabric      =  1,
        discontinuousUrbanFabric   =  2,
        industrialCommercial       =  3,
        roadAndRail                =  4,
        port                       =  5,
        airport                    =  6,
        mineralExtractionSites     =  7,
        dumpSites                  =  8,
        constructionSites          =  9,
        greenUrbanAreas            = 10,
        sportLeisureFacilities     = 11,
        nonIrrigatedArableLand     = 12,
        permanentIrrigatedLand     = 13,
        riceFields                 = 14,
        vineyards                  = 15,
        fruitAndBerryPlantations   = 16,
        oliveGroves                = 17,
        pastures                   = 18,
        annualCropsAssociatedWithPermanentCrops = 19,
        complexCultivationPatterns = 20,
        landPrincipallyOccupiedByAgricultureWithSignificantAreasOfNaturalVegetation = 21,
        agroForestryAreas          = 22,
        broadLeavedForest          = 23,
        coniferousForest           = 24,
        mixedForest                = 25,
        naturalGrassland           = 26,
        moorsAndHeatland           = 27,
        sclerophyllousVegetation   = 28,
        transitionalWoodlandShrub  = 29,
        beachesDunesSands          = 30,
        bareRocks                  = 31,
        sparselyVegetatedAreas     = 32,
        burnedAreas                = 33,
        claciersAndPerpetualSnow   = 34,
        inlandMarshes              = 35,
        peatBogs                   = 36,
        saltMarshes                = 37,
        salines                    = 38,
        intertidalFlats            = 39,
        waterCourses               = 40,
        waterBodies                = 41,
        coastalLagoons             = 42,
        estuaries                  = 43,
        seaAndOcean                = 44,
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
