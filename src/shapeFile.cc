
#include "shapeFile.h"
#include <iostream>

using std::string;

ShapeFile::ShapeFile (string fileName)
{
    OGRRegisterAll();
    if (!(poDS_ = OGRSFDriverRegistrar::Open (fileName.c_str(), FALSE)))
        throw ShapeFileOpenFileException();
    if (!(layer_ = poDS_->GetLayer (0)))
        throw ShapeFileGetLayerException();
    if (!(coordinateSystem_ = layer_->GetSpatialRef()))
        throw ShapeFileGetSpatialRefException();
    layer_->ResetReading();
}

ShapeFile::~ShapeFile()
{
    OGRDataSource::DestroyDataSource (poDS_);
}

const int ShapeFile::getLayerCount() const
{
    return poDS_->GetLayerCount();
}

const int ShapeFile::getFeatureCount() const
{
    return layer_->GetFeatureCount();
}

void ShapeFile::resetFeatures()
{
    layer_->ResetReading();
}

OGRFeature* ShapeFile::getNextFeature()
{
    return layer_->GetNextFeature();
}

const double ShapeFile::getAreaRatio (OGRGeometry* targetGeometry,
        OGRSpatialReference* targetCoordSys)
{
    resetFeatures();
    OGRSpatialReference* shapeCoordSys =
        getNextFeature()->GetGeometryRef()->getSpatialReference();
//    OGRSpatialReference* targetCoordSys = targetGeometry->getSpatialReference();
    OGRCoordinateTransformation* transformation =
        OGRCreateCoordinateTransformation (shapeCoordSys, targetCoordSys);

    double targetArea = ((OGRPolygon*)targetGeometry)->get_Area();
    double shapeArea = 0.0;

    resetFeatures();
    OGRFeature* feature;
    while ((feature = getNextFeature()))
    {
        OGRGeometry* geometry = feature->GetGeometryRef();
        geometry->transform (transformation);
        if (geometry->Intersects (targetGeometry))
        {
            OGRGeometry* intersection = geometry->Intersection (targetGeometry);
            shapeArea += ((OGRPolygon*)intersection)->get_Area();
            OGRGeometryFactory::destroyGeometry (intersection);
        }

        OGRFeature::DestroyFeature (feature);
    }
    
    OGRCoordinateTransformation::DestroyCT (transformation);
    return shapeArea/targetArea;
}

void ShapeFile::setSpatialFilter (OGRGeometry* filter)
{
    layer_->SetSpatialFilter (filter);
}

