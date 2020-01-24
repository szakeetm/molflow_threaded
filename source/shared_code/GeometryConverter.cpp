#include "GeometryConverter.h"
//
// Created by pbahr on 09/01/2020.
//

#include <GLApp/MathTools.h>
#include "GeometryConverter.h"
#include "Facet_shared.h"


std::vector<Facet*> GeometryConverter::GetTriangulatedGeometry(Geometry* geometry)
{
	std::vector<Facet*> triangleFacets;
	for (size_t i = 0; i < geometry->sh.nbFacet; i++) {
		size_t nb = geometry->facets[i]->sh.nbIndex;
		if (nb > 3) {
			// Create new triangle facets and invalidate old polygon facet
			std::vector<Facet*> newTriangles = Triangulate(geometry->facets[i]);
			triangleFacets.insert(std::end(triangleFacets), std::begin(newTriangles), std::end(newTriangles));
		}
		else {
			//Copy
			Facet* newFacet = new Facet(geometry->facets[i]->indices.size());
			newFacet->CopyFacetProperties(geometry->facets[i], false);
			triangleFacets.push_back(); 
		}
	}
	return triangleFacets;
}

// Update facet list of geometry by removing polygon facets and replacing them with triangular facets with the same properties
void GeometryConverter::PolygonsToTriangles(Geometry* geometry){
    std::vector<Facet*> triangleFacets = GetTriangulatedGeometry(geometry);

	for (size_t i = 0;i < geometry->sh.nbFacet;i++) {
		SAFE_DELETE(geometry->facets[i]);
	}

    geometry->sh.nbFacet = triangleFacets.size();
    geometry->facets = (Facet **)realloc(geometry->facets, geometry->sh.nbFacet * sizeof(Facet *));

    // Update facet list
    memcpy(geometry->facets,triangleFacets.data(),triangleFacets.size()*sizeof(Facet*));

    // to recalculate various facet properties
    geometry->InitializeGeometry();
}

std::vector<Facet*> GeometryConverter::Triangulate(Facet *f) {

    // Triangulate a facet (rendering purpose)
    // The facet must have at least 3 points
    // Use the very simple "Two-Ears" theorem. It computes in O(n^2).
    std::vector<Facet*> triangleFacets;
    if (f->nonSimple) {
        // Not a simple polygon
        // Abort triangulation
        return triangleFacets;
    }

    // Build a Polygon
    GLAppPolygon p;
    p.pts = f->vertices2;
    //p.sign = f->sign;

    // Perform triangulation
    while (p.pts.size() > 3) {
        int e = FindEar(p);
        //DrawEar(f, p, e, addTextureCoord);

        // Create new triangle facet and copy polygon parameters, but change indices
        Facet* triangle = GetTriangleFromEar(f, p, e);
        triangleFacets.push_back(triangle);

        // Remove the ear
        p.pts.erase(p.pts.begin() + e);
    }

    // Draw the last ear
    Facet* triangle = GetTriangleFromEar(f, p, 0);
    triangleFacets.push_back(triangle);

    //DrawEar(f, p, 0, addTextureCoord);

    return triangleFacets;
}

int  GeometryConverter::FindEar(const GLAppPolygon& p){

    int i = 0;
    bool earFound = false;
    while (i < p.pts.size() && !earFound) {
        if (IsConvex(p, i))
            earFound = !ContainsConcave(p, i - 1, i, i + 1);
        if (!earFound) i++;
    }

    // REM: Theoritically, it should always find an ear (2-Ears theorem).
    // However on degenerated geometry (flat poly) it may not find one.
    // Returns first point in case of failure.
    if (earFound)
        return i;
    else
        return 0;
}

// Return indices to vertices3d for new triangle facet
Facet* GeometryConverter::GetTriangleFromEar(Facet *f, const GLAppPolygon& p, int ear) {

    //Commented out sections: theoretically in a right-handed system the vertex order is inverse
    //However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

    size_t* indices = new size_t[3];

    indices[0] = f->indices[Previous(ear, p.pts.size())];
    indices[1] = f->indices[IDX(ear, p.pts.size())];
    indices[2] = f->indices[Next(ear, p.pts.size())];

    // Create new triangle facet and copy polygon parameters, but change indices
    Facet* triangle = new Facet(3);
    triangle->CopyFacetProperties(f,0);
    for (size_t i = 0; i < 3; i++) {
        triangle->indices[i] = indices[i];
    }
    f->indices.erase(f->indices.begin()+IDX(ear, p.pts.size()));
    delete[] indices;

/*    const Vector2d* p1 = &(p.pts[Previous(ear, p.pts.size())]);
    const Vector2d* p2 = &(p.pts[IDX(ear, p.pts.size())]);
    const Vector2d* p3 = &(p.pts[Next(ear, p.pts.size())]);*/

    return triangle;
}