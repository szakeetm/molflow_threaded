// Copyright (c) 2011 rubicon IT GmbH

/* Supported grammar :                                           */
/*                                                              */
/* <expr>   ::= <factor> [ <oper1> <factor> ]*                  */
/* <factor> ::= <power>  [ <oper2> <power> ]*                   */
/* <power>  ::= <term> [ <oper3> <term> ]                       */
/* <term>   ::= '(' <exp> ')'      |                            */
/*                <double>         |                            */
/*                <variable>       |                            */
/*               '-' <term>        |                            */
/*               'sin(' <exp> ')'  |                            */
/*               'cos(' <exp> ')'  |                            */
/*               'tan(' <exp> ')'  |                            */
/*               'asin(' <exp> ')' |                            */
/*               'acos(' <exp> ')' |                            */
/*               'atan(' <exp> ')' |                            */
/*               'sinh(' <exp> ')' |                            */
/*               'cosh(' <exp> ')' |                            */
/*               'tanh(' <exp> ')' |                            */
/*               'exp(' <exp> ')'  |                            */
/*               'ln(' <exp> ')'   |                            */
/*               'log(' <exp> ')'  |                            */
/*               'inv(' <exp> ')'  |                            */
/*               'sqrt(' <exp> ')' |                            */
/*               'abs(' <exp> ')'  |                            */
/*               'fact(' <exp> ')'  |                           */
/*               'pow(' <exp>,<exp> ')'  |                      */
/*               'ci95(' <exp>,<exp> ')' |                      */
/*               'sum(' <variable>,<double>,<double> ')' |      */
/*               'PI'                                           */
/* <oper1>    ::= '+' | '-'                                     */
/* <oper2>    ::= '*' | '/'                                     */
/* <oper3>    ::= '^'                                           */
/* <double>   ::= <nb>* '.' <nb> <nb>* ['E' [-] <nb> <nb>*]     */
/* <nb>       ::= '0'..'9'                                      */
/* <variable> ::= <letter>[<letter> | <nb>]*                    */
/* <letter>   ::= 'A'..'Z' | 'a'..'z' | '_'                     */

#ifndef _GLPARSERH_
#define _GLPARSERH_

#include "GLTypes.h" // For bool typedef
#include <string>

// Evaluation tree node type
#define OPER_PLUS   1
#define OPER_MINUS  2
#define OPER_MUL    3
#define OPER_DIV    4
#define OPER_PUIS   5
#define OPER_COS    6
#define OPER_SIN    7
#define OPER_TAN    8
#define OPER_ACOS   9
#define OPER_ASIN   10
#define OPER_ATAN   11
#define OPER_COSH   12
#define OPER_SINH   13
#define OPER_TANH   14
#define OPER_EXP    15
#define OPER_LN     16
#define OPER_LOG2   17
#define OPER_LOG10  18
#define OPER_INV    19
#define OPER_SQRT   20
#define OPER_ABS    21
#define OPER_MINUS1 22
#define OPER_FACT   23
#define OPER_CI95   24
#define OPER_POW    25

#define TDOUBLE     50
#define TVARIABLE   51

// Variable list
typedef struct list {
  char   name[64];
  double value;
  struct list *next;
} VLIST;

//Evaluation tree node info
typedef union {
  double value;
  VLIST *variable;
} ETREE_NODE;

//Evaluation tree node
typedef struct _ETREE {
  int            type;
  ETREE_NODE     info;
  struct _ETREE *left;
  struct _ETREE *right;
} ETREE;

class GLParser {

public:

  GLParser();
  ~GLParser();

  void SetName(const char *name);
  char *GetName();

  // Expression management
  void SetExpression(const char *expr);  // Set formula expression
  char *GetExpression();           // Get the expression
  bool Parse();                    // Compile expression
  char *GetErrorMsg();             // Return error message
  int   GetCurrentPos();           // Current parsing cursor position

  // Variable
  int    GetNbVariable();          // Return num of var created during parsing
  VLIST *GetVariableAt(int idx);   // Return var at specified index
  void   SetVariable(const char *name,double value); // Set the variable value

  std::string GetVariableEvalError();
  void SetVariableEvalError(std::string errMsg);

  // Evaluation
  bool   Evaluate(double *result); // Evaluate the expression
  bool   hasVariableEvalError;
  std::string variableEvalErrorMsg;

private:

  double EvalTree(ETREE *t);
  void   ReadExpression(ETREE **node,VLIST **var_list);
  void   ReadTerm(ETREE **node,VLIST **var_list);
  void   ReadPower(ETREE **node,VLIST **var_list);
  void   ReadFactor(ETREE **node,VLIST **var_list);
  void   ReadVariable( char *name );
  void   ReadDouble(double *R);
  void   AddNode(int type,ETREE_NODE info,ETREE **t,ETREE *left,ETREE *right);
  VLIST *AddVar(const char *var_name,VLIST **l);
  VLIST *FindVar(const char *var_name,VLIST *l);
  void   SetError(const char *err,int p);
  void   AV();
  char  *Extract(int lg);

  char name[256];   // Name
  char errMsg[512]; // Error message
  char expr[4096];  // Expression
  char EC;          // Current char
  int  current;     // Current char index
  bool error;       // Error flag

  VLIST *varList;
  ETREE *evalTree;

};

#endif /* _GLPARSERH_ */