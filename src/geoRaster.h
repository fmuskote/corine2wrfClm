#ifndef GEORASTER_H
#define GEORASTER_H

#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <string>
#include <boost/multi_array.hpp>
#include "coordinate.h"
#include "clm.h"

class GeoRaster
{
  private:
    void affineTransformation (const double, const double,
            double *, double *) const;
    void inverseAffineTransformation (const double, const double,
            double *, double *) const;

  protected:
    double               _padfTransform[6];
    double               _padfTransformInverse[6];
    OGRSpatialReference* _coordinateSystem;

  public:
    GeoRaster ();
    ~GeoRaster ();
    virtual const size_t iSize () const = 0;
    virtual const size_t jSize () const = 0;
    const Coordinate getCoordinate (const double, const double) const;
    OGRGeometry* getPolygon2 (const size_t, const size_t) const;
    OGRGeometry* getCompleteExtend () const;
    OGRSpatialReference* getCoordinateSystem () const;
    void getArrayIndex (const Coordinate, double *, double *) const;
    void writeEmptyGeoTiff (const std::string);
    virtual boost::multi_array<float, 2> getClmType (const size_t) = 0;
};

class OutOfDomainException {};
class GDALDriverNotFoundException {};

#endif
