%{

#define YYSTYPE double

#include "define.h"

float	yyanswer;
int	yy_indx;

extern std::vector<DATASET_INFO> expSet;


int yyerror(char *s)
{
  fprintf(stderr, "%s\n", s);
  return(0);
}

int yylex ();

%}

%token  NUMBER
%token  VARIABLE
%token  PLUS MINUS TIMES DIVIDE POWER
%token  LEFT_PARENTHESIS RIGHT_PARENTHESIS
%token	SQRT LN LOG EXP SIN COS TAN ABS
%token  END

%left   PLUS    MINUS
%left   TIMES   DIVIDE
%left   NEG
%right  POWER

%start Input

%%

Input:
          /* Empty */
        | Line Input
        ;

Line:
          END
        | Expression END                { yyanswer = $1; }
	| error END                     { yyerrok; }
        ;

Expression:
          NUMBER                        { $$=$1; }
        | VARIABLE                      { $$=expSet[(int)$1].data[yy_indx]; }
        | Expression PLUS Expression    { $$=$1+$3; }
        | Expression MINUS Expression   { $$=$1-$3; }
        | Expression TIMES Expression   { $$=$1*$3; }
        | Expression DIVIDE Expression  { $$=$1/$3; }
        | MINUS Expression %prec NEG    { $$=-$2; }
        | Expression POWER Expression   { $$=pow($1,$3); }
        | SQRT LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS { $$ = sqrt($3); }
        | LN LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS   { $$ = log($3); }
        | LOG LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = log10($3); }
        | EXP LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = exp($3); }
        | SIN LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = sin($3); }
        | COS LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = cos($3); }
        | TAN LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = tan($3); }
        | ABS LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS  { $$ = fabs($3); }
        | LEFT_PARENTHESIS Expression RIGHT_PARENTHESIS { $$=$2; }
        ;


%%
