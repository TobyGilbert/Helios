%{
// C++ declarations
#include <iostream>
#include <vector>
#include <string>
#include "OSLCompiler/OsoReader.h"
extern int yylex();
#include "FlexLexer.h"
void yyerror(char *s);
/*#ifdef __clang__
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif*/
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
                    OsoReader* reader = getOsoReader();
                    reader->version($2, 0.0);
                }
        ;

shader_declaration
        : shader_type IDENTIFIER
                {
                    OsoReader* reader = getOsoReader();
                    reader->shader($1, $2);
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
                }
        ;

instructions
        : instruction
        | instructions instruction
        ;

instruction
        : label opcode
                {
                    OsoReader* reader = getOsoReader();
                    int lineNo = reader->getLineNo();
                    reader->instruction($2, lineNo);
                    reader->incrimentLineNo();
                }
            arguments_opt jumptargets_opt hints_opt ENDOFLINE
                {
                }
        | codemarker
        | ENDOFLINE
        ;

shader_type
        : IDENTIFIER
                {
                }
        ;

symbols
        : symbol
        | symbols symbol
        ;

symbol
        : SYMTYPE typespec arraylen_opt IDENTIFIER
                {
                    OsoReader* reader = getOsoReader();
                    Type t;
                    if ($2 == FLOATTYPE){
                       t = TypeFloat;
                    }
                    else if ($2 == INTTYPE){
                       t = TypeInt;
                    }
                    else if ($2 == COLORTYPE){
                       t = TypeColour;
                    }
                    else if ($2 == MATRIXTYPE){
                       t = TypeMatrix;
                    }
                    else if ($2 == STRINGTYPE){
                       t = TypeString;
                    }
                    else if ($2 == NORMALTYPE){
                       t = TypeNormal;
                    }
                    else if ($2 == VECTORTYPE){
                       t = TypeVector;
                    }
                    else if ($2 == POINTTYPE){
                       t = TypePoint;
                    }
                    else if ($2 == VOIDTYPE){
                       t = TypeVoid;
                    }
                    else if ($2 == CLOSURE){
                       t = TypeColour;
                    }
                    reader->addSymbols($1, t, $4);
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
                }
        | STRUCT IDENTIFIER
                {
                }
        ;

simple_typename
        : COLORTYPE
                {
                }
        | FLOATTYPE
                {
                }
        | INTTYPE
                {
                }
        | MATRIXTYPE
                {
                }
        | NORMALTYPE
                {
                }
        | POINTTYPE
                {
                }
        | STRINGTYPE
                {
                }
        | VECTORTYPE
                {
                }
        | VOIDTYPE
                {
                }
        ;

arraylen_opt
        : '[' INT_LITERAL ']'           {
                                          OsoReader* reader = getOsoReader();
                                          reader->makeSymbolArrayType(std::to_string($2));
                                        }
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
                    OsoReader* reader = getOsoReader();
                    reader->addSymbolDefaults(std::to_string($1));
                }
        | INT_LITERAL
                {
                    OsoReader* reader = getOsoReader();
                    reader->addSymbolDefaults(std::to_string($1));
                }
        | STRING_LITERAL
                {
                    OsoReader* reader = getOsoReader();
                    reader->addSymbolDefaults($1);
                }
        ;

label
        : INT_LITERAL ':'
                {
                }
        | /* empty */                   { ; }
        ;

opcode
        : IDENTIFIER
                {
                }
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
                    OsoReader* reader = getOsoReader();
                    reader->instructionArguments($1);
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
                    OsoReader* reader = getOsoReader();
                    reader->instructionArguments(std::to_string($1));
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
                }
        ;

hintcontents_opt
        : '{' hintcontents '}'          { ; }
        | /* empty */                   { ; }
        ;

hintcontents
        : hintcontents_item
        | hintcontents ',' hintcontents_item
                 {
                    ;
                 }
        | /* empty */                   { ; }
        ;

hintcontents_item
        : INT_LITERAL
                {
                }
        | FLOAT_LITERAL
                {
                }
        | STRING_LITERAL
                {
                }
        | IDENTIFIER arraylen_opt
                {
                }
        | simple_typename arraylen_opt
                {
                }
        ;

%%
void yyerror(char *s) {
   std::cout << "EEK, parse error!  Message: " << s << std::endl;
   // might as well halt now:
   exit(-1);
}




