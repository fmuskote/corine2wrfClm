#include <string>
#include "corine.h"
#include "clm.h"

using namespace corine;
using std::string;

CorineFractions::CorineFractions ()
    : NotClmFractions (typeCount, missing)
{}

clm::ClmFractions CorineFractions::map2Clm () const
{
    clm::ClmFractions clmFractions;

    clmFractions.add (clm::noPFT,                            operator[] (continuousUrbanFabric));
    clmFractions.add (clm::noPFT,                            operator[] (discontinuousUrbanFabric));
    clmFractions.add (clm::noPFT,                            operator[] (industrialCommercial));
    clmFractions.add (clm::noPFT,                            operator[] (roadAndRail));
    clmFractions.add (clm::noPFT,                            operator[] (port));
    clmFractions.add (clm::noPFT,                            operator[] (airport));
    clmFractions.add (clm::noPFT,                            operator[] (mineralExtractionSites));
    clmFractions.add (clm::noPFT,                            operator[] (dumpSites));
    clmFractions.add (clm::noPFT,                            operator[] (constructionSites));
    clmFractions.add (clm::c3Grass,                          operator[] (greenUrbanAreas));
    clmFractions.add (clm::c3Grass,                          operator[] (sportLeisureFacilities));
    clmFractions.add (clm::crop,                             operator[] (nonIrrigatedArableLand));
    clmFractions.add (clm::crop,                             operator[] (permanentIrrigatedLand));
    clmFractions.add (clm::crop,                             operator[] (riceFields));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  operator[] (vineyards));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  operator[] (fruitAndBerryPlantations));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  operator[] (oliveGroves));
    clmFractions.add (clm::c3Grass,                          operator[] (pastures));
    clmFractions.add (clm::crop,                             operator[] (annualCropsAssociatedWithPermanentCrops));
    clmFractions.add (clm::crop,                             operator[] (complexCultivationPatterns));
    clmFractions.add (clm::crop,                             operator[] (landPrincipallyOccupiedByAgricultureWithSignificantAreasOfNaturalVegetation));
    clmFractions.add (clm::crop,                             operator[] (agroForestryAreas));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  operator[] (broadLeavedForest));
    clmFractions.add (clm::needleleafEvergreenTreeTemperate, operator[] (coniferousForest));
    clmFractions.add (clm::broadleafDeciduousTreeTemperate,  operator[] (mixedForest));
    clmFractions.add (clm::c3Grass,                          operator[] (naturalGrassland));
    clmFractions.add (clm::broadleafEvergreenShrubTemperate, operator[] (moorsAndHeatland));
    clmFractions.add (clm::broadleafEvergreenShrubTemperate, operator[] (sclerophyllousVegetation));
    clmFractions.add (clm::broadleafDeciduousShrubTemperate, operator[] (transitionalWoodlandShrub));
    clmFractions.add (clm::noPFT,                            operator[] (beachesDunesSands));
    clmFractions.add (clm::noPFT,                            operator[] (bareRocks));
    clmFractions.add (clm::noPFT,                            operator[] (sparselyVegetatedAreas));
    clmFractions.add (clm::noPFT,                            operator[] (burnedAreas));
    clmFractions.add (clm::noPFT,                            operator[] (claciersAndPerpetualSnow));
    clmFractions.add (clm::noPFT,                            operator[] (inlandMarshes));
    clmFractions.add (clm::noPFT,                            operator[] (peatBogs));
    clmFractions.add (clm::noPFT,                            operator[] (saltMarshes));
    clmFractions.add (clm::noPFT,                            operator[] (salines));
    clmFractions.add (clm::noPFT,                            operator[] (intertidalFlats));
    clmFractions.add (clm::noPFT,                            operator[] (waterCourses));
    clmFractions.add (clm::noPFT,                            operator[] (waterBodies));
    clmFractions.add (clm::noPFT,                            operator[] (coastalLagoons));
    clmFractions.add (clm::noPFT,                            operator[] (estuaries));
    clmFractions.add (clm::noPFT,                            operator[] (seaAndOcean));

    return clmFractions;
}

const string corine::getFileName (const string path, const size_t type)
{
    string result ("");
    result += path;
    result += "/clc06_c";
    switch (type)
    {
        case  1:
            result += "111"; break;
        case  2:
            result += "112"; break;
        case  3:
            result += "121"; break;
        case  4:
            result += "122"; break;
        case  5:
            result += "123"; break;
        case  6:
            result += "124"; break;
        case  7:
            result += "131"; break;
        case  8:
            result += "132"; break;
        case  9:
            result += "133"; break;
        case 10:
            result += "141"; break;
        case 11:
            result += "142"; break;
        case 12:
            result += "211"; break;
        case 13:
            result += "212"; break;
        case 14:
            result += "213"; break;
        case 15:
            result += "221"; break;
        case 16:
            result += "222"; break;
        case 17:
            result += "223"; break;
        case 18:
            result += "231"; break;
        case 19:
            result += "241"; break;
        case 20:
            result += "242"; break;
        case 21:
            result += "243"; break;
        case 22:
            result += "244"; break;
        case 23:
            result += "311"; break;
        case 24:
            result += "312"; break;
        case 25:
            result += "313"; break;
        case 26:
            result += "321"; break;
        case 27:
            result += "322"; break;
        case 28:
            result += "323"; break;
        case 29:
            result += "324"; break;
        case 30:
            result += "331"; break;
        case 31:
            result += "332"; break;
        case 32:
            result += "333"; break;
        case 33:
            result += "334"; break;
        case 34:
            result += "335"; break;
        case 35:
            result += "411"; break;
        case 36:
            result += "412"; break;
        case 37:
            result += "421"; break;
        case 38:
            result += "422"; break;
        case 39:
            result += "423"; break;
        case 40:
            result += "511"; break;
        case 41:
            result += "512"; break;
        case 42:
            result += "521"; break;
        case 43:
            result += "522"; break;
        case 44:
            result += "523"; break;
    }
    result += ".shp";
    return result;
}

double CorineFractions::getArtificialFraction () const
{
    double result = 0.0;
    result += operator[] (continuousUrbanFabric);
    result += operator[] (discontinuousUrbanFabric);
    result += operator[] (industrialCommercial);
    result += operator[] (roadAndRail);
    result += operator[] (port);
    result += operator[] (airport);
    result += operator[] (mineralExtractionSites);
    result += operator[] (dumpSites);
    result += operator[] (constructionSites);
    result += operator[] (greenUrbanAreas);
    result += operator[] (sportLeisureFacilities);
    return result;
}

double CorineFractions::getWetlandFraction () const
{
    double result = 0.0;
    result += operator[] (inlandMarshes);
    result += operator[] (peatBogs);
    result += operator[] (saltMarshes);
    result += operator[] (salines);
    result += operator[] (intertidalFlats);
    return result;
}

double CorineFractions::getWaterFraction () const
{
    double result = 0.0;
    result += operator[] (waterCourses);
    result += operator[] (waterBodies);
    result += operator[] (coastalLagoons);
    result += operator[] (estuaries);
    result += operator[] (seaAndOcean);
    return result;
}

double CorineFractions::getGlacierFraction () const
{
    double result = 0.0;
    result += operator[] (claciersAndPerpetualSnow);
    return result;
}
