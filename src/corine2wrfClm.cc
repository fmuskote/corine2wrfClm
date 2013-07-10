#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <getopt.h>
#include <boost/multi_array.hpp>

#include "notClmFractions.h"
#include "corine.h"
#include "wrfFile.h"
#include "shapeFile.h"
#include "clm.h"


#ifdef _OPENMP
#include <omp.h>
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <netcdfcpp.h>

using namespace std;
void doTheWork  (const string, const string);

static int verbosity = 0;

int main (int argc, char ** argv) {
    string wrfFileName ("wrfinput_d01");
    string corineFileName ("g100_06.tif");

    while (true) {
        static struct option long_options[] = {
            {"help",       no_argument,       0, 'h'},
            {"verbose",    no_argument,       0, 'v'},
            {"version",    no_argument,       0, 'V'},
            {"corineFile", required_argument, 0, 'c'},
            {"wrfFile",    required_argument, 0, 'w'},
            {0,            0,                 0, 0  }
        };

        int option_index = 0;
        int c = getopt_long (argc, argv, "hvVc:w:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                cerr << "option " << long_options[option_index].name;
                if (optarg)
                    cerr << " with arg " << optarg;
                cerr << endl;
                break;
            case 'h':
                cout << "print the help text" << endl;
                return EXIT_SUCCESS;
            case 'v':
                verbosity++;
                break;
            case 'V':
                cout << PACKAGE_STRING << endl;
                return EXIT_SUCCESS;
            case 'c':
                corineFileName = string (optarg);
                break;
            case 'w':
                wrfFileName = string (optarg);
                break;
            case '?':
                break;
            default:
                exit (EXIT_FAILURE);
        }
    }

    if (verbosity > 0) {
        cout << "corineFileName = '" << corineFileName << "'" << endl;
        cout << "wrfFileName =    '" << wrfFileName << "'" << endl;
    }

    doTheWork (corineFileName, wrfFileName);

    return EXIT_SUCCESS;
}

void doTheWork (const string corineFileName, const string wrfFileName) {
    // Open WRF file //
    //---------------//
    WrfFile wrf (wrfFileName, WrfFile::Write);
    if (!(wrf.isUsgsLUType () or wrf.isModisLUType ()))
        throw UnknownLUTypeException ();

    boost::multi_array<corine::CorineFractions, 2>
        fractions (boost::extents[wrf.iSize ()][wrf.jSize ()]);

    OGRSpatialReference* wrfCoordSys = wrf.getCoordinateSystem();
    wrfCoordSys->Reference ();
    OGRRegisterAll();

#ifdef DEBUG
    for (size_t type = 1; type < 2; type++)
#else
    for (size_t type = 1; type < corine::typeCount; type++)
#endif
    {
        string fileName = corine::getFileName (corineFileName, type);
        if (verbosity > 0) cout << "working on corine file " << fileName << endl;

#ifdef DEBUG2
        for (size_t i = 0; i < 1; i++)
#else
#pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < wrf.iSize (); i++)
#endif
        {
            OGRDataSource* dataSource = OGRSFDriverRegistrar::Open (fileName.c_str (), FALSE);

            OGRLayer* layer = dataSource->GetLayer (0);
            if (layer->GetFeatureCount () > 0) {
                OGRSpatialReference* corineCoordSys = layer->GetSpatialRef ();
                OGRCoordinateTransformation* trafoCorine2Wrf =
                    OGRCreateCoordinateTransformation (corineCoordSys, wrfCoordSys);
                OGRCoordinateTransformation* trafoWrf2Corine =
                    OGRCreateCoordinateTransformation (wrfCoordSys, corineCoordSys);

#ifdef DEBUG2
                for (size_t j = 0; j < 1; j++)
#else
                for (size_t j = 0; j < wrf.jSize (); j++)
#endif
                {
                    OGRGeometry* wrfPolygon = wrf.getPolygon2 (i, j);
                    double wrfArea = ((OGRPolygon*)wrfPolygon)->get_Area ();

                    OGRGeometry* wrfPolygonInCorineCoord =
                        wrfPolygon->clone ();
                    wrfPolygonInCorineCoord->transform (trafoWrf2Corine);
                    layer->SetSpatialFilter (wrfPolygonInCorineCoord);
                    if (layer->GetFeatureCount () > 0) {

                        layer->ResetReading ();
                        OGRFeature* feature;
                        while ((feature = layer->GetNextFeature ())) {
                            OGRGeometry* corinePolygon = feature->GetGeometryRef ();
                            corinePolygon->transform (trafoCorine2Wrf);

                            if (corinePolygon->Intersects (wrfPolygon)) {
                                OGRGeometry* intersection =
                                    corinePolygon->Intersection (wrfPolygon);
                                double intersectionArea =
                                    ((OGRPolygon*)intersection)->get_Area ();

                                fractions[i][j].add (type, intersectionArea/wrfArea);
                                //fractions[i][j][type] += intersectionArea/wrfArea;

                                OGRGeometryFactory::destroyGeometry (intersection);
                            }
                            OGRFeature::DestroyFeature (feature);
                        }
                    }
                    OGRGeometryFactory::destroyGeometry (wrfPolygonInCorineCoord);
                    OGRGeometryFactory::destroyGeometry (wrfPolygon);
                }

                OGRCoordinateTransformation::DestroyCT (trafoCorine2Wrf);
                OGRCoordinateTransformation::DestroyCT (trafoWrf2Corine);
            }
            OGRDataSource::DestroyDataSource (dataSource);
        }
    }
    wrfCoordSys->Release ();

#ifndef NOOUTPUT
#ifdef DEBUG3
    for (size_t i = 0; i < 1; i++)
        for (size_t j = 0; j < 1; j++)
#else
#pragma omp parallel for
    for (size_t i = 0; i < wrf.iSize (); i++)
        for (size_t j = 0; j < wrf.jSize (); j++)
#endif
        {
            cout << "corine fractions:" << endl;
            cout << fractions[i][j] << endl;

            clm::ClmFractions clmFractions = fractions[i][j].map2Clm ();
            
            cout << "clm fractions:" << endl;
            cout << clmFractions << endl;

            double missing = clmFractions[clm::missing];
            if (missing > 1.0e-33) {

                if (verbosity > 1)
                    cerr << "WARNING: using partly original land use in grid cell "
                         << i << " " << j << " with missing fraction of " << 
                         missing << endl;

                clm::ClmFractions originalLandUseFractions = wrf.getLandUseFraction (i, j)->map2Clm ();

                cout << "original fractions:" << endl;
                cout << originalLandUseFractions << endl;

                for (size_t type = 0; type < clm::typeCount; ++type)
                    clmFractions.set (type, originalLandUseFractions[type]*missing);

                cout << "clm fractions:" << endl;
                cout << clmFractions << endl;

            }

            try
            {
                clmFractions.check ();
            }
            catch (std::exception& e)
            {
                cout << clmFractions;
                throw e;
            }

            double fractionWater = fractions[i][j].getWaterFraction ();
            double fractionUrban = fractions[i][j].getArtificialFraction ();
            double fractionGlacier = fractions[i][j].getGlacierFraction ();
            double fractionWetland = fractions[i][j].getWetlandFraction ();
            
            wrf.writeClmPftTypeFractions (i, j, clmFractions);
            wrf.writeWaterFraction (i, j, &fractionWater);
            wrf.writeUrbanFraction (i, j, &fractionUrban);
            wrf.writeGlacierFraction (i, j, &fractionGlacier);
            wrf.writeWetlandFraction (i, j, &fractionWetland);
        }
#endif

}
