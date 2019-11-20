/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
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

//#include "File.h"
class FileReader;
#include "Vector.h"

//--- Definitions -----------------------------------------------------------------

#define MAX_NUMBER_OF_OBJECT     4096

#define FLAT_SHADING             1
#define GOURAUD_SHADING          2
#define FLAT_SHADING_GROUP       3
#define GOURAUD_SHADING_GROUP    4

/* ASE Lexical coce */

#define KEYWORD_NB		        81

#define NUMBER					1
#define STRING					2
#define OPEN_BLOCK				3
#define CLOSE_BLOCK				4
#define COLON					5
#define _3DSMAX_ASCIIEXPORT_KW	6
#define COMMENT_KW				7
#define SCENE_KW				8
#define GEOMOBJECT_KW			9
#define NODE_NAME_KW			10
#define NODE_TM_KW				11
#define MESH_KW					12
#define TIMEVALUE_KW			13
#define MESH_NUMVERTEX_KW		14
#define MESH_NUMFACES_KW		15
#define MESH_VERTEX_LIST_KW		16
#define MESH_VERTEX_KW			17
#define MESH_FACE_LIST_KW		18
#define MESH_FACE_KW			19
#define MESH_NUMTVERTEX_KW		20
#define MESH_NORMALS_KW			21
#define MESH_FACENORMAL_KW		22
#define MESH_VERTEXNORMAL_KW	23
#define PROP_MOTIONBLUR_KW		24
#define PROP_CASTSHADOW_KW		25
#define PROP_RECVSHADOW_KW		26
#define MATERIAL_REF_KW         27
#define MESH_TVERTLIST_KW       28
#define MESH_NUMTVFACES_KW      29
#define MESH_TFACELIST_KW       30
#define MATERIAL_LIST_KW		31
#define MATERIAL_COUNT_KW		32
#define MATERIAL_KW				33
#define MATERIAL_NAME_KW		34
#define MATERIAL_CLASS_KW		35
#define MATERIAL_AMBIENT_KW		36
#define MATERIAL_DIFFUSE_KW		37
#define MATERIAL_SPECULAR_KW	38
#define MATERIAL_SHINE_KW		39
#define MATERIAL_SHINESTRENGTH_KW 40
#define MATERIAL_TRANSPARENCY_KW 41
#define MATERIAL_WIRESIZE_KW	42
#define MATERIAL_SHADING_KW		43
#define MATERIAL_XP_FALLOFF_KW	44
#define MATERIAL_SELFILLUM_KW	45
#define MATERIAL_FALLOFF_KW		46
#define MATERIAL_SOFTEN_KW		47
#define MATERIAL_XP_TYPE_KW		48
#define MAP_DIFFUSE_KW			49
#define MAP_NAME_KW				50
#define MAP_CLASS_KW			51
#define MAP_SUBNO_KW			52
#define MAP_AMOUNT_KW			53
#define BITMAP_KW				54
#define MAP_TYPE_KW				55
#define UVW_U_OFFSET_KW			56
#define UVW_V_OFFSET_KW			57
#define UVW_U_TILING_KW			58
#define UVW_V_TILING_KW			59
#define UVW_ANGLE_KW			60
#define UVW_BLUR_KW				61
#define UVW_BLUR_OFFSET_KW		62
#define UVW_NOUSE_AMT_KW		63
#define UVW_NOISE_SIZE_KW		64
#define UVW_NOISE_LEVEL_KW		65
#define UVW_NOISE_PHASE_KW		66
#define BITMAP_FILTER_KW		67
#define MESH_TVERT_KW           68
#define MESH_TFACE_KW           69
#define INHERIT_POS_KW			70
#define INHERIT_ROT_KW			71
#define INHERIT_SCL_KW			72
#define TM_ROW0_KW				73
#define TM_ROW1_KW				74
#define TM_ROW2_KW				75
#define TM_ROW3_KW				76
#define TM_POS_KW				77
#define TM_ROTAXIS_KW			78
#define TM_ROTANGLE_KW			79
#define TM_SCALE_KW				80
#define TM_SCALEAXIS_KW			81
#define TM_SCALEAXISANG_KW		82
#define GROUP_KW				83
#define NODE_PARENT_KW          84
#define WIREFRAME_COLOR_KW      85
#define MAP_OPACITY_KW		    86

static const char *lexical_word[]= {
"NULL",
"NUMBER",
"STRING",
"{",
"}",
"COLON",
"*3DSMAX_ASCIIEXPORT",
"*COMMENT",
"*SCENE",
"*GEOMOBJECT",
"*NODE_NAME",
"*NODE_TM",
"*MESH",
"*TIMEVALUE",
"*MESH_NUMVERTEX",
"*MESH_NUMFACES",
"*MESH_VERTEX_LIST",
"*MESH_VERTEX",
"*MESH_FACE_LIST",
"*MESH_FACE",
"*MESH_NUMTVERTEX",
"*MESH_NORMALS",
"*MESH_FACENORMAL",
"*MESH_VERTEXNORMAL",
"*PROP_MOTIONBLUR",
"*PROP_CASTSHADOW",
"*PROP_RECVSHADOW",
"*MATERIAL_REF",
"*MESH_TVERTLIST",
"*MESH_NUMTVFACES",
"*MESH_TFACELIST",
"*MATERIAL_LIST",
"*MATERIAL_COUNT",
"*MATERIAL",
"*MATERIAL_NAME",
"*MATERIAL_CLASS",
"*MATERIAL_AMBIENT",
"*MATERIAL_DIFFUSE",
"*MATERIAL_SPECULAR",
"*MATERIAL_SHINE",
"*MATERIAL_SHINESTRENGTH",
"*MATERIAL_TRANSPARENCY",
"*MATERIAL_WIRESIZE",
"*MATERIAL_SHADING",
"*MATERIAL_XP_FALLOFF",
"*MATERIAL_SELFILLUM",
"*MATERIAL_FALLOFF",
"*MATERIAL_SOFTEN",
"*MATERIAL_XP_TYPE",
"*MAP_DIFFUSE",
"*MAP_NAME",
"*MAP_CLASS",
"*MAP_SUBNO",
"*MAP_AMOUNT",
"*BITMAP",
"*MAP_TYPE",
"*UVW_U_OFFSET",
"*UVW_V_OFFSET",
"*UVW_U_TILING",
"*UVW_V_TILING",
"*UVW_ANGLE",
"*UVW_BLUR",
"*UVW_BLUR_OFFSET",
"*UVW_NOUSE_AMT",
"*UVW_NOISE_SIZE",
"*UVW_NOISE_LEVEL",
"*UVW_NOISE_PHASE",
"*BITMAP_FILTER",
"*MESH_TVERT",
"*MESH_TFACE",
"*INHERIT_POS",
"*INHERIT_ROT",
"*INHERIT_SCL",
"*TM_ROW0",
"*TM_ROW1",
"*TM_ROW2",
"*TM_ROW3",
"*TM_POS",
"*TM_ROTAXIS",
"*TM_ROTANGLE",
"*TM_SCALE",
"*TM_SCALEAXIS",
"*TM_SCALEAXISANG",
"*GROUP",
"*NODE_PARENT",
"*WIREFRAME_COLOR",
"*MAP_OPACITY"
};

typedef struct {

  int v1;
  int v2;
  int v3;
  int tv1;
  int tv2;
  int tv3;
  Vector3d  n0;
  Vector3d  n1;
  Vector3d  n2;
  Vector3d  n3;

} ASEFACE;

typedef struct {

  double u;
	double v;

} ASEMAP;

typedef struct {

    union {
        struct {
            double        _11, _12, _13, _14;
            double        _21, _22, _23, _24;
            double        _31, _32, _33, _34;
            double        _41, _42, _43, _44;

        };
        double m[4][4];
    };

} ASEMATRIX;

typedef struct {

	  char      name[64];
	  //char      gname[64];
	  int       nb_pts;
	  Vector3d *pts;
	  int       nb_face;
	  ASEFACE  *face;
	  int       nb_map;
	  ASEMAP   *map;
	  int       mat;
	  Vector3d  u;
	  double    angle;
	  //ASEMATRIX rot;

} ASEOBJECT;

// -------------------- Macros -------------------------------------------------------

#define CHECK_LEX(lt,le)  if( lt!=le ) { \
  sprintf(err_str,"line %d, %s expected but got %s.",f->GetCurrentLine(),lexical_word[le],w); \
  throw Error(err_str); \
  }

typedef struct {

  int *indices;
  int nbIndex;

} SIMPLEFACET;

class ASELoader {

public:

  // Construction
  ASELoader(FileReader *f);
  ~ASELoader();

  // Load an ASE file (throw Error in case of failure)
  void Load();

  size_t          nbObj;
  ASEOBJECT   *OBJ;

private:

  FileReader  *f;

  // Parsing stuff
  int   class_lex(char *word);

};