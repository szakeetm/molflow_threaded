//
// Created by pbahr on 09/01/2020.
//

#ifndef MOLFLOW_PROJ_GEOMETRYCONVERTER_H
#define MOLFLOW_PROJ_GEOMETRYCONVERTER_H

#include "Geometry_shared.h"

class GeometryConverter {
    
    static int  FindEar(const GLAppPolygon& p);
	static std::vector<Facet*> Triangulate(Facet *f);
    //void Triangulate(Facet *f);
    static Facet* GetTriangleFromEar(Facet *f, const GLAppPolygon& p, int ear);
public:
	static std::vector<Facet*> GetTriangulatedGeometry(Geometry* geometry);
    static void PolygonsToTriangles(Geometry* geometry);

};


#endif //MOLFLOW_PROJ_GEOMETRYCONVERTER_H
