#include "coordinate.h"

Coordinate::Coordinate (const double x, const double y,
        OGRSpatialReference* coordinateSystem)
{
    _x = x;
    _y = y;
    _coordinateSystem = coordinateSystem;
    _coordinateSystem->Reference ();
}

Coordinate::~Coordinate ()
{
    //_coordinateSystem->Release ();
}

const double Coordinate::getX () const
{
    return _x;
}

const double Coordinate::getY () const
{
    return _y;
}

OGRSpatialReference* Coordinate::getCoordinateSystem () const
{
    return _coordinateSystem;
}

const Coordinate Coordinate::transform (OGRSpatialReference* targetCoordinateSystem) const
{
    if (getCoordinateSystem ()->IsSame (targetCoordinateSystem))
        return *this;
    else
    {
        OGRCoordinateTransformation* trafo =
            OGRCreateCoordinateTransformation (_coordinateSystem, targetCoordinateSystem);
        double xNew = getX ();
        double yNew = getY ();
        trafo->Transform (1, &xNew, &yNew);
        OGRCoordinateTransformation::DestroyCT (trafo);
        Coordinate result(xNew, yNew, targetCoordinateSystem);
        return result;
    }
}

ostream& operator<< (ostream& out, const Coordinate coord)
{
    out << coord.getX () << " " << coord.getY ();
    return out;
}

