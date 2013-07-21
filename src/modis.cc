#include "modis.h"
#include "clm.h"
#include "usgs.h"

using namespace modis;

ModisFractions::ModisFractions()
    : NotClmFractions (modis::typeCount)
{}

usgs::UsgsFractions ModisFractions::map2Usgs() const
{
    usgs::UsgsFractions usgsFractions;

    usgsFractions.add (usgs::urbanAndBuildUpLand,        operator[] (urbanAndBuiltUp));
    usgsFractions.add (usgs::croplandAndGrasslandMosaic, operator[] (croplands));
    usgsFractions.add (usgs::croplandAndWoodlandMosaic,  operator[] (croplandAndNaturalVegetationMosaic));
    usgsFractions.add (usgs::grassland,                  operator[] (grasslands));
    usgsFractions.add (usgs::shrubland,                  operator[] (closedShrublands));
    usgsFractions.add (usgs::mixedShrublandAndGrassland, operator[] (openShrublands));
    usgsFractions.add (usgs::savanna,                    operator[] (woodySavannas));
    usgsFractions.add (usgs::savanna,                    operator[] (savannas));
    usgsFractions.add (usgs::deciduousBroadleafForest,   operator[] (deciduousBroadleafForest));
    usgsFractions.add (usgs::deciduousNeedleleafForest,  operator[] (deciduousNeedleleafForest));
    usgsFractions.add (usgs::evergreenBroadleafForest,   operator[] (evergreenBroadleafForest));
    usgsFractions.add (usgs::evergreenNeedleleafForest,  operator[] (evergreenNeedleleafForest));
    usgsFractions.add (usgs::mixedForest,                operator[] (mixedForest));
    usgsFractions.add (usgs::waterBodies,                operator[] (water));
    usgsFractions.add (usgs::herbaceousWetland,          operator[] (permanentWetlands));
    usgsFractions.add (usgs::barrenOrSparselyVegetated,  operator[] (barrenOrSparselyVegetated));
    usgsFractions.add (usgs::woodenTundra,               operator[] (woodedTundra));
    usgsFractions.add (usgs::mixedTundra,                operator[] (mixedTundra));
    usgsFractions.add (usgs::bareGroundTundra,           operator[] (barrenTundra));
    usgsFractions.add (usgs::snowOrIce,                  operator[] (snowAndIce));

    return usgsFractions;
}

clm::ClmFractions ModisFractions::map2Clm() const
{
    return map2Usgs().map2Clm();
}
