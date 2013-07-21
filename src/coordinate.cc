#include "coordinate.h"

Coordinate::Coordinate (double x, double y,
        OGRSpatialReference* coordinateSystem)
    : x_ (x),
      y_ (y)
{
    coordinateSystem_ = coordinateSystem;
    coordinateSystem_->Reference();
}

Coordinate::~Coordinate()
{
    //coordinateSystem_->Release();
}

const double Coordinate::getX() const
{
    return x_;
}

const double Coordinate::getY() const
{
    return y_;
}

OGRSpatialReference* Coordinate::getCoordinateSystem() const
{
    return coordinateSystem_;
}

const Coordinate Coordinate::transform (OGRSpatialReference* targetCoordinateSystem) const
{
    if (getCoordinateSystem()->IsSame(targetCoordinateSystem))
        return *this;
    else
    {
        OGRCoordinateTransformation* trafo =
            OGRCreateCoordinateTransformation (coordinateSystem_, targetCoordinateSystem);
        double xNew (getX());
        double yNew (getY());
        trafo->Transform (1, &xNew, &yNew);
        OGRCoordinateTransformation::DestroyCT (trafo);
        Coordinate result(xNew, yNew, targetCoordinateSystem);
        return result;
    }
}

ostream& operator<< (ostream& out, const Coordinate& coord)
{
    out << coord.getX() << " " << coord.getY();
    return out;
}

