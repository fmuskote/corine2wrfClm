#ifndef GEORASTER_H
#define GEORASTER_H

#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <string>
#include <boost/multi_array.hpp>
#include "coordinate.h"

class GeoRaster
{
  private:
    void affineTransformation (double, double, double&, double&) const;
    void inverseAffineTransformation (double, double, double&, double&) const;

  protected:
    double               padfTransform_[6];
    double               padfTransformInverse_[6];
    OGRSpatialReference* coordinateSystem_;

  public:
    GeoRaster();
    virtual ~GeoRaster();
    virtual const size_t iSize() const = 0;
    virtual const size_t jSize() const = 0;
    const Coordinate getCoordinate (double, double) const;
    OGRGeometry* getPolygon (size_t, size_t) const;
    OGRGeometry* getCompleteExtend() const;
    OGRSpatialReference* getCoordinateSystem() const;
    void getArrayIndex (const Coordinate, double&, double&) const;
    void writeEmptyGeoTiff (std::string);
    virtual boost::multi_array<float, 2> getClmType (size_t) = 0;
};

class OutOfDomainException {};
class GDALDriverNotFoundException {};

#endif
