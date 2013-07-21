#ifndef COORDINATE_H
#define COORDINATE_H

#include <ogr_spatialref.h>
#include <iostream>

using std::ostream;

/**
 * 2D coordinates with informations of the coordinate system attached
 */
class Coordinate
{
  private:
    double x_;
    double y_;
    OGRSpatialReference* coordinateSystem_;
  public:
    Coordinate (double, double, OGRSpatialReference*);
    ~Coordinate();
    const double getX() const;
    const double getY() const;
    OGRSpatialReference* getCoordinateSystem() const;
    const Coordinate transform (OGRSpatialReference*) const;
    friend ostream& operator<< (ostream&, const Coordinate&);
};

#endif
