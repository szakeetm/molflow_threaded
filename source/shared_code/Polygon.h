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
#pragma once
#include "Vector.h"
#include <tuple>
#include <optional>
#include <vector>

class GLAppPolygon { //To distinguish from possible other Polygon classes in the namespace
public:
  std::vector<Vector2d>  pts;   // Array of 2D vertex
  //int sign;  // Polygon orientation

};

class PolyVertex  {
public:
  Vector2d  p;       // Vertex coordinates
  int       mark=0;    // Cycle detection (0=>not processed, 1=>processed)
  int       isStart=0; // Possible starting point

  size_t       nbOut=0;  // Number of outgoing arc
  size_t       nbIn=0;   // Number of incoming arc
  int       VI[2];  // Tangent point detection, can be -1
  int       VO[2];  // Tangent point detection, can be -1
};

class PolyArc {
public:

	size_t i1=0;  // Node 1 index
	size_t i2=0;  // Node 2 index
	size_t s=0;   // Source polygon (tangent point detection)

} ;

class PolyGraph {
public:
  std::vector<PolyVertex> nodes;
  std::vector<PolyArc> arcs;
};

bool   IsConvex(const GLAppPolygon& p,size_t idx);
bool   ContainsConcave(const GLAppPolygon& p,int i1,int i2,int i3);
std::tuple<bool,Vector2d>  EmptyTriangle(const GLAppPolygon& p,int i1,int i2,int i3);
bool   IsInPoly(const Vector2d& p,const std::vector<Vector2d>& polyPoints);
bool   IsOnPolyEdge(const double & u, const double & v, const std::vector<Vector2d>& polyPoints, const double & tolerance);
bool   IsOnSection(const double & u, const double & v, const double & baseU, const double & baseV, const double & targetU, const double & targetV, const double & tolerance);
std::optional<std::vector<GLAppPolygon>> IntersectPoly(const GLAppPolygon& p1, const GLAppPolygon& p2,const std::vector<bool>& visible2);
std::tuple<double, Vector2d, std::vector<Vector2d>>  GetInterArea(const GLAppPolygon& inP1,const GLAppPolygon& inP2,const std::vector<bool>& edgeVisible);
std::tuple<double,Vector2d> GetInterAreaBF(const GLAppPolygon& inP1,const Vector2d& p0, const Vector2d& p1);

