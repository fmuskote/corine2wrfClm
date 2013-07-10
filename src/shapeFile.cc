
#include "shapeFile.h"
#include <iostream>

using std::string;

ShapeFile::ShapeFile (const string fileName)
{
    OGRRegisterAll();
    if (!(_poDS = OGRSFDriverRegistrar::Open (fileName.c_str (), FALSE)))
        throw ShapeFileOpenFileException ();
    if (!(_layer = _poDS->GetLayer (0)))
        throw ShapeFileGetLayerException ();
    if (!(_coordinateSystem = _layer->GetSpatialRef ()))
        throw ShapeFileGetSpatialRefException ();
    _layer->ResetReading ();
}

ShapeFile::~ShapeFile ()
{
    OGRDataSource::DestroyDataSource (_poDS);
}

const int ShapeFile::getLayerCount () const
{
    return _poDS->GetLayerCount ();
}

const int ShapeFile::getFeatureCount () const
{
    return _layer->GetFeatureCount ();
}

void ShapeFile::resetFeatures ()
{
    _layer->ResetReading ();
}

OGRFeature* ShapeFile::getNextFeature ()
{
    return _layer->GetNextFeature ();
}

const double ShapeFile::getAreaRatio (OGRGeometry* targetGeometry,
        OGRSpatialReference* targetCoordSys)
{
    resetFeatures ();
    OGRSpatialReference* shapeCoordSys =
        getNextFeature ()->GetGeometryRef ()->getSpatialReference ();
//    OGRSpatialReference* targetCoordSys = targetGeometry->getSpatialReference ();
    OGRCoordinateTransformation* transformation =
        OGRCreateCoordinateTransformation (shapeCoordSys, targetCoordSys);

    double targetArea = ((OGRPolygon*)targetGeometry)->get_Area ();
    double shapeArea = 0.0;

    resetFeatures ();
    OGRFeature* feature;
    while ((feature = getNextFeature ()))
    {
        OGRGeometry* geometry = feature->GetGeometryRef ();
        geometry->transform (transformation);
        if (geometry->Intersects (targetGeometry))
        {
            OGRGeometry* intersection = geometry->Intersection (targetGeometry);
            shapeArea += ((OGRPolygon*)intersection)->get_Area ();
            OGRGeometryFactory::destroyGeometry (intersection);
        }

        OGRFeature::DestroyFeature (feature);
    }
    
    OGRCoordinateTransformation::DestroyCT (transformation);
    return shapeArea/targetArea;
}

void ShapeFile::setSpatialFilter (OGRGeometry* filter)
{
    _layer->SetSpatialFilter (filter);
}

