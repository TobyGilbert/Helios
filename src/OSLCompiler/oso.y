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
                    std::cout<<"shader version "<<$2<<std::endl;
                }
        ;

shader_declaration
        : shader_type IDENTIFIER
                {
                    OsoReader* reader = getOsoReader();
                    reader->shader($1, $2);
                    std::cout<<"shader name: "<<$2<<std::endl;
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
                    std::cout<<"CODE"<<" IDENTIFIER "<<$2<<std::endl;
                }
        ;

instructions
        : instruction
        | instructions instruction
        ;

instruction
        : label opcode
                {
                    std::cout<<"Dollar 2: "<<$2<<std::endl;
                    OsoReader* reader = getOsoReader();
                    int lineNo = reader->getLineNo();
                    reader->instruction($2, lineNo);
                    reader->incrimentLineNo();
                }
            arguments_opt jumptargets_opt hints_opt ENDOFLINE
                {
                    std::cout<<$2<<std::endl;
                }
        | codemarker
        | ENDOFLINE
        ;

shader_type
        : IDENTIFIER
                {
                    std::cout<<"shader_type: "<<$1<<std::endl;
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
                    std::cout<<"symbol IDENTIFIER: "<<$4;
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
                    std::cout<<"closure ";
                }
        | STRUCT IDENTIFIER
                {
                    std::cout<<"STRUCT "<<$1<<" IDENTIFIER "<<$2<<std::endl;
                }
        ;

simple_typename
        : COLORTYPE
                {
                     std::cout<<"colour: ";
                }
        | FLOATTYPE
                {
                    std::cout<<"float: ";
                }
        | INTTYPE
                {
                    std::cout<<"int: ";
                }
        | MATRIXTYPE
                {
                    std::cout<<"matrix: ";
                }
        | NORMALTYPE
                {
                    std::cout<<"normal: ";
                }
        | POINTTYPE
                {
                    std::cout<<"point: ";
                }
        | STRINGTYPE
                {
                    std::cout<<"string: ";
                }
        | VECTORTYPE
                {
                    std::cout<<"vector: ";
                }
        | VOIDTYPE
                {
                    std::cout<<"void ";
                }
        ;

arraylen_opt
        : '[' INT_LITERAL ']'           {
                                          OsoReader* reader = getOsoReader();
                                          reader->makeSymbolArrayType(std::to_string($2));
                                          std::cout<<"INT_LITERAL arraylen_opt "<<std::endl;
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
                    std::cout<<" initial value "<<$1<<std::endl;
                }
        | INT_LITERAL
                {
                    OsoReader* reader = getOsoReader();
                    reader->addSymbolDefaults(std::to_string($1));
                    std::cout<<" initial_value "<<$1<<std::endl;
                }
        | STRING_LITERAL
                {
                    OsoReader* reader = getOsoReader();
                    reader->addSymbolDefaults($1);
                    std::cout<<" initial_value "<<$1<<std::endl;
                }
        ;

label
        : INT_LITERAL ':'
                {
                    std::cout<<"INT_LITERAL label: "<<$1<<std::endl;
                }
        | /* empty */                   { ; }
        ;

opcode
        : IDENTIFIER
                {
                    std::cout<<"INDENTIFIER opcode "<<$1<<std::endl;
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
                    std::cout<<"IDENTIFIER argument: "<<$1<<std::endl;
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
                    std::cout<<"INT_LITERAL jumptarget "<<$1<<std::endl;
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
                    std::cout<<"HINT "<<$1<<std::endl;
                }
        ;

hintcontents_opt
        : '{' hintcontents '}'          { std::cout<<"{}"<<std::endl; }
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
                    std::cout<<" INT_LITERAL hintcontents_item "<<$1<<std::endl;
                }
        | FLOAT_LITERAL
                {
                    std::cout<<" FLOAT_LITERAL hintcontents_item "<<$1<<std::endl;
                }
        | STRING_LITERAL
                {
                    std::cout<<" STRING_LITERAL hintcontents: "<<$1<<std::endl;
                }
        | IDENTIFIER arraylen_opt
                {
                    std::cout<<"IDENTIFIER hintcontents: "<<$1;
                }
        | simple_typename arraylen_opt
                {
                    ;
                }
        ;

%%
void yyerror(char *s) {
   std::cout << "EEK, parse error!  Message: " << s << std::endl;
   // might as well halt now:
   exit(-1);
}



