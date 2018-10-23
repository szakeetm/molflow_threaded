// Copyright (c) 2011 rubicon IT GmbH
#include <stdio.h>
#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <errno.h>
#include "GLParser.h"
#include <cstring> //strcpy, etc.
#include "MathTools.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad *mApp;
#endif

// Utils functions

// Add a character to a string
void stradd(char *s,char c)
{
  int l=(int)strlen(s);
  s[l]=c;
  s[l+1]=0;
}

// free a list
void free_list(VLIST *l) {
  if(l->next!=NULL) free_list(l->next);
  free(l);
}

void safe_free_list(VLIST **l)
{
  if(*l!=NULL) {
    free_list(*l);
    *l=NULL;
  }
}

// free a tree
void free_tree(ETREE *t) {
  if(t->left!=NULL)  free_tree(t->left);
  if(t->right!=NULL) free_tree(t->right);
  free(t);
}

void safe_free_tree(ETREE **t)
{
  if(*t!=NULL) {
    free_tree(*t);
    *t=NULL;
  }
}

// GLParser

GLParser::GLParser() {

 evalTree=NULL;
 varList=NULL;
 strcpy(expr,"");
 strcpy(name,"");
 hasVariableEvalError = false;

}

GLParser::~GLParser() {
  safe_free_tree(&evalTree);
  safe_free_list(&varList);
}

void GLParser::SetName(const char *name) {
  strncpy(this->name,name,256);
}

char *GLParser::GetName() {
  return name;
}

void GLParser::SetExpression(const char *expr) {
  strncpy(this->expr,expr,4096);
}

char *GLParser::GetExpression() {
  return expr;
}

// Extract a part of the string to analyse 
char *GLParser::Extract(int lg)
{
  static char s[128];
  int i;

  s[0]=0;
  for(i=current;i<current+lg;i++)
    stradd(s,expr[i]);

  return s;
}

// Return fisrt significative char in the string to analyse 
void GLParser::AV()
{
  do {
    current++;
    EC=expr[current];
  } while (EC==' ' || EC=='\t');
}

// Set global error
void GLParser::SetError(const char *err,int p) {
  sprintf(errMsg,"%s at %d",err,p);
  error=true;
}

VLIST *GLParser::FindVar(const char *var_name,VLIST *l) {

  VLIST *p = l;
  bool found = false;
  while(!found && p) {
    found = (iequals(p->name,var_name));
    if(!found) p=p->next;
  }
  if( found ) return p;
  else        return NULL;

}

// Add variables into the chained list
VLIST *GLParser::AddVar(const char *var_name,VLIST **l)
{
  // Search if already added
  VLIST *a = FindVar(var_name,*l);
  if(!a) {
    VLIST *elem;
    elem=(VLIST *)malloc(sizeof(VLIST));
    strcpy(elem->name,var_name);
    elem->next=*l;
    *l=elem;
    return elem;
  } else {
    return a;
  }

}

// Add node into the evaluation tree
void GLParser::AddNode( int type , ETREE_NODE info ,
                        ETREE **t,ETREE *left,ETREE *right) {

  ETREE *elem;

  elem=(ETREE *)malloc(sizeof(ETREE));
  elem->type=type;
  elem->info=info;
  elem->left=left;
  elem->right=right;

  *t=elem;
}

// Gramar functions

void GLParser::ReadDouble(double *R)
{
  char S[128];
  char ex[128];
  int  c;
  int  p;
  int nega;
  int n;

  S[0]=0;
  ex[0]=0;
  p=current;

  do {
    stradd(S,EC);
    AV();
  }while ( (EC>='0' && EC<='9') || EC=='.');

  if (EC=='E' || EC=='e') {
    AV();
    nega=false;

    if (EC=='-') {
      nega=true;
      AV();
    }

    while (EC>='0' && EC<='9')
    {
      stradd(ex,EC);
      AV();
    }
    c=sscanf(ex,"%d",&n);
    if (c==0) { SetError("Incorrect exposant in number",p);return; }
    if (nega) { strcat(S,"e-");strcat(S,ex);}
    else      { strcat(S,"e");strcat(S,ex); }
  }
  c=sscanf(S,"%lf",R);
  if (c==0) SetError("Incorrect number",p);
}

void GLParser::ReadVariable( char *name ) {
  int i=0;
  int p;
  p=current;

  name[0]=0;
  do {

    stradd(name,EC);
    AV();
    i++;
    if(i>=63) SetError("Variable name too long",p);

  } while ( (!error) && (
     (EC>='A' && EC<='Z') || 
     (EC>='a' && EC<='z') ||
     (EC>='0' && EC<='9') ||
     (EC=='_') ||
	 (EC=='[') ||
	 (EC==']')));
}

void GLParser::ReadTerm(ETREE **node,VLIST **var_list)
{
  ETREE *l_t=NULL;
  ETREE *r_t=NULL;
  ETREE_NODE elem;
  char v_name[64];
  elem.value = 0.0;
  elem.variable = NULL;

  if(!error)
  switch(EC) {

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':  ReadDouble(&(elem.value));
               if(!error) AddNode( TDOUBLE , elem , node , NULL , NULL);
               break;
         
    case '(' : AV();
               ReadExpression(node,var_list);
               if (EC!=')') SetError(") expected",current);
               AV();
               break;

    
    // unary operator
    

    case '-' :AV();
              ReadTerm(&l_t,var_list);
              AddNode( OPER_MINUS1 , elem , node , l_t , NULL);
        break;

    
    // Math functions
    

    case 'a':
    case 'A': if ( iequals(Extract(4),"abs(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_ABS , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"asin(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_ASIN , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"acos(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_ACOS , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else  if ( iequals(Extract(5),"atan(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_ATAN , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              }
			  else if (iequals(Extract(4), "AVG(") ) {
				  std::string avgExpression;
				  avgExpression += EC;
				  while (EC != ')') {
					  AV();
					  avgExpression += EC;
				  };
				  AV();
				  elem.variable = AddVar(avgExpression.c_str(), var_list);
				  AddNode(TVARIABLE, elem, node, NULL, NULL);
			  }
			  else{
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'S':
    case 's': if ( iequals(Extract(4),"sin(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_SIN , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"sqrt(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_SQRT , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"sinh(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_SINH , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
			  }
			  else if (iequals(Extract(4), "SUM(")) {
				  std::string sumExpression;
				  sumExpression+= EC;
				  while (EC != ')') {
					  AV();
					  sumExpression += EC;
				  } ;
				  AV();
				  elem.variable = AddVar(sumExpression.c_str(), var_list);
				  AddNode(TVARIABLE, elem, node, NULL, NULL);
			  } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'C':
    case 'c': if ( iequals(Extract(4),"cos(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_COS , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"cosh(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_COSH , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } /*else if ( iequals(Extract(5),"ci95(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                if (EC!=',') SetError(", expected",current);
                AV();
                ReadExpression(&r_t,var_list);
                AddNode( OPER_CI95 , elem , node , l_t , r_t);
                if (EC!=')') SetError(") expected",current);
                AV();
              }*/ else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'E':
    case 'e': if ( iequals(Extract(4),"exp(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_EXP , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'F':
    case 'f': if ( iequals(Extract(5),"fact(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_FACT , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'I':
    case 'i': if ( iequals(Extract(4),"inv(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_INV , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;
    
    case 'L':
    case 'l': if ( iequals(Extract(3),"ln(") ) {
                AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_LN , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"log2(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_LOG2 , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(6),"log10(") ) {
                AV();AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_LOG10 , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    case 'T':
    case 't': if ( iequals(Extract(4),"tan(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_TAN , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else if ( iequals(Extract(5),"tanh(") ) {
                AV();AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                AddNode( OPER_TANH , elem , node , l_t , NULL);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    
    // Constants
    

    case 'P':
    case 'p': if ( iequals(Extract(2),"pi") ) {
                AV();AV();
                elem.value=3.14159265358979323846;
                AddNode( TDOUBLE , elem , node , NULL , NULL);
              } else if ( iequals(Extract(4),"pow(") ) {
                AV();AV();AV();AV();
                ReadExpression(&l_t,var_list);
                if (EC!=',') SetError(", expected",current);
                AV();
                ReadExpression(&r_t,var_list);
                AddNode( OPER_POW , elem , node , l_t , r_t);
                if (EC!=')') SetError(") expected",current);
                AV();
              } else {
                ReadVariable(v_name);
                elem.variable=AddVar(v_name,var_list);
                AddNode( TVARIABLE , elem , node , NULL , NULL);
              }
              break;

    default: if( (EC>='A' && EC<='Z') || 
                 (EC>='a' && EC<='z') ||
                 (EC=='_')) 
             {
               ReadVariable(v_name);
               elem.variable=AddVar(v_name,var_list);
               AddNode( TVARIABLE , elem , node , NULL , NULL);
             } else
               SetError("Syntax error",current);
  }

}

void GLParser::ReadPower(ETREE **node,VLIST **var_list)
{
  ETREE *l_t=NULL;
  ETREE *r_t=NULL;
  ETREE_NODE elem;
  elem.value = 0.0;
  elem.variable = NULL;

  if (!error)
  {
    ReadTerm(&l_t,var_list);
    if (EC=='^')
    {
      AV();
      ReadTerm(&r_t,var_list);
      AddNode( OPER_PUIS , elem , node , l_t , r_t );
    } else {
      *node=l_t;
    }
  }
}

void GLParser::ReadFactor(ETREE **node,VLIST **var_list)
{
  ETREE *l_t=NULL;
  ETREE *r_t=NULL;
  ETREE_NODE elem;
  elem.value = 0.0;
  elem.variable = NULL;

  if(!error)
  {
    ReadPower(&l_t,var_list);

    while((EC=='*' || EC=='/') && !error)
    {
      switch(EC) {
      case '*': AV();
                ReadPower(&r_t,var_list);
                AddNode( OPER_MUL , elem , &l_t , l_t , r_t );
                break;

      case '/': AV();
                ReadPower(&r_t,var_list);
                AddNode( OPER_DIV , elem , &l_t , l_t , r_t );
                break;
      }
    }
    *node=l_t;
  }
}

void GLParser::ReadExpression(ETREE **node,VLIST **var_list)
{
  ETREE *l_t=NULL;
  ETREE *r_t=NULL;
  ETREE_NODE elem;
  elem.value = 0.0;
  elem.variable = NULL;

  if(!error)
  {
    ReadFactor(&l_t,var_list);

    while((EC=='+' || EC=='-') && !error)
    {
      switch(EC) {
      case '+': AV();
                ReadFactor(&r_t,var_list);
                AddNode( OPER_PLUS , elem , &l_t , l_t , r_t );
                break;

      case '-': AV();
                ReadFactor(&r_t,var_list);
                AddNode( OPER_MINUS , elem , &l_t , l_t , r_t );
                break;
      }
    }
    *node=l_t;
  }
}

char *GLParser::GetErrorMsg() {
  return errMsg;
}

bool GLParser::Parse()
{
  if( strlen(expr)==0 ) {
    sprintf(errMsg,"Empty expression");
    return false;
  }

  current=0;
  EC=expr[0];
  error=false;
  strcpy(errMsg,"No error");

  safe_free_tree(&evalTree);
  safe_free_list(&varList);

  ReadExpression(&evalTree,&varList);

  if(current != (int)strlen(expr)) 
    SetError("Syntax error",current);

  if(error) {
    safe_free_tree(&evalTree);
    safe_free_list(&varList);
  }

  return !error;
}

double fact(double x) {

  int f = (int)(x+0.5);
  size_t r = 1;
  for(int i=1;i<=f;i++) {
    r = (size_t)i * r;
  }
  return (double)r;

}

double GLParser::EvalTree(ETREE *t) {

  double a,b,r;
  r=0.0;

  if(!error) 
  switch( t->type ) {
   case OPER_PLUS : 
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        r=a+b;
        break;
   case OPER_MINUS: 
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        r=a-b;
        break;
   case OPER_MUL:   
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        r=a*b;
        break;
   case OPER_DIV:   
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        if(b==0.0) {
          error=true;
          sprintf(errMsg,"Divide by 0");
        } else {
          r=a/b;
        }
        break;
   case OPER_PUIS: 
       a=EvalTree(t->left);
       b=EvalTree(t->right);
       r=pow(a,b);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_COS:  
       a=EvalTree(t->left);
       r=cos(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_CI95:  
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        r=1.96*sqrt(a*(1.0-a)/b);
        break;
   case OPER_POW:  
        a=EvalTree(t->left);
        b=EvalTree(t->right);
        r=pow(a,b);
        break;
   case OPER_FACT:  a=EvalTree(t->left);
       r=fact(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_ACOS:  a=EvalTree(t->left);
       r=acos(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_SIN:  a=EvalTree(t->left);
       r=sin(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_ASIN:  a=EvalTree(t->left);
       r=asin(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_COSH:  a=EvalTree(t->left);
       r=cosh(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_SINH:  a=EvalTree(t->left);
       r=sinh(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_EXP:   a=EvalTree(t->left);
       r=exp(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_LN:   a=EvalTree(t->left);
       r=log(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_LOG10:   a=EvalTree(t->left);
       r=log10(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_LOG2:   a=EvalTree(t->left);
       r=log(a)/log(2.0);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_INV:  a=EvalTree(t->left);
       if(a==0.0) {
         error=true;
         sprintf(errMsg,"Divide by 0");
       } else {
         r=1/a;
       }
       break;
   case OPER_SQRT: a=EvalTree(t->left);
       r=sqrt(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_TAN:  a=EvalTree(t->left);
       r=tan(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_ATAN:  a=EvalTree(t->left);
       r=atan(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_TANH:  a=EvalTree(t->left);
       r=tanh(a);
       if( errno!=0 ) {
          error=true;
          strcpy(errMsg,strerror(errno));
       }
       break;
   case OPER_ABS:  a=EvalTree(t->left);
       r=fabs(a);
       break;
   case OPER_MINUS1: a=EvalTree(t->left);
       r=-a;
       break;
   case TDOUBLE:   r=(t->info.value);
       break;
   case TVARIABLE: r=(t->info.variable)->value;
       break;
  }

  return r;
}

int GLParser::GetNbVariable() {
  int nb = 0;
  VLIST *p = varList;
  while(p) {
    nb++;
    p=p->next;
  }
  return nb;
}

VLIST *GLParser::GetVariableAt(int idx) {
  int nb = 0;
  VLIST *p = varList;
  while(nb<idx && p) {
    nb++;
    p=p->next;
  }
  return p;
}

void   GLParser::SetVariable(const char *name,double value) {
  VLIST *v = FindVar(name,varList);
  if( v ) v->value = value;
}

std::string GLParser::GetVariableEvalError()
{
	return variableEvalErrorMsg;
}

void GLParser::SetVariableEvalError(std::string errMsg)
{
	variableEvalErrorMsg = errMsg;
	hasVariableEvalError = true;
}

int GLParser::GetCurrentPos() {
  return current;
}

bool GLParser::Evaluate(double *result)
{
  error=false;
  errno=0;

  /* Evaluate expression */
  
  if(evalTree)
  {
    *result=EvalTree(evalTree);
  } else {
    //sprintf(errMsg,"Parsing failed"); //Already has an error message
    error=true;
  }

  return !error;
}