 /* Define regular expression macros
  ************************************************/

 /* white space, not counting newline */
WHITE           [ \t\v\f\r]+
 /* alpha character */
ALPHA           [A-Za-z]
 /* numerals */
DIGIT           [0-9]
 /* Integer literal */
INTEGER         [-+]?{DIGIT}+
 /* floating point literal (E, FLT1, FLT2, FLT3 are just helpers) */
E               [eE][-+]?{DIGIT}+
FLT1            [-+]?{DIGIT}+\.{DIGIT}*{E}?
FLT2            [-+]?{DIGIT}*\.{DIGIT}+{E}?
FLT3            [-+]?{DIGIT}+{E}
FLT             {FLT1}|{FLT2}|{FLT3}
 /* string literal */
STR     \"(\\.|[^\\"])*\"
 /* Identifier: alphanumeric, may contain digits after the first character.
  * Also '$' and '.' are allowed!
  */
IDENT           ({ALPHA}|[_$])({ALPHA}|{DIGIT}|[_$\.])*
 /* C preprocessor (cpp) directives */
COMMENT         \#[^\n]*\n
 /* Hints */
HINTPATTERN     \%{IDENT}


 /* Note for lex newbies: the following '%{ .. %}' section contains literal
  * C code that will be inserted at the top of code that flex generates.
  */
%{
#include <iostream>
#include <vector>
#include <string>
#include "OsoReader.h"
#include "y.tab.hpp"

extern YYSTYPE yylval;
%}

/* Declare modes */
%s DECLARATION
%%

{COMMENT}               { ; }

<DECLARATION>"closure"               {
                            return (yylval.i=CLOSURE);
                        }
<DECLARATION>"color"                 {
                            return (yylval.i=COLORTYPE);
                        }
<DECLARATION>"float"                 {
                            return (yylval.i=FLOATTYPE);
                        }
<DECLARATION>"int"                   {
                            return (yylval.i=INTTYPE);
                        }
<DECLARATION>"matrix"                {
                            return (yylval.i=MATRIXTYPE);
                        }
<DECLARATION>"normal"                {
                            return (yylval.i=NORMALTYPE);
                        }
<DECLARATION>"point"                 {
                            return (yylval.i=POINTTYPE);
                        }
<DECLARATION>"string"                {
                            return (yylval.i=STRINGTYPE);
                        }
<DECLARATION>"struct"                {
                            return (yylval.i=STRUCT);
                        }
<DECLARATION>"vector"                {
                            return (yylval.i=VECTORTYPE);
                        }

^local                  {
                            BEGIN(DECLARATION);
                            return SYMTYPE;
                        }

^temp                   {
                            BEGIN(DECLARATION);
                            return SYMTYPE;
                        }

^global                 {
                            return SYMTYPE;
                        }

^param                  {
                            BEGIN(DECLARATION);
                            return SYMTYPE;
                        }

^oparam                 {
                            BEGIN(DECLARATION);
                            return SYMTYPE;
                        }

^const                  {
                            BEGIN(DECLARATION);
                            return SYMTYPE;
                        }

^code                   {
                            BEGIN(INITIAL);
                            return CODE;
                        }

{IDENT}	               {
                            char *res = new char[strlen(yytext) + 1];
                            strcpy(res, yytext);
                            yylval.s = res;
                            return IDENTIFIER;
                        }

{INTEGER}               {
                            yylval.i = atoi(yytext);
                            return INT_LITERAL;
                        }

{FLT}                   {
                            yylval.f = atof(yytext);
                            return FLOAT_LITERAL;
                        }

{STR}                   {
                            char *res = new char[strlen(yytext) + 1];
                            strcpy(res, yytext);
                            yylval.s = res;
                            return STRING_LITERAL;
                        }

{HINTPATTERN}           {
                            char *res = new char[strlen(yytext) + 1];
                            strcpy(res, yytext);
                            yylval.s = res;
                            return HINT;
                        }

{WHITE}                 {
                           ;
                        }

"\\\n"                  {
                            return ENDOFLINE;
                        }

[\n]                    {
                            return ENDOFLINE;
                        }

.                       {
                            return yytext[0];
                        }

%%
