%{
// C++ declarations

#include <iostream>
#include <vector>
#include <string>
extern int yylex();
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
                    std::cout<<"shader version "<<$2<<std::endl;
                }
        ;

shader_declaration
        : shader_type IDENTIFIER
                {
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
                    std::cout<<" initial value "<<$1<<std::endl;
                }
        | INT_LITERAL
                {
                    std::cout<<" initial_value "<<$1<<std::endl;
                }
        | STRING_LITERAL
                {
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

extern int yylex();
extern int yyparse();
extern FILE *yyin;

void yyerror(char *s) {
   std::cout << "EEK, parse error!  Message: " << s << std::endl;
   // might as well halt now:
   exit(-1);
}

int main() {
  // open a file handle to a particular file:
  FILE *myfile = fopen("metal.oso", "r");
  // make sure it is valid:
  if (!myfile) {
     std::cout << "I can't open metal.oso!" << std::endl;
     return -1;
  }
  // set lex to read from it instead of defaulting to STDIN:
  yyin = myfile;

  // parse through the input until there is no more:
  do {
     yyparse();
  } while (!feof(yyin));

return 0;

}



