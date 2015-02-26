/*
Copyright (c) 2009-2010 Sony Pictures Imageworks Inc., et al.
All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Sony Pictures Imageworks nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** Parser for OpenShadingLanguage 'object' files
 **/


%{

// C++ declarations

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

#include "FlexLexer.h"
extern "C" int yylex(void);


void yyerror (const char *err);
#ifdef __clang__
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif

%}


// This is the definition for the union that defines YYSTYPE
%union
{
    int         i;  // For integer falues
    float       f;  // For float values
    const char *s;  // For string values -- guaranteed to be a ustring.c_str()
}


// Tell Bison to track locations for improved error messages
%locations


// Define the terminal symbols.
%token <s> IDENTIFIER STRING_LITERAL HINT
%token <i> INT_LITERAL
%token <f> FLOAT_LITERAL
%token <i> COLORTYPE FLOATTYPE INTTYPE MATRIXTYPE
%token <i> NORMALTYPE POINTTYPE STRINGTYPE VECTORTYPE VOIDTYPE CLOSURE STRUCT
%token <i> CODE SYMTYPE ENDOFLINE

// Define the nonterminals
%type <i> oso_file version shader_declaration
%type <s> shader_type
%type <i> symbols_opt symbols symbol typespec simple_typename arraylen_opt
%type <i> initial_values_opt initial_values initial_value
%type <i> codemarker label
%type <i> instructions instruction
%type <s> opcode
%type <i> arguments_opt arguments argument
%type <i> jumptargets_opt jumptargets jumptarget
%type <i> hints_opt hints hint
%type <s> hintcontents hintcontents_opt hintcontents_item

// Define the starting nonterminal
%start oso_file


%%

oso_file
        : version shader_declaration symbols_opt codemarker instructions
                {
                    ;
                }
   ;

version
        : IDENTIFIER FLOAT_LITERAL ENDOFLINE
                {
                    std::cout<<"shader version "<<std::endl;
                }
        ;

shader_declaration
        : shader_type IDENTIFIER
                {
                    std::cout<<"IDENTIFIER shader declaration "<<std::endl;
                }
            hints_opt ENDOFLINE
                {
                    ;
                }
        ;

symbols_opt
        : symbols                       { ; }
        | /* empty */                   { ; }
        ;

codemarker
        : CODE IDENTIFIER ENDOFLINE
                {
                    std::cout<<"CODE INDENTIFIER "<<std::endl;
                }
        ;

instructions
        : instruction
        | instructions instruction
        ;

instruction
        : label opcode
                {
                    ;
                }
            arguments_opt jumptargets_opt hints_opt ENDOFLINE
                {
                    ;
                }
        | codemarker
        | ENDOFLINE
        ;

shader_type
        : IDENTIFIER
        ;

symbols
        : symbol
        | symbols symbol
        ;

symbol
        : SYMTYPE typespec arraylen_opt IDENTIFIER
                {
                    ;
                }
            initial_values_opt hints_opt
                {
                    ;
                }
            ENDOFLINE
        | ENDOFLINE
        ;

/* typespec operates by merely setting the current_typespec */
typespec
        : simple_typename
                {
                    ;
                }
        | CLOSURE simple_typename
                {
                    ;
                }
        | STRUCT IDENTIFIER
                {
                    ;
                }
        ;

simple_typename
        : COLORTYPE
        | FLOATTYPE
        | INTTYPE
        | MATRIXTYPE
        | NORMALTYPE
        | POINTTYPE
        | STRINGTYPE
        | VECTORTYPE
        | VOIDTYPE
        ;

arraylen_opt
        : '[' INT_LITERAL ']'           { std::cout<<"INT_LITERAL arraylen_opt "<<std::endl; }
        | /* empty */                   { ; }
        ;

initial_values_opt
        : initial_values
        | /* empty */                   { ; }
        ;

initial_values
        : initial_value
        | initial_values initial_value
        ;

initial_value
        : FLOAT_LITERAL
                {
                    std::cout<<"FLOAT_LITERALL initial value"<<std::endl;
                }
        | INT_LITERAL
                {
                    std::cout<<"INT_LITERAL initial_value "<<std::endl;
                }
        | STRING_LITERAL
                {
                    std::cout<<"STRING_LITERALL initial_value "<<std::endl;
                }
        ;

label
        : INT_LITERAL ':'
        | /* empty */                   {; }
        ;

opcode
        : IDENTIFIER {std::cout<<"INDENTIFIER opcode "<<std::endl;}
        ;

arguments_opt
        : arguments
        | /* empty */                   { ; }
        ;

arguments
        : argument
        | arguments argument
        ;

argument
        : IDENTIFIER
                {
                    std::cout<<"IDENTIFIER argument "<<std::endl;
                }
        ;

jumptargets_opt
        : jumptargets
        | /* empty */                   { ; }
        ;

jumptargets
        : jumptarget
        | jumptargets jumptarget
        ;

jumptarget
        : INT_LITERAL
                {
                    std::cout<<"INT_LITERAL jumptarget "<<std::endl;
                }
        ;

hints_opt
        : hints
        | /* empty */                   { ; }
        ;

hints
        : hint
        | hints hint
        ;

hint
        : HINT hintcontents_opt
                {
                  ;
                }
        ;

hintcontents_opt
        : '{' hintcontents '}'      { ; }
        | /* empty */               { ;}
        ;

hintcontents
        : hintcontents_item
        | hintcontents ',' hintcontents_item
                {
                    ;
                }
        | /* empty */    { ; }
        ;

hintcontents_item
        : INT_LITERAL       { std::cout<<"INT_LITERAL hintcontents_item "<<std::endl; }
        | FLOAT_LITERAL     { std::cout<<"FLOAT_LITERAL hintcontents_item "<<std::endl; }
        | STRING_LITERAL
            {
                std::cout<<"STRING_LITERAL hintcontents_item"<<std::endl;
            }
        | IDENTIFIER arraylen_opt
            {
                std::cout<<"INDENTIFIER hintcontents_item "<<std::endl;
            }
        | simple_typename arraylen_opt
            {
              ;
            }
        ;

%%
void yyerror (const char *err)
{
      std::cerr<<"There was an error!!"<<std::endl;
}

int main (void){
  return yyparse();
}

/*int yylex(void){
  return 1;
}*/


