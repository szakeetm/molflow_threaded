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
#include "ASELoader.h"
#include "File.h"
//#include "Vector.h"
#include <math.h>
#include <string.h>
#include <cstring> //strcpy, etc.
//#include <malloc.h>

#define SAFE_FREE(x) if(x) { free(x);x=NULL; }

// Construction

ASELoader::ASELoader(FileReader *f) {

  this->f = f;
  OBJ = NULL;
  nbObj = 0;

}

ASELoader::~ASELoader() {

  for(size_t i=0;i<nbObj;i++) {
    SAFE_FREE(OBJ[i].pts);
    SAFE_FREE(OBJ[i].face);
    SAFE_FREE(OBJ[i].map);
  }
  SAFE_FREE(OBJ);
  nbObj = 0;

}

/************************************************************/
/* return the lexical classe of the next word               */
/************************************************************/
int ASELoader::class_lex(char *word) {

  int i,found=0;

  /* exepction */

  if(word==NULL) return 0;
  if(strlen(word)==0) return STRING;

  /* Keywords */
  if( word[0] == '*' ) {
    i=6;
    while(!found && i<6+KEYWORD_NB) {
      found = (strcmp( word , lexical_word[i] )==0);
      if(!found) i++;
    }
    if( found ) return i;
  }

  /* Special character */

  if(strcmp(word,"{")==0) return OPEN_BLOCK;
  if(strcmp(word,"}")==0) return CLOSE_BLOCK;
  if(strcmp(word,":")==0) return COLON;

  /* number */
  if( (word[0]<='9' && word[0]>='0') || (word[0]=='.') || (word[0]=='-') ) return NUMBER;

  return STRING;
}

/***********************************/
/* Load a ASE file (throw Error)   */
/***********************************/
void ASELoader::Load()
{
  char   *w;
  int    eof=0;
  int    i,nb,lex,lex2,lex3,lex4,map_type;
  char   err_str[1024];
  int    nb_mat=0;

  typedef struct {
    char diff_name[128];
    char alpha_name[128];
    char ref[64];
  } MAT;
  MAT materials[200];

  for(i=0;i<200;i++)
  {
    strcpy(materials[i].diff_name,"none");
    strcpy(materials[i].alpha_name,"none");
    strcpy(materials[i].ref,"none");
  }

  ASEMATRIX m;
  m._12 = m._13 = m._14 = m._21 = m._23 = m._24 = 0.0f;
  m._31 = m._32 = m._34 = m._41 = m._42 = m._43 = 0.0f;
  m._11 = m._22 = m._33 = m._44 = 1.0f;

  OBJ = (ASEOBJECT *)malloc( MAX_NUMBER_OF_OBJECT * sizeof(ASEOBJECT) );
  memset(OBJ,0,MAX_NUMBER_OF_OBJECT * sizeof(ASEOBJECT));

  /**********************************/
  /* CHECK BEGINING OF CONFIG FILE  */
  /**********************************/
  w=f->ReadWord();
  if( strlen(w)==0 ) 
    throw Error("File empty");
  lex = class_lex(w);

  /**********************************/
  /* PARSE                          */
  /**********************************/
  while( !eof )
  {
    switch(lex) {

      // Header section
    case _3DSMAX_ASCIIEXPORT_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,NUMBER);
      break;

      // Comment section
    case COMMENT_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,STRING);
      break;

      // Scene section 
    case SCENE_KW:
      w=f->ReadWord(); lex2=class_lex(w);
      CHECK_LEX(lex2,OPEN_BLOCK);
      while( lex2!=CLOSE_BLOCK )
      {
        w=f->ReadWord(); lex2=class_lex(w);
        if(  strlen(w)==0  )
          throw Error("Unexpected end of file '}' is missing.");
      }
      break;

      // Meterial section
    case MATERIAL_LIST_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,OPEN_BLOCK);
      w=f->ReadWord(); lex2=class_lex(w);
      CHECK_LEX(lex2,MATERIAL_COUNT_KW);
      // Number of material
      nb_mat = f->ReadInt();

      for( i=0 ; i < nb_mat ; i++) {
        w=f->ReadWord(); lex2=class_lex(w);
        CHECK_LEX(lex2,MATERIAL_KW);
        w=f->ReadWord(); lex2=class_lex(w);
        CHECK_LEX(lex2,NUMBER);
        w=f->ReadWord(); lex2=class_lex(w);
        CHECK_LEX(lex2,OPEN_BLOCK);
        w=f->ReadWord(); lex3=class_lex(w);
        while( lex3 != CLOSE_BLOCK ) {
          switch( lex3 ) {

    case MATERIAL_NAME_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      strcpy( materials[i].ref , w );
      break;

    case MATERIAL_CLASS_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      break;

    case MATERIAL_AMBIENT_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;

    case MATERIAL_DIFFUSE_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;

    case MATERIAL_SPECULAR_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;

    case MATERIAL_SHINE_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_SHINESTRENGTH_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_TRANSPARENCY_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_WIRESIZE_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_SHADING_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      break;

    case MATERIAL_XP_FALLOFF_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_SELFILLUM_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MATERIAL_FALLOFF_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      break;

    case MATERIAL_SOFTEN_KW:
      break;

    case MATERIAL_XP_TYPE_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      break;

    case MAP_DIFFUSE_KW:
    case MAP_OPACITY_KW:
      map_type=lex3;
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      while( lex4 != CLOSE_BLOCK ) {
        switch( lex4 ) {
    case MAP_NAME_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,STRING);
      break;
    case MAP_CLASS_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,STRING);
      break;
    case MAP_SUBNO_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case MAP_AMOUNT_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case BITMAP_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,STRING);
      if( map_type==MAP_DIFFUSE_KW ) 
        strcpy( materials[i].diff_name , w );
      else
        strcpy( materials[i].alpha_name , w );
      break;
    case MAP_TYPE_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,STRING);
      break;
    case UVW_U_OFFSET_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_V_OFFSET_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_U_TILING_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_V_TILING_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_ANGLE_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_BLUR_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_BLUR_OFFSET_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_NOUSE_AMT_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_NOISE_SIZE_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_NOISE_LEVEL_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case UVW_NOISE_PHASE_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;
    case BITMAP_FILTER_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,STRING);
      break;
    default:
      sprintf(err_str,"Unexpected keyword %s in *MAP_DIFFUSE section at line %d."
        ,w,f->GetCurrentLine());
      throw Error(err_str);
        }
        w=f->ReadWord(); lex4=class_lex(w);
      }
      break;     

    default:
      sprintf(err_str,"Unexpected keyword %s\n*MATERIAL[%s] section\nat line %d."
        ,w,materials[i].ref,f->GetCurrentLine());
      throw Error(err_str);
          }
          w=f->ReadWord(); lex3=class_lex(w);
        }
      }

      w=f->ReadWord(); lex2=class_lex(w);
      CHECK_LEX(lex2,CLOSE_BLOCK);

      break;

      //Group
    case GROUP_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,STRING);
      //get_string(cur_group,w);     
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,OPEN_BLOCK);
      break;

    case CLOSE_BLOCK:
      //strcpy(cur_group,"none");
      break;

      //Geometry section

    case GEOMOBJECT_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,OPEN_BLOCK);
      w=f->ReadWord(); lex2=class_lex(w);
      while( lex2!=CLOSE_BLOCK ) {
        switch( lex2 ) {
    case NODE_NAME_KW:
      w=f->ReadWord(); lex=class_lex(w);
      CHECK_LEX(lex,STRING);

      OBJ[nbObj].nb_face=0;
      OBJ[nbObj].nb_pts=0;
      OBJ[nbObj].nb_map=0;
      //strcpy( OBJ[nbObj].gname , cur_group );
      //strcpy( OBJ[nbObj].name , w );
      //OBJ[nbObj].rot = m;
      break;

    case NODE_PARENT_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);
      break;

    case NODE_TM_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,OPEN_BLOCK);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NODE_NAME_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,STRING);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,INHERIT_POS_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,INHERIT_ROT_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,INHERIT_SCL_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROW0_KW);
      //OBJ[nbObj].rot._11 = f->ReadDouble();
      //OBJ[nbObj].rot._12 = f->ReadDouble();
      //OBJ[nbObj].rot._13 = f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROW1_KW);
      //OBJ[nbObj].rot._21 = f->ReadDouble();
      //OBJ[nbObj].rot._22 = f->ReadDouble();
      //OBJ[nbObj].rot._23 = f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROW2_KW);
      //OBJ[nbObj].rot._31 = f->ReadDouble();
      //OBJ[nbObj].rot._32 = f->ReadDouble();
      //OBJ[nbObj].rot._33 = f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();
      f->ReadDouble();

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROW3_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_POS_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROTAXIS_KW);
      OBJ[nbObj].u.x = f->ReadDouble();
      OBJ[nbObj].u.y = f->ReadDouble();
      OBJ[nbObj].u.z = f->ReadDouble();

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_ROTANGLE_KW);
      OBJ[nbObj].angle = f->ReadDouble();

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_SCALE_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_SCALEAXIS_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);

      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,TM_SCALEAXISANG_KW);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,CLOSE_BLOCK);

      break;
    case MESH_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,OPEN_BLOCK);
      w=f->ReadWord(); lex3=class_lex(w);
      while( lex3!=CLOSE_BLOCK ) {
        switch( lex3 ) {
    case TIMEVALUE_KW:
      w=f->ReadWord(); lex3=class_lex(w);
      CHECK_LEX(lex3,NUMBER);
      break;
    case MESH_NUMVERTEX_KW:
      OBJ[nbObj].nb_pts= f->ReadInt();
      OBJ[nbObj].pts = (Vector3d *)malloc( OBJ[nbObj].nb_pts * sizeof(Vector3d) );
      break;
    case MESH_NUMFACES_KW:
      OBJ[nbObj].nb_face= f->ReadInt();
      OBJ[nbObj].face = (ASEFACE *)malloc( OBJ[nbObj].nb_face * sizeof(ASEFACE) );
      break;
    case MESH_NUMTVERTEX_KW:
      OBJ[nbObj].nb_map= f->ReadInt();
      OBJ[nbObj].map = (ASEMAP *)malloc( OBJ[nbObj].nb_map * sizeof(ASEMAP) );
      break;
    case MESH_NUMTVFACES_KW:
      nb = f->ReadInt();
      if( nb!=OBJ[nbObj].nb_face ) {
        sprintf(err_str,"%s NUM_TVFACE must be equal to NB_FACE line %d.",
          OBJ[nbObj].name,f->GetCurrentLine());
        throw Error(err_str);
      }
      break;

    case MESH_VERTEX_LIST_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      for(i=0;i<OBJ[nbObj].nb_pts;i++) {
        CHECK_LEX(lex4,MESH_VERTEX_KW);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER); // Vertex idx
        OBJ[nbObj].pts[i].x = f->ReadDouble();
        OBJ[nbObj].pts[i].y = f->ReadDouble();
        OBJ[nbObj].pts[i].z = f->ReadDouble();
        w=f->ReadWord(); lex4=class_lex(w);
      }
      CHECK_LEX(lex4,CLOSE_BLOCK);
      break;

    case MESH_FACE_LIST_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      for(i=0;i<OBJ[nbObj].nb_face;i++) {
        CHECK_LEX(lex4,MESH_FACE_KW);
        w=f->ReadWord(); lex4=class_lex(w);
        // Face idx
        CHECK_LEX(lex4,NUMBER);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        // Face vertex 1
        OBJ[nbObj].face[i].v1 = f->ReadInt();

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        // Face vertex 2
        OBJ[nbObj].face[i].v2 = f->ReadInt();

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        // Face vertex 3
        OBJ[nbObj].face[i].v3 = f->ReadInt();

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,COLON);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);

        while(lex4!=STRING) {
          w=f->ReadWord(); lex4=class_lex(w);
        }

        CHECK_LEX(lex4,STRING);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);

        w=f->ReadWord(); lex4=class_lex(w);
      }
      CHECK_LEX(lex4,CLOSE_BLOCK);
      break;

    case MESH_NORMALS_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      for(i=0;i<OBJ[nbObj].nb_face;i++) {
        CHECK_LEX(lex4,MESH_FACENORMAL_KW);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);
        OBJ[nbObj].face[i].n0.x = f->ReadDouble();

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].face[i].n0.y = f->ReadDouble();

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,NUMBER);
        OBJ[nbObj].face[i].n0.z = f->ReadDouble();

        // First normal
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,MESH_VERTEXNORMAL_KW);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].face[i].n1.x = f->ReadDouble();
        OBJ[nbObj].face[i].n1.y = f->ReadDouble();
        OBJ[nbObj].face[i].n1.z = f->ReadDouble();

        // Second normal
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,MESH_VERTEXNORMAL_KW);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].face[i].n2.x = f->ReadDouble();
        OBJ[nbObj].face[i].n2.y = f->ReadDouble();
        OBJ[nbObj].face[i].n2.z = f->ReadDouble();

        // Third
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4,MESH_VERTEXNORMAL_KW);

        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].face[i].n3.x = f->ReadDouble();
        OBJ[nbObj].face[i].n3.y = f->ReadDouble();
        OBJ[nbObj].face[i].n3.z = f->ReadDouble();
        w=f->ReadWord(); lex4=class_lex(w);

      }
      CHECK_LEX(lex4,CLOSE_BLOCK);
      break;

    case MESH_TVERTLIST_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      for(i=0;i<OBJ[nbObj].nb_map;i++) {
        CHECK_LEX(lex4,MESH_TVERT_KW);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].map[i].u = f->ReadDouble();
        OBJ[nbObj].map[i].v = f->ReadDouble();
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        w=f->ReadWord(); lex4=class_lex(w);
      }
      CHECK_LEX(lex4,CLOSE_BLOCK);
      break;

    case MESH_TFACELIST_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,OPEN_BLOCK);
      w=f->ReadWord(); lex4=class_lex(w);
      for(i=0;i<OBJ[nbObj].nb_face;i++) {
        CHECK_LEX(lex4,MESH_TFACE_KW);
        w=f->ReadWord(); lex4=class_lex(w);
        CHECK_LEX(lex4, NUMBER );
        OBJ[nbObj].face[i].tv1 = f->ReadInt();
        OBJ[nbObj].face[i].tv2 = f->ReadInt();
        OBJ[nbObj].face[i].tv3 = f->ReadInt();
        w=f->ReadWord(); lex4=class_lex(w);
      }
      CHECK_LEX(lex4,CLOSE_BLOCK);
      break;

    default:
      sprintf(err_str,"Unexpected keyword %s\n"
        "*MESH[%s] section\n"
        "at line %d"
        ,w,OBJ[nbObj].name,f->GetCurrentLine());
      throw Error(err_str);
        }
        w=f->ReadWord(); lex3=class_lex(w);
      }
      break;

    case PROP_MOTIONBLUR_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;

    case PROP_CASTSHADOW_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;

    case PROP_RECVSHADOW_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      break;

    case MATERIAL_REF_KW:
      OBJ[nbObj].mat= f->ReadInt();
      break;

    case WIREFRAME_COLOR_KW:
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      w=f->ReadWord(); lex4=class_lex(w);
      CHECK_LEX(lex4,NUMBER);
      OBJ[nbObj].mat=-1;
      break;

    default:
      sprintf(err_str,"Unexpected keyword %s\n"
        "*GEOMOBJECT[%s] section\n"
        "at line %d"
        ,w,OBJ[nbObj].name,f->GetCurrentLine());
      throw Error(err_str);
        }
        w=f->ReadWord(); lex2=class_lex(w);
      }
      nbObj++;
      if( nbObj>=MAX_NUMBER_OF_OBJECT )
      {
        sprintf(err_str,"Too many objects.\nkeyword %s\nobject name %s"
          ,w,OBJ[nbObj].name);
        throw Error(err_str);

      }
      break;

    default:
      sprintf(err_str,"Unexpected keyword %s at line %d"
        ,w,f->GetCurrentLine());
      throw Error(err_str);
    }
    w=f->ReadWord(); lex=class_lex(w);
    eof=(strlen(w)==0);
  }

#if 0
  strcpy(cur_group,"none");
  strcpy(sm_group,"none");
  nb_smgroup = 0;

  for(i=0;i<nbObj;i++) {

    int shade_type;
    double progStart = 10.0 + 80.0*(double)i/(double)nbObj;
    double progStep  = 80.0*1.0/(double)nbObj;
    
    nb_opt_face   = OBJ[i].nb_face;
    opt_face      = (optFace *)malloc(nb_opt_face*sizeof(optFace));
    nb_opt_point  = nb_opt_face * 3;
    opt_point     = (optPoint *)malloc(nb_opt_point*sizeof(optPoint));
    opt_idx       = (long *)malloc(nb_opt_point*sizeof(long));
    memset( opt_point , 0 , nb_opt_point*sizeof(optPoint) );
    memset( opt_idx   , 0 , nb_opt_point*sizeof(long) );

    // Build info for smothin group
    if( strncmp( OBJ[i].gname , "sm_" , 3 )==0 ) {
      add_sm_group( OBJ[i].gname , i );
    }

    // Get shading type
    if( strncmp( OBJ[i].name , "box" , 3 )==0 ) {
      shade_type = FLAT_SHADING;
    } else if( strncmp( OBJ[i].name , "sm_" , 3 )==0 ) {
      shade_type = GOURAUD_SHADING;
    } else {
      sprintf(tmp,"!!! Warning !!! %s forced FLAT shading",OBJ[i].name);
      dprintln(tmp);
      shade_type = FLAT_SHADING;
      /*
      if( strcmp(OBJ[i].gname,"none")==0 ) {
        shade_type = GetShade( OBJ[i].name , OBJ[i].gname , 0);
        strcpy(cur_group,"none");
      } else {
        if( strcmp(OBJ[i].gname,cur_group)!=0 ) {
          shade_type = GetShade( OBJ[i].name , OBJ[i].gname , 1);
          if( shade_type == FLAT_SHADING_GROUP ||
            shade_type == GOURAUD_SHADING_GROUP )
          {
            strcpy(cur_group,OBJ[i].gname);
          }
        }
      }
      */
    }

    if( OBJ[i].nb_map==0 ) {
      sprintf(tmp,"No mapping coordinate for %s",OBJ[i].name);
      MessageBox( NULL , tmp  ,
        "[Warning message]" ,
        MB_ICONINFORMATION | MB_APPLMODAL | MB_OK );
    }

    // Fill opt tables

    for( j=0 ; j < nb_opt_face ; j++)
    {

      // Vertex coordinates

      opt_point[3*j+0].x = OBJ[i].pts[OBJ[i].face[j].v1].x;
      opt_point[3*j+0].y = OBJ[i].pts[OBJ[i].face[j].v1].y;
      opt_point[3*j+0].z = OBJ[i].pts[OBJ[i].face[j].v1].z;
      opt_point[3*j+1].x = OBJ[i].pts[OBJ[i].face[j].v2].x;
      opt_point[3*j+1].y = OBJ[i].pts[OBJ[i].face[j].v2].y;
      opt_point[3*j+1].z = OBJ[i].pts[OBJ[i].face[j].v2].z;
      opt_point[3*j+2].x = OBJ[i].pts[OBJ[i].face[j].v3].x;
      opt_point[3*j+2].y = OBJ[i].pts[OBJ[i].face[j].v3].y;
      opt_point[3*j+2].z = OBJ[i].pts[OBJ[i].face[j].v3].z;

      // Normals

      if( shade_type == GOURAUD_SHADING ||
        shade_type == GOURAUD_SHADING_GROUP ) {

          v.x = OBJ[i].face[j].n1.x;
          v.y = OBJ[i].face[j].n1.y;
          v.z = OBJ[i].face[j].n1.z;
          VectorMatrixMultiply(v,OBJ[i].rot,&n);
          opt_point[3*j+0].nx = n.x;
          opt_point[3*j+0].ny = n.y;
          opt_point[3*j+0].nz = n.z;

          v.x = OBJ[i].face[j].n2.x;
          v.y = OBJ[i].face[j].n2.y;
          v.z = OBJ[i].face[j].n2.z;
          VectorMatrixMultiply(v,OBJ[i].rot,&n);
          opt_point[3*j+1].nx = n.x;
          opt_point[3*j+1].ny = n.y;
          opt_point[3*j+1].nz = n.z;

          v.x = OBJ[i].face[j].n3.x;
          v.y = OBJ[i].face[j].n3.y;
          v.z = OBJ[i].face[j].n3.z;
          VectorMatrixMultiply(v,OBJ[i].rot,&n);
          opt_point[3*j+2].nx = n.x;
          opt_point[3*j+2].ny = n.y;
          opt_point[3*j+2].nz = n.z;

        } else {

          v.x = OBJ[i].face[j].n0.x;
          v.y = OBJ[i].face[j].n0.y;
          v.z = OBJ[i].face[j].n0.z;
          VectorMatrixMultiply(v,OBJ[i].rot,&n);
          opt_point[3*j+0].nx = n.x;
          opt_point[3*j+0].ny = n.y;
          opt_point[3*j+0].nz = n.z;
          opt_point[3*j+1].nx = n.x;
          opt_point[3*j+1].ny = n.y;
          opt_point[3*j+1].nz = n.z;		  
          opt_point[3*j+2].nx = n.x;
          opt_point[3*j+2].ny = n.y;
          opt_point[3*j+2].nz = n.z;

        }

        // Mapping
        if( OBJ[i].nb_map==0 ) {
          opt_point[3*j+0].u = 0;
          opt_point[3*j+0].v = 0;
          opt_point[3*j+1].u = 0;
          opt_point[3*j+1].v = 0;
          opt_point[3*j+2].u = 0;
          opt_point[3*j+2].v = 0;

        } else {
          opt_point[3*j+0].u = OBJ[i].map[ OBJ[i].face[j].tv1 ].u;
          opt_point[3*j+0].v = OBJ[i].map[ OBJ[i].face[j].tv1 ].v;
          opt_point[3*j+1].u = OBJ[i].map[ OBJ[i].face[j].tv2 ].u;
          opt_point[3*j+1].v = OBJ[i].map[ OBJ[i].face[j].tv2 ].v;
          opt_point[3*j+2].u = OBJ[i].map[ OBJ[i].face[j].tv3 ].u;
          opt_point[3*j+2].v = OBJ[i].map[ OBJ[i].face[j].tv3 ].v;
        }

        opt_point[3*j+0].mark = -1;
        opt_point[3*j+1].mark = -1;
        opt_point[3*j+2].mark = -1;

    }

    // Compress point array
    idx=0;
    for( j=0 ; j < nb_opt_point ; j++ )
    {
      SetProgress((int)(progStart + progStep*(double)j/(double)nb_opt_point));
      if( opt_point[j].mark == -1 )
      {
        for(k=0 ; k < nb_opt_point ; k++)
        {
          if( equals(opt_point[j] , opt_point[k]) )
          {
            opt_point[k].mark=idx;
            opt_idx[idx]=k;
          }
        }
        idx++;
      }

    }

    // Upadte faces
    for( j=0 ; j < nb_opt_face ; j++)
    {
      opt_face[j].v1 = opt_point[3*j+0].mark;
      opt_face[j].v2 = opt_point[3*j+1].mark;
      opt_face[j].v3 = opt_point[3*j+2].mark;
    }

    // Build the _3DObject

    ret_scn->obj[i].hp = (VERTEX *)malloc( idx * sizeof(VERTEX) );
    ret_scn->obj[i].nbp = idx;
    ret_scn->obj[i].nbi = nb_opt_face*3;
    ret_scn->obj[i].ind = (LPWORD)malloc(nb_opt_face*3*sizeof(WORD));
    if( OBJ[i].mat==-1 ) {
      ret_scn->obj[i].diff_name=strdup( "none" );
      ret_scn->obj[i].alpha_name=strdup( "none" );
    } else {
      ret_scn->obj[i].diff_name=strdup( materials[OBJ[i].mat].diff_name );
      ret_scn->obj[i].alpha_name=strdup( materials[OBJ[i].mat].alpha_name );
    }

    sprintf(tmp,"[%s] : Vertex:%d Face:%d",OBJ[i].name,idx,nb_opt_face);
    dprintln(tmp);

    for(j=0;j<idx;j++)
    {
      k = opt_idx[j];

      ret_scn->obj[i].hp[j].x = opt_point[k].x;//-(float)tx;
      ret_scn->obj[i].hp[j].y = opt_point[k].y;//-(float)ty;
      ret_scn->obj[i].hp[j].z = opt_point[k].z;//-(float)tz;
      ret_scn->obj[i].hp[j].nx = opt_point[k].nx;
      ret_scn->obj[i].hp[j].ny = opt_point[k].ny;
      ret_scn->obj[i].hp[j].nz = opt_point[k].nz;
      ret_scn->obj[i].hp[j].tu = opt_point[k].u;
      ret_scn->obj[i].hp[j].tv = opt_point[k].v;
    }

    for(j=0;j<nb_opt_face;j++)
    {
      ret_scn->obj[i].ind[j*3+0]=(WORD)opt_face[j].v1;
      ret_scn->obj[i].ind[j*3+1]=(WORD)opt_face[j].v2;
      ret_scn->obj[i].ind[j*3+2]=(WORD)opt_face[j].v3;
    }

    // free data

    free( opt_point );
    free( opt_face );
    free( OBJ[i].pts );
    free( OBJ[i].face );
    free( OBJ[i].map );

  }

#endif

}

