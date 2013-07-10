#include <gdal_priv.h>
#include "geoRaster.h"
#include "clm.h"

using namespace std;

GeoRaster::GeoRaster ()
{
    _coordinateSystem = new OGRSpatialReference ();
}

GeoRaster::~GeoRaster ()
{
    _coordinateSystem->Release ();
}
    
const Coordinate GeoRaster::getCoordinate (const double i, const double j) const
{
    double x, y;
    affineTransformation (i, j, &x, &y);
    Coordinate result (x, y, _coordinateSystem);
    return result;
}

void GeoRaster::affineTransformation (const double i, const double j,
        double * x, double * y) const
{
    *x = _padfTransform[0] + i*_padfTransform[1] + j*_padfTransform[2];
    *y = _padfTransform[3] + i*_padfTransform[4] + j*_padfTransform[5];
}

void GeoRaster::inverseAffineTransformation (const double x, const double y,
	double * i, double * j) const
{
    *i = _padfTransformInverse[0] + x*_padfTransformInverse[1] + y*_padfTransformInverse[2];
    *j = _padfTransformInverse[3] + x*_padfTransformInverse[4] + y*_padfTransformInverse[5];
}

OGRGeometry* GeoRaster::getPolygon2 (const size_t i, const size_t j) const
{
    if (i > iSize () or j > jSize ())
        throw OutOfDomainException ();

    OGRPolygon* result = new OGRPolygon ();
    result->assignSpatialReference (getCoordinateSystem ());

    OGRLinearRing* ring = new OGRLinearRing ();
    double x, y;
    affineTransformation ((double)i - 0.5, (double)j - 0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation ((double)i + 0.5, (double)j - 0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation ((double)i + 0.5, (double)j + 0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation ((double)i - 0.5, (double)j + 0.5, &x, &y);
    ring->addPoint (x, y);

    ring->closeRings ();
    result->addRingDirectly (ring);
    return (OGRGeometry*)result;
}

OGRGeometry* GeoRaster::getCompleteExtend () const
{
    OGRPolygon* result = new OGRPolygon ();
    result->assignSpatialReference (getCoordinateSystem ());

    OGRLinearRing* ring = new OGRLinearRing ();
    double x, y;
    affineTransformation (-0.5, -0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation ((double)iSize () + 0.5, -0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation ((double)iSize () + 0.5, (double)jSize () + 0.5, &x, &y);
    ring->addPoint (x, y);
    affineTransformation (-0.5, (double)jSize () + 0.5, &x, &y);
    ring->addPoint (x, y);

    ring->closeRings ();
    result->addRingDirectly (ring);
    return (OGRGeometry*)result;
}

OGRSpatialReference* GeoRaster::getCoordinateSystem () const
{
    return _coordinateSystem;
}

void GeoRaster::getArrayIndex (const Coordinate coord, double * i, double * j) const
{
    Coordinate coordInMySystem = coord.transform (getCoordinateSystem ());
    inverseAffineTransformation (coordInMySystem.getX (),
	    coordInMySystem.getY (), i, j);
}

void GeoRaster::writeEmptyGeoTiff (const string fileName)
{
    GDALAllRegister ();

    GDALDriver* driver;
    if (!(driver = GetGDALDriverManager ()->GetDriverByName ("GTiff")))
        throw GDALDriverNotFoundException ();

    GDALDataset* dataSet;
    char** options = NULL;
    dataSet = driver->Create (fileName.c_str (), iSize (), jSize (), clm::typeCount,
            GDT_Float32, options);

    dataSet->SetGeoTransform (_padfTransform);

    char *pszSRS_WKT = NULL;
    _coordinateSystem->exportToWkt( &pszSRS_WKT );
    dataSet->SetProjection( pszSRS_WKT );
    CPLFree( pszSRS_WKT );

    for (size_t type = 0; type < clm::typeCount; type++)
    {
        float* data = getClmType (type);

        GDALRasterBand* band = dataSet->GetRasterBand(type+1);
        band->RasterIO (GF_Write, 0, 0, iSize (), jSize (),
                          data, iSize (), jSize (), GDT_Float32, 0, 0);

        delete[] data;
    }

    GDALClose ((GDALDatasetH)dataSet);
}
