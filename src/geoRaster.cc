#include <gdal_priv.h>
#include "geoRaster.h"
#include "clm.h"

using namespace std;

GeoRaster::GeoRaster()
{
    coordinateSystem_ = new OGRSpatialReference ();
}

GeoRaster::~GeoRaster()
{
    coordinateSystem_->Release ();
}
    
const Coordinate GeoRaster::getCoordinate (double i, double j) const
{
    double x, y;
    affineTransformation (i, j, x, y);
    Coordinate result (x, y, coordinateSystem_);
    return result;
}

void GeoRaster::affineTransformation (double i, double j, double& x, double& y) const
{
    x = padfTransform_[0] + i*padfTransform_[1] + j*padfTransform_[2];
    y = padfTransform_[3] + i*padfTransform_[4] + j*padfTransform_[5];
}

void GeoRaster::inverseAffineTransformation (double x, double y, double& i, double& j) const
{
    i = padfTransformInverse_[0] + x*padfTransformInverse_[1] + y*padfTransformInverse_[2];
    j = padfTransformInverse_[3] + x*padfTransformInverse_[4] + y*padfTransformInverse_[5];
}

OGRGeometry* GeoRaster::getPolygon (size_t i, size_t j) const
{
    if (i > iSize () or j > jSize ())
        throw OutOfDomainException ();

    OGRPolygon* result = new OGRPolygon ();
    result->assignSpatialReference (getCoordinateSystem ());

    OGRLinearRing* ring = new OGRLinearRing ();
    double x, y;
    affineTransformation ((double)i - 0.5, (double)j - 0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation ((double)i + 0.5, (double)j - 0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation ((double)i + 0.5, (double)j + 0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation ((double)i - 0.5, (double)j + 0.5, x, y);
    ring->addPoint (x, y);

    ring->closeRings ();
    result->addRingDirectly (ring);
    return (OGRGeometry*)result;
}

OGRGeometry* GeoRaster::getCompleteExtend() const
{
    OGRPolygon* result = new OGRPolygon();
    result->assignSpatialReference (getCoordinateSystem());

    OGRLinearRing* ring = new OGRLinearRing();
    double x, y;
    affineTransformation (-0.5, -0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation (static_cast<double> (iSize()) + 0.5, -0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation (static_cast<double> (iSize()) + 0.5, static_cast<double> (jSize()) + 0.5, x, y);
    ring->addPoint (x, y);
    affineTransformation (-0.5, static_cast<double> (jSize()) + 0.5, x, y);
    ring->addPoint (x, y);

    ring->closeRings();
    result->addRingDirectly (ring);
    return (OGRGeometry*)result;
}

OGRSpatialReference* GeoRaster::getCoordinateSystem() const
{
    return coordinateSystem_;
}

void GeoRaster::getArrayIndex (const Coordinate coord, double& i, double& j) const
{
    Coordinate coordInMySystem = coord.transform (getCoordinateSystem());
    inverseAffineTransformation(
            coordInMySystem.getX(),
	    coordInMySystem.getY(),
            i, j);
}

void GeoRaster::writeEmptyGeoTiff (string fileName)
{
    GDALAllRegister();

    GDALDriver* driver;
    if (!(driver = GetGDALDriverManager()->GetDriverByName ("GTiff")))
        throw GDALDriverNotFoundException();

    GDALDataset* dataSet;
    char** options = NULL;
    dataSet = driver->Create (fileName.c_str(), iSize(), jSize(), clm::typeCount,
            GDT_Float32, options);

    dataSet->SetGeoTransform (padfTransform_);

    char *pszSRS_WKT = NULL;
    coordinateSystem_->exportToWkt (&pszSRS_WKT);
    dataSet->SetProjection (pszSRS_WKT);
    CPLFree (pszSRS_WKT);

    for (size_t type = 0; type < clm::typeCount; type++)
    {
        boost::multi_array<float, 2> data = getClmType (type);

        GDALRasterBand* band = dataSet->GetRasterBand(type+1);
        band->RasterIO (GF_Write, 0, 0, iSize(), jSize(),
                          data.data(),  iSize(), jSize(), GDT_Float32, 0, 0);
    }

    GDALClose ((GDALDatasetH)dataSet);
}
