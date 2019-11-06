/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#include "Polygon.h"
#include "GLApp/MathTools.h"
#include <math.h>
#include <algorithm> //min max

bool IsConvex(const GLAppPolygon &p,size_t idx) {

  // Check if p.pts[idx] is a convex vertex (calculate the sign of the oriented angle)

  size_t i1 = Previous(idx,p.pts.size());
  size_t i2 = IDX(idx, p.pts.size());
  size_t i3 = Next(idx, p.pts.size());

  double d = DET22(p.pts[i1].u - p.pts[i2].u,p.pts[i3].u - p.pts[i2].u,
                   p.pts[i1].v - p.pts[i2].v,p.pts[i3].v - p.pts[i2].v);

  //return (d*p.sign)>=0.0;
  return d <= 0.0;
}

bool ContainsConcave(const GLAppPolygon &p,int i1,int i2,int i3)
{

  // Determine if the specified triangle contains or not a concave point
  size_t _i1 = IDX(i1, p.pts.size());
  size_t _i2 = IDX(i2, p.pts.size());
  size_t _i3 = IDX(i3, p.pts.size());

  const Vector2d& p1 = p.pts[_i1];
  const Vector2d& p2 = p.pts[_i2];
  const Vector2d& p3 = p.pts[_i3];

  int found = 0;
  int i = 0;
  while(!found && i<p.pts.size()) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) {
	  if (IsInPoly(p.pts[i], { p1,p2,p3 }))
        found = !IsConvex(p,i);
    }
    i++;
  }

  return found;

}

std::tuple<bool,Vector2d> EmptyTriangle(const GLAppPolygon& p,int i1,int i2,int i3)
{

  // Determine if the specified triangle contains or not an other point of the poly
	size_t _i1 = IDX(i1, p.pts.size());
	size_t _i2 = IDX(i2, p.pts.size());
	size_t _i3 = IDX(i3, p.pts.size());

    const Vector2d& p1 = p.pts[_i1];
    const Vector2d& p2 = p.pts[_i2];
    const Vector2d& p3 = p.pts[_i3];

  bool found = false;
  size_t i = 0;
  while(!found && i<p.pts.size()) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) { 
	  found = IsInPoly(p.pts[i], { p1,p2,p3 });
    }
    i++;
  }

  return { !found,(1.0 / 3.0)*(p1 + p2 + p3) };

}
bool IsOnPolyEdge(const double & u, const double & v, const std::vector<Vector2d>& polyPoints, const double & tolerance)
{
	bool onEdge = false;
	for (int i = 0;!onEdge && i < polyPoints.size();i++) {
		const double& x1 = polyPoints[i].u;
		const double& y1 = polyPoints[i].v;
		const double& x2 = polyPoints[(i + 1) % polyPoints.size()].u;
		const double& y2 = polyPoints[(i + 1) % polyPoints.size()].v;
		onEdge = IsOnSection(u, v, x1, y1, x2, y2, tolerance);
	}
	return onEdge;
}

bool IsOnSection(const double & u, const double & v, const double & baseU, const double & baseV, const double & targetU, const double & targetV, const double & tolerance)
{
	//Notation from https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	//u=x0
	//v=y0
	//baseU=x1
	//baseV=y1
	//targetU=x2
	//targetV=y2
	double distU = targetU - baseU;
	double distV = targetV - baseV;
	double distance = std::fabs(distV*u - distU*v + targetU*baseV - targetV*baseU) / sqrt(pow(distV, 2) + pow(distU, 2));
	return distance < tolerance;
}

bool IsOnEdge(const Vector2d& p1,const Vector2d& p2,const Vector2d& p)
{

  // Returns 1 wether p lies in [P1P2], 0 otherwise

  double t = Dot(p2-p1,p-p1);

  double n1 = (p - p1).Norme();
  double n2 = (p2 - p1).Norme();

  double c = t / ( n1*n2 );

  // Check that p1,p2 and p are aligned
  if( c != 1.0 )  return 0;

  // Check wether p is between p1 and p2
  t = n1 / n2;
  return (t>=0.0 && t<=1.0);

}

std::optional<size_t> GetNode(const PolyGraph& g, const Vector2d& p)
{

  // Search a node in the polygraph and returns its id (-1 not found)

  int found = 0;
  int i = 0;
  size_t nb = g.nodes.size();

  while(i<nb && !found) {
    found = VertexEqual(g.nodes[i].p,p);
    if(!found) i++;
  }

  if( found )
    return i;
  else
    return std::nullopt;

}

int SearchFirst(const PolyGraph& g)
{

  // Search a not yet processed starting point
  //(Used by IntersectPoly)

  for (int i = 0; i < g.nodes.size(); i++) {
	const PolyVertex& node = g.nodes[i];
	if (node.mark == 0 && node.isStart) return i;
  }
  //Not found
  return -1;
}

size_t AddNode(PolyGraph& g,const Vector2d& p)
{
  
  // Add a node to the polygraph and returns its id

	if (auto i = GetNode(g, p)) return *i;

	else {
		// New node
		PolyVertex newNode;
		newNode.p = p;
		newNode.VI[0] =
		newNode.VI[1] =
		newNode.VO[0] =
		newNode.VO[1] = -1;
		g.nodes.push_back(newNode);
		return g.nodes.size()-1;    
	 }

}

size_t AddArc(PolyGraph& g,const size_t &i1,const size_t &i2,const size_t &source)
{

  // Add an arc to the polygraph and returns its id

	for (size_t i = 0; i < g.arcs.size();i++) {
		if (g.arcs[i].i1==i1 && g.arcs[i].i2==i2) return i;
	  }

	// Not found, new arc
	PolyArc newArc;
	newArc.i1 = i1;
	newArc.i2 = i2;
	newArc.s = source;
	g.arcs.push_back(newArc);
	return g.arcs.size() - 1;

}

void CutArc(PolyGraph& g, size_t idx, size_t ni)
{

  // Cut the arc idx by inserting ni
  if( g.arcs[idx].i1!=ni && g.arcs[idx].i2!=ni ) {
	  size_t tmp = g.arcs[idx].i2;
    g.arcs[idx].i2 = ni;
    AddArc(g,ni,tmp,1);
  }

}

void InsertEdge(PolyGraph& g,const Vector2d& p1,const Vector2d& p2,const int &a0)
{

  // Insert a polygon edge in the polygraph, a0 = first arc to be checked

  if( VertexEqual(p1,p2) )
    // Does not add null arc
    return;

  // Insert nodes
  size_t n1 = AddNode(g,p1);
  size_t n2 = AddNode(g,p2);

  // Check intersection of the new arc with the arcs of the first polygon.
  bool itFound = false;
  int i = a0;
  while(i<g.arcs.size() && !itFound) {

    if( g.arcs[i].s==1 ) {

      const Vector2d& e1 = g.nodes[ g.arcs[i].i1 ].p;
	  const Vector2d& e2 = g.nodes[ g.arcs[i].i2 ].p;
      Vector2d I;

      if( auto intersectPoint = Intersect2D(p1,p2,e1,e2) ) {
		size_t ni = AddNode(g,*intersectPoint);
        InsertEdge(g,p1, *intersectPoint,i+1);
        InsertEdge(g, *intersectPoint,p2,i+1);
        CutArc(g,i,ni);
        itFound = 1;
      }
    
    }
    i++;

  }

  if( !itFound ) AddArc(g,n1,n2,2);

}

PolyGraph CreateGraph(const GLAppPolygon& inP1, const GLAppPolygon& inP2,const std::vector<bool>& visible2)
{

  // Create the polygraph which represent the 2 intersected polygons
  // with their oriented edges.

	size_t MAXEDGE = inP1.pts.size() * inP2.pts.size() + 1;

	PolyGraph g; //result
	g.nodes.resize(inP1.pts.size());
    g.arcs.resize(inP1.pts.size());
	
  // Fill up the graph with the 1st polygon

  for(size_t i=0;i<inP1.pts.size();i++) {
    g.nodes[i].p = inP1.pts[i];
    g.nodes[i].VI[0] = -1;
    g.nodes[i].VI[1] = -1;
    g.nodes[i].VO[0] = -1;
    g.nodes[i].VO[1] = -1;
    g.arcs[i].i1 = i;
    g.arcs[i].i2 = Next(i,inP1.pts.size());
    g.arcs[i].s = 1;
  }

  // Intersect with 2nd polygon
  for(size_t i=0;i<inP2.pts.size();i++)  {
	  size_t i2 = Next(i,inP2.pts.size());
    if( visible2[i] ) {
      //if( inP2.sign < 0 ) {
      //  InsertEdge(g,inP2.pts[i2],inP2.pts[i],0);
      //} else {
        InsertEdge(g,inP2.pts[i],inP2.pts[i2],0);
      //}
    }
  }

  // Remove tangent edge
  for(size_t i=0;i<g.arcs.size();i++) {
    if( (g.arcs[i].s>0) ) {
		size_t j = i+1;
      bool found = false;
      while(j<g.arcs.size() && !found) {
        if( (g.arcs[j].s>0) &&
            (g.arcs[j].i1 == g.arcs[i].i2) &&
            (g.arcs[j].i2 == g.arcs[i].i1) )
        {
          g.arcs[i].s = 0;
          g.arcs[j].s = 0;
		  //found=true??
        }
        if(!found) j++;
      }
    }
  }

  // Fill up successor in the polyvertex array to speed up search
  // of next vertices

  for(size_t i=0;i<g.arcs.size();i++) {
    if(g.arcs[i].s>0) {
		size_t idxO = g.arcs[i].i1;
		size_t idxI = g.arcs[i].i2;
      if( g.nodes[idxI].nbIn<2 ) {
        g.nodes[idxI].VI[ g.arcs[i].s-1 ]=(int)g.arcs[i].i1;
        g.nodes[idxI].nbIn++;
      }
      if( g.nodes[idxO].nbOut<2 ) {
        g.nodes[idxO].VO[ g.arcs[i].s-1 ]=(int)g.arcs[i].i2;
        g.nodes[idxO].nbOut++;
      }
    }
  }

  // Mark starting points (2 outgoing arcs)

  for(size_t i=0;i<g.nodes.size();i++) {
    if( g.nodes[i].nbOut>=2 ) {
      if( g.nodes[i].nbIn>=2 ) {
        
        // Check tangent point
        Vector2d vi1,vi2,vo1,vo2;
        
		// TO DEBUG!!! Causes frequent crashes
		if (g.nodes[i].VI[0] >= 0 && g.nodes[i].VO[0] >= 0 && g.nodes[i].VI[1] >= 0 && g.nodes[i].VO[1] >= 0) {
			vi1 = g.nodes[g.nodes[i].VI[0]].p - g.nodes[i].p;
			vo1 = g.nodes[g.nodes[i].VO[0]].p - g.nodes[i].p;

			vi2 = g.nodes[g.nodes[i].VI[1]].p - g.nodes[i].p;
			vo2 = g.nodes[g.nodes[i].VO[1]].p - g.nodes[i].p;
		}

        double angI  = GetOrientedAngle(vi1,vo1);
        double angII = GetOrientedAngle(vi1,vi2);
        double angIO = GetOrientedAngle(vi1,vo2);

        g.nodes[i].isStart = (angII<angI) || (angIO<angI);

      } else {
        g.nodes[i].isStart = 1;
      }
    }
  }
  return g;
}

bool CheckLoop(const PolyGraph& g)
{

  // The grapth is assumed to not contains starting point
  // 0 => More than one loop
  // 1 => tangent node/egde detected

  const PolyVertex* s = &g.nodes[0];
  const PolyVertex* s0 = s;
  size_t nbVisited=0;
  bool ok = s->nbOut == 1;

  do {
    if( ok ) {
      if( s->VO[0]>=0 )
        s = &g.nodes[s->VO[0]];
      else
        s = &g.nodes[s->VO[1]];
      ok = (s->nbOut == 1);
      nbVisited++;
    }
  } while( (s0 != s) && (ok) );

  // !ok                   => only tangent node
  // nbVisited==g->nbNode  => only tangent edge

    return nbVisited==g.nodes.size();

}

std::optional<std::vector<GLAppPolygon>> IntersectPoly(const GLAppPolygon& inP1,const GLAppPolygon& inP2,const std::vector<bool>& visible2)
{

  // Computes the polygon intersection between p1 and p2.
  // Operates on simple polygon only. (Hole connection segments 
  // must be marked non visible in visible2)
  // Return the number of polygon created (0 on null intersection, -1 on failure)

  size_t MAXEDGE = inP1.pts.size() * inP2.pts.size() + 1;

  // Create polygraph
  PolyGraph g = CreateGraph(inP1,inP2,visible2);

  // Search a divergent point
  

  int vertexId = SearchFirst(g);

  if( vertexId==-1 ) { //Not found

    // Check particular cases

    if( (g.nodes.size()==inP1.pts.size()) && (g.nodes.size()==inP2.pts.size()) ) {
      // P1 and P2 are equal
		auto resultVector = { inP1 };
		return resultVector;
    }

    if( CheckLoop(g) ) {
      // Only tangent edge/point found => null intersection
      return std::nullopt;
    }

    int i;
	bool insideP1 = false;
	bool insideP2 = false;

	std::vector<Vector2d> p1Pts = inP1.pts;
	std::vector<Vector2d> p2Pts = inP2.pts;

    i=0;
    while( i<inP1.pts.size() && !insideP2 ) {
      insideP2 = IsInPoly(inP1.pts[i],p2Pts);
      i++;
    }
    if( insideP2 ) {
      // P1 is fully inside P2
	  auto resultVector = { inP1 };
      return resultVector;
    }

    i=0;
    while( i<inP2.pts.size() && !insideP1 ) {
      insideP1 = IsInPoly(inP2.pts[i],p1Pts);
      i++;
    }
    if( insideP1 ) {
      // P2 is fully inside P1
		auto resultVector = { inP2 };
		return resultVector;
    }

    return std::nullopt;

  }

  // Compute intersection
  int eop;
  Vector2d n1,n2;
  double sine;
  std::vector<GLAppPolygon> polys;
  do {

    // Starts a new polygon
	  GLAppPolygon newPoly;
	  //newPoly.sign = 1;
	  polys.push_back(newPoly);

    eop = 0;
	PolyVertex* s = &g.nodes[vertexId]; //Not a reference, can change
	PolyVertex* s0 = &g.nodes[vertexId];

    while( !eop && polys.back().pts.size()<MAXEDGE ) {

      // Add point to the current polygon
      polys.back().pts.push_back(s->p);
      s->mark = 1;

      // Go to next point
      switch( s->nbOut ) {

        case 1:

          // Next point
          if(s->VO[0]>=0) {
            // On a P1 edge
            s = &g.nodes[s->VO[0]];
          } else {
            // On a P2 edge
            s = &g.nodes[s->VO[1]];
          }
          break;

        case 2:

          if( s->VO[0]==-1 || s->VO[1]==-1 ) {
            //Failure!!! (tangent edge not marked)
            return std::nullopt;
          }
            
          // We have to turn left
          n1 = g.nodes[s->VO[0]].p;
          n2 = g.nodes[s->VO[1]].p;

          sine = DET22(n1.u-(s->p.u),n2.u-(s->p.u),
                       n1.v-(s->p.v),n2.v-(s->p.v));

          if( sine<0.0 )
            // Go to n1
            s = &g.nodes[s->VO[0]];
          else
            // Go to n2
            s = &g.nodes[s->VO[1]];

          break;

        default:
          //Failure!!! (not ended polygon)
          return std::nullopt;

      }

      // Reach start point, end of polygon
      eop = (s0==s); 

    }

    if( !eop ) {
      //Failure!!! (inner cycle found)
      return std::nullopt;
    }

  } while( (vertexId = SearchFirst(g))!=-1 );

  return polys;

}

std::tuple<double,Vector2d,std::vector<Vector2d>> GetInterArea(const GLAppPolygon &inP1,const GLAppPolygon &inP2,const std::vector<bool>& edgeVisible)
{
	Vector2d center(0.0, 0.0);

  auto polys = IntersectPoly(inP1, inP2, edgeVisible);
  if (!(polys)) return { 0.0,center,{} };
  
  // Count number of pts
  size_t nbV = 0;
  for(auto p : *polys)
	  nbV += p.pts.size();
  std::vector<Vector2d> lList(nbV);

  // Area
  size_t nbE = 0;
  double sum = 0.0;
  double A0;
  for(size_t i=0;i<(*polys).size();i++) {
    double A = 0.0;
    for(size_t j=0;j<(*polys)[i].pts.size();j++) {
      size_t j1 = Next(j,(*polys)[i].pts.size());
      A += ((*polys)[i].pts[j].u*(*polys)[i].pts[j1].v - (*polys)[i].pts[j1].u*(*polys)[i].pts[j].v);
      lList[nbE++] = (*polys)[i].pts[j];
    }
    if( i==0 ) A0 = std::fabs(0.5 * A);
    sum += std::fabs(0.5 * A);
  }

  // Centroid (polygon 0)
  for(int j=0;j<(*polys)[0].pts.size();j++) {
    size_t j1 = Next(j,(*polys)[0].pts.size());
    double d = (*polys)[0].pts[j].u*(*polys)[0].pts[j1].v - (*polys)[0].pts[j1].u*(*polys)[0].pts[j].v;
    center = center + ( (*polys)[0].pts[j]+(*polys)[0].pts[j1] )*d;
  }

  return { sum,(1.0 / (6.0*A0))*center,lList };
}

std::tuple<double, Vector2d> GetInterAreaBF(const GLAppPolygon& inP1, const Vector2d& p0, const Vector2d& p1)
{

  // Compute area of the intersection between the (u0,v0,u1,v1) rectangle 
  // and the inP1 polygon using Jordan theorem.
  // Slow but sure, scan by 50x50 matrix and return center

  int step = 50;
  int nbTestHit = 0;
  Vector2d center;
  double ui = (p1.u-p0.u) / (double)step;
  double vi = (p1.v-p0.v) / (double)step;

  for(int i=0;i<step;i++) {
    double uc = p0.u + ui*((double)i+0.5);
    for(int j=0;j<step;j++) {
      double vc = p0.v + vi*((double)j+0.5);
	  Vector2d testPoint(uc, vc);
      if( IsInPoly(testPoint,inP1.pts) ) {
        nbTestHit++;
		center = testPoint;
      }
    }
  }

  return { (p1.u - p0.u)*(p1.v - p0.v)*((double)nbTestHit / (double)(step*step)) , center};

}

bool IsInPoly(const Vector2d &p, const std::vector<Vector2d>& polyPoints) {

	// Fast method to check if a point is inside a polygon or not.
	// Works with convex and concave polys, orientation independent
	int n_updown = 0;
	int n_found = 0;

	size_t nbSizeMinusOne = polyPoints.size() - 1;
	for (size_t j = 0; j < nbSizeMinusOne; j++) {
		const Vector2d& p1 = polyPoints[j];
		const Vector2d& p2 = polyPoints[j+1];

		/*
		if (p1.u < p2.u) {
			 minx = p1.u;
			 maxx = p2.u;
		}
		else {
			 minx = p2.u;
			 maxx = p1.u;
		}
		if (p.u > minx && p.u <= maxx) {
		*/
		
		if (p.u<p1.u != p.u<p2.u) {
			double slope = (p2.v - p1.v) / (p2.u - p1.u);
			if ((slope * p.u - p.v) < (slope * p1.u - p1.v)) {
				n_updown++;
			}
			else {
				n_updown--;
			}
			n_found++;
		}
	}
	//Last point. Repeating code because it's the fastest and this function is heavily used
	const Vector2d& p1 = polyPoints[nbSizeMinusOne];
	const Vector2d& p2 = polyPoints[0];

	/*
	if (p1.u < p2.u) {
			minx = p1.u;
			maxx = p2.u;
		}
		else {
			minx = p2.u;
			maxx = p1.u;
		}
	if (p.u > minx && p.u <= maxx) {
	*/

	if (p.u<p1.u != p.u<p2.u) {
		double slope = (p2.v - p1.v) / (p2.u - p1.u);
		if ((slope * p.u - p.v) < (slope * p1.u - p1.v)) {
			n_updown++;
		}
		else {
			n_updown--;
		}
		n_found++;
	}




    /*int    cn = 0;    // the  crossing number counter

    // loop through all edges of the polygon
    for (int i=0; i<nbSizeMinusOne; i++) {    // edge from V[i]  to V[i+1]
        if (((polyPoints[i].v <= p.v) && (polyPoints[i+1].v > p.v))     // an upward crossing
            || ((polyPoints[i].v > p.v) && (polyPoints[i+1].v <=  p.v))) { // a downward crossing
            // compute  the actual edge-ray intersect x-coordinate
            double vt = (double)(p.v  - polyPoints[i].v) / (polyPoints[i+1].v - polyPoints[i].v);
            if (p.u <  polyPoints[i].u + vt * (polyPoints[i+1].u - polyPoints[i].u)) // P.u < intersect
                ++cn;   // a valid crossing of y=P.y right of P.x
        }
    }
    if (((polyPoints[0].v <= p.v) && (polyPoints[nbSizeMinusOne].v > p.v))     // an upward crossing
        || ((polyPoints[0].v > p.v) && (polyPoints[nbSizeMinusOne].v <=  p.v))) { // a downward crossing
        // compute  the actual edge-ray intersect x-coordinate
        double vt = (double)(p.v  - polyPoints[0].v) / (polyPoints[nbSizeMinusOne].v - polyPoints[0].v);
        if (p.u <  polyPoints[0].u + vt * (polyPoints[nbSizeMinusOne].u - polyPoints[0].u)) // P.u < intersect
            ++cn;   // a valid crossing of y=P.y right of P.x
    }*/

    //return (cn&1);    // 0 if even (out), and 1 if  odd (in)

    /*if(((n_found / 2) & 1) ^ ((n_updown / 2) & 1) != (cn&1)){
        std::ofstream coordinatefile;
        coordinatefile.open("inpolygon.txt", std::ios::out | std::ios::app);
        coordinatefile << (int)(((n_found / 2) & 1) ^ ((n_updown / 2) & 1)) << " | " << (int)(((n_found) & 1) ^ ((n_updown) & 1)) <<
        " = " << n_found << " " << n_updown <<
        " | " <<(int)((n_found) & 1) << " " << (int)((n_updown) & 1) <<
        " | " <<(int)((n_found / 2) & 1) << " " << (int)((n_updown / 2) & 1) <<
        " -- new: " << (int)(cn&1) << " = " << cn <<std::endl;

        coordinatefile.close();
        coordinatefile.open("weird_poly.txt", std::ios::out | std::ios::app);
        coordinatefile << "("<<p.u << " , " <<p.v << ") inside ";
        for(auto vec : polyPoints){
            coordinatefile << "("<<vec.u << " , " <<vec.v << ")";
        }
        coordinatefile << std::endl;
        coordinatefile.close();
    */

    //if (n_updown<0) n_updown = -n_updown; //Not needed, oddity of negative numbers works the same way with (X/2)&1
	//return ((n_found / 2) & 1) ^ ((n_updown / 2) & 1); //(X & 1 means last bit of X is 1 i.e. the number is odd) The full expression means one is even and the other is odd
	return ((n_found / 2) & 1) ^ ((n_updown / 2) & 1);
}

/*
bool IsInPoly(const Vector2d& p, const std::vector<Vector2d>& polyPoints)
{

// 2D polygon "is inside" solving
// Using the "Jordan curve theorem" (we intersect in v direction here)

int n_updown,n_found,j;
double x1,x2,y1,y2,a,minx,maxx;

n_updown=0;
n_found=0;

for (j = 0; j < (int)polyPoints.size()-1; j++) {

x1 = polyPoints[j].u;
y1 = polyPoints[j].v;
x2 = polyPoints[j+1].u;
y2 = polyPoints[j+1].v;

if( x2>x1 ) { minx=x1;maxx=x2; }
else        { minx=x2;maxx=x1; }

if (p.u > minx && p.u <= maxx) {
a = (y2 - y1) / (x2 - x1);
if ((a*(p.u-x1) + y1) < p.v) {
n_updown = n_updown + 1;
} else {
n_updown = n_updown - 1;
}
n_found++;
}

}

// Last point
x1 = polyPoints[j].u;
y1 = polyPoints[j].v;
x2 = polyPoints[0].u;
y2 = polyPoints[0].v;

if( x2>x1 ) { minx=x1;maxx=x2; }
else        { minx=x2;maxx=x1; }

if (p.u > minx && p.u <= maxx) {
a = (y2 - y1) / (x2 - x1);
if ((a*(p.u-x1) + y1) < p.v) {
n_updown = n_updown + 1;
} else {
n_updown = n_updown - 1;
}
n_found++;
}

if (n_updown<0) n_updown=-n_updown;
return (((n_found/2)&1) ^ ((n_updown/2)&1));

}
*/
