%{

#define YYSTYPE double

#include "exp.tab.h"
#include <stdlib.h>

int yyparse(void);

float scanit(char s[], int indx)
{
  YY_BUFFER_STATE     ybs;
  extern float	yyanswer;
  extern int	yy_indx;

  yy_indx = indx;

  ybs = yy_scan_string(s);
  yyparse();
  yy_delete_buffer(ybs);

  return(yyanswer);

}


%}

white           [ \t]+


digit           [0-9]
integer         {digit}+
exponant        [eE][+-]?{integer}

real            {integer}("."{integer})?{exponant}?

%%

{white}         { /* We ignore white characters */ }

{real}          {
                  yylval=atof(yytext);
                  return(NUMBER);
                }

[A-P]		{ yylval = yytext[0] - 'A'; return(VARIABLE); }
"+"           return(PLUS);
"-"           return(MINUS);

"*"           return(TIMES);
"/"           return(DIVIDE);

"^"           return(POWER);

"("           return(LEFT_PARENTHESIS);
")"           return(RIGHT_PARENTHESIS);

sqrt    return SQRT;
ln      return LN;
log     return LOG;
exp     return EXP;
sin     return SIN;
cos     return COS;
tan     return TAN;
abs     return ABS;

"\0"  return(END);
"\n"  return(END);

