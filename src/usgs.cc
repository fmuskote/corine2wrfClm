#include "usgs.h"
#include "clm.h"

using namespace usgs;

UsgsFractions::UsgsFractions ()
    : NotClmFractions (typeCount)
{}

clm::ClmFractions UsgsFractions::map2Clm () const
{
    clm::ClmFractions clmFractions;

    clmFractions.add (clm::noPFT, operator[] (urbanAndBuildUpLand));

    clmFractions.add (clm::noPFT, 0.15*operator[] (drylandCropAndPasture));
    clmFractions.add (clm::crop,  0.85*operator[] (drylandCropAndPasture));
                                  
    clmFractions.add (clm::noPFT, 0.15*operator[] (irrigatedCroplandAndPasture));
    clmFractions.add (clm::crop,  0.85*operator[] (irrigatedCroplandAndPasture));
                                  
    clmFractions.add (clm::noPFT, 0.15*operator[] (mixedDrylandAndIrrigatedCropland));
    clmFractions.add (clm::crop,  0.85*operator[] (mixedDrylandAndIrrigatedCropland));

    clmFractions.add (clm::noPFT,   0.15*operator[] (croplandAndGrasslandMosaic));
    clmFractions.add (clm::c4Grass, 0.35*operator[] (croplandAndGrasslandMosaic));
    clmFractions.add (clm::crop,    0.50*operator[] (croplandAndGrasslandMosaic));

    clmFractions.add (clm::noPFT,                         0.30*operator[] (croplandAndWoodlandMosaic));
    clmFractions.add (clm::needleleafDeciduousTreeBoreal, 0.30*operator[] (croplandAndWoodlandMosaic));
    clmFractions.add (clm::crop,                          0.40*operator[] (croplandAndWoodlandMosaic));

    clmFractions.add (clm::noPFT,   0.20*operator[] (grassland));
    clmFractions.add (clm::c3Grass, 0.20*operator[] (grassland));
    clmFractions.add (clm::c4Grass, 0.60*operator[] (grassland));

    clmFractions.add (clm::noPFT,                            0.20*operator[] (shrubland));
    clmFractions.add (clm::broadleafEvergreenShrubTemperate, 0.80*operator[] (shrubland));

    clmFractions.add (clm::noPFT,                            0.20*operator[] (mixedShrublandAndGrassland));
    clmFractions.add (clm::broadleafEvergreenShrubTemperate, 0.40*operator[] (mixedShrublandAndGrassland));
    clmFractions.add (clm::c4Grass,                          0.40*operator[] (mixedShrublandAndGrassland));

    clmFractions.add (clm::broadleafDeciduousTreeTropical, 0.30*operator[] (savanna));
    clmFractions.add (clm::c4Grass,                        0.70*operator[] (savanna));

    clmFractions.add (clm::noPFT,                           0.25*operator[] (deciduousBroadleafForest));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate, 0.75*operator[] (deciduousBroadleafForest));

    clmFractions.add (clm::noPFT,                         0.50*operator[] (deciduousNeedleleafForest));
    clmFractions.add (clm::needleleafDeciduousTreeBoreal, 0.50*operator[] (deciduousNeedleleafForest));

    clmFractions.add (clm::noPFT,                          0.05*operator[] (evergreenBroadleafForest));
    clmFractions.add (clm::broadleafEvergreenTreeTropical, 0.95*operator[] (evergreenBroadleafForest));

    clmFractions.add (clm::noPFT,                            0.25*operator[] (evergreenNeedleleafForest));
    clmFractions.add (clm::needleleafEvergreenTreeTemperate, 0.75*operator[] (evergreenNeedleleafForest));

    clmFractions.add (clm::noPFT,                            0.26*operator[] (mixedForest));
    clmFractions.add (clm::needleleafEvergreenTreeTemperate, 0.37*operator[] (mixedForest));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  0.37*operator[] (mixedForest));

    clmFractions.add (clm::noPFT, operator[] (waterBodies));

    clmFractions.add (clm::noPFT, operator[] (herbaceousWetland));

    clmFractions.add (clm::noPFT,                          0.20*operator[] (woodedWetland));
    clmFractions.add (clm::broadleafEvergreenTreeTropical, 0.80*operator[] (woodedWetland));

    clmFractions.add (clm::noPFT,                         0.90*operator[] (barrenOrSparselyVegetated));
    clmFractions.add (clm::broadleafDeciduousShrubBoreal, 0.10*operator[] (barrenOrSparselyVegetated));

    clmFractions.add (clm::noPFT,                         0.40*operator[] (herbaceousTundra));
    clmFractions.add (clm::broadleafDeciduousShrubBoreal, 0.30*operator[] (herbaceousTundra));
    clmFractions.add (clm::c3ArcticGrass,                 0.30*operator[] (herbaceousTundra));

    clmFractions.add (clm::noPFT,                            0.50*operator[] (woodenTundra));
    clmFractions.add (clm::needleleafEvergreenTreeBoreal,    0.13*operator[] (woodenTundra));
    clmFractions.add (clm::needleleafDeciduousTreeBoreal,    0.13*operator[] (woodenTundra));
    clmFractions.add (clm::broadleafDeciduousShrubTemperate, 0.24*operator[] (woodenTundra));

    clmFractions.add (clm::noPFT,                         0.60*operator[] (mixedTundra));
    clmFractions.add (clm::broadleafDeciduousShrubBoreal, 0.20*operator[] (mixedTundra));
    clmFractions.add (clm::c3ArcticGrass,                 0.20*operator[] (mixedTundra));

    clmFractions.add (clm::noPFT,                         0.80*operator[] (bareGroundTundra));
    clmFractions.add (clm::broadleafDeciduousShrubBoreal, 0.10*operator[] (bareGroundTundra));
    clmFractions.add (clm::c3ArcticGrass,                 0.10*operator[] (bareGroundTundra));

    clmFractions.add (clm::noPFT, operator[] (snowOrIce));
    return clmFractions;
}

