#include <iostream>
#include <string>
#include <getopt.h>
#include <boost/multi_array.hpp>

#include <ogr_spatialref.h>
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>

#include "corine.h"
#include "wrf.h"
#include "clm.h"


#ifdef _OPENMP
#include <omp.h>
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;
void doTheWork  (const string, const string);

static int verbosity = 0;

int main (int argc, char ** argv)
{
    string wrfFileName ("wrfinput_d01");
    string corineFileDirectory (".");

    while (true)
    {
        static struct option long_options[] =
        {
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

        switch (c)
        {
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
                corineFileDirectory = string (optarg);
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

    if (verbosity > 0)
    {
        cout << "corineFileDirectory = '" << corineFileDirectory << "'" << endl;
        cout << "wrfFileName =         '" << wrfFileName << "'" << endl;
    }

    doTheWork (corineFileDirectory, wrfFileName);

    return EXIT_SUCCESS;
}

void doTheWork (const string corineFileDirectory, const string wrfFileName)
{
    // Open WRF file //
    //---------------//
    wrf::File wrf (wrfFileName, wrf::File::Write);
    if (!(wrf.isUsgsLUType () or wrf.isModisLUType ()))
        throw wrf::UnknownLUTypeException ();

    boost::multi_array<corine::CorineFractions, 2>
        fractions (boost::extents[wrf.iSize ()][wrf.jSize ()]);

    OGRSpatialReference* wrfCoordSys = wrf.getCoordinateSystem();
    wrfCoordSys->Reference ();
    OGRRegisterAll();

#ifdef DEBUG
    for (size_t type = 0; type < 1; ++type)
#else
    for (size_t type = 0; type < corine::typeCount; ++type)
#endif
    {
        string fileName = corine::getFileName (corineFileDirectory, type);
        if (verbosity > 0) cout << "working on corine file " << fileName << endl;

#ifdef DEBUG2
        for (size_t i = 0; i < 1; ++i)
#else
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (size_t i = 0; i < wrf.iSize (); ++i)
#endif
        {
            OGRDataSource* dataSource = OGRSFDriverRegistrar::Open (fileName.c_str (), FALSE);

            OGRLayer* layer = dataSource->GetLayer (0);
            if (layer->GetFeatureCount () > 0)
            {
                OGRSpatialReference* corineCoordSys = layer->GetSpatialRef ();
                OGRCoordinateTransformation* trafoCorine2Wrf =
                    OGRCreateCoordinateTransformation (corineCoordSys, wrfCoordSys);
                OGRCoordinateTransformation* trafoWrf2Corine =
                    OGRCreateCoordinateTransformation (wrfCoordSys, corineCoordSys);

#ifdef DEBUG2
                for (size_t j = 0; j < 1; ++j)
#else
                for (size_t j = 0; j < wrf.jSize (); ++j)
#endif
                {
                    OGRGeometry* wrfPolygon = wrf.getPolygon (i, j);
                    double wrfArea = ((OGRPolygon*)wrfPolygon)->get_Area ();

                    OGRGeometry* wrfPolygonInCorineCoord =
                        wrfPolygon->clone ();
                    wrfPolygonInCorineCoord->transform (trafoWrf2Corine);
                    layer->SetSpatialFilter (wrfPolygonInCorineCoord);
                    if (layer->GetFeatureCount () > 0)
                    {

                        layer->ResetReading ();
                        OGRFeature* feature;
                        while ((feature = layer->GetNextFeature ()))
                        {
                            OGRGeometry* corinePolygon = feature->GetGeometryRef ();
                            corinePolygon->transform (trafoCorine2Wrf);

                            if (corinePolygon->Intersects (wrfPolygon))
                            {
                                OGRGeometry* intersection =
                                    corinePolygon->Intersection (wrfPolygon);
                                double intersectionArea =
                                    ((OGRPolygon*)intersection)->get_Area ();

                                fractions[i][j].add (type, intersectionArea/wrfArea);

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

#ifdef _OPENMP
    boost::scoped_ptr<omp_lock_t> lock (new omp_lock_t);
    omp_init_lock (lock.get ());
#endif

#ifndef NOOUTPUT
#ifdef DEBUG3
    for (size_t i = 11; i < 12; ++i)
        for (size_t j = 15; j < 16; ++j)
#else
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (size_t i = 0; i < wrf.iSize (); ++i)
        for (size_t j = 0; j < wrf.jSize (); ++j)
#endif
        {

            // map the corine fractions to CLM fractions
            // -----------------------------------------
            clm::ClmFractions clmFractions = fractions[i][j].map2Clm ();

            // if missing is too large, fill with default values from the
            // original WRF file mapped to CLM types
            // ----------------------------------------------------------
            double missing = clmFractions.missing ();
            if (missing > 1.0e-5)
            {

                if (verbosity > 1)
                {
#ifdef _OPENMP
                    omp_set_lock (lock.get ());
#endif
                    cerr << "WARNING: using partly original land use in grid cell "
                         << i << " " << j << " with missing fraction of " << 
                         missing << endl;
#ifdef _OPENMP
                    omp_unset_lock (lock.get ());
#endif
                }

                clm::ClmFractions originalLandUseFractions
                    = wrf.getLandUseFraction (i, j)->map2Clm ();

                for (size_t type = 0; type < clm::typeCount; ++type)
                    clmFractions.set (type, originalLandUseFractions[type]*missing);
                            
            }

#ifdef CHECK
            try
            {
                clmFractions.check ();
            }
            catch (std::exception& e)
            {
                cout << clmFractions << endl;
                throw e;
            }
#endif
            
            // write result to WRF file
            // ------------------------
            wrf.writeClmPftTypeFractions (i, j, clmFractions);
            wrf.writeWaterFraction (i, j, fractions[i][j].getWaterFraction ());
            wrf.writeUrbanFraction (i, j, fractions[i][j].getArtificialFraction ());
            wrf.writeGlacierFraction (i, j, fractions[i][j].getGlacierFraction ());
            wrf.writeWetlandFraction (i, j, fractions[i][j].getWetlandFraction ());
        }
#endif

#ifdef _OPENMP
    omp_destroy_lock (lock.get ());
#endif

}
