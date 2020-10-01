%skeleton "lalr1.cc"

%require "3.5"

%define api.parser.class {Parser}
%define api.token.prefix {TOKEN_}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.trace
%define parse.error verbose

%locations
%debug

%code requires
{

#include "ast/Node.h"
#include "ast/Type.h"
#include "ast/Interface.h"
#include "ast/Function.h"
#include "ast/Prototype.h"
#include "ast/If.h"
#include "ast/Loop.h"
#include "ast/While.h"
#include "ast/For.h"


namespace silicon::compiler {
    class Context;
}

}

%param { silicon::compiler::Context &ctx }

%code
{

#include "compiler/Context.h"
#include <iostream>


void yy::Parser::error(const location_type &l, const std::string &m) {
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << std::endl;

    exit(1);
}

namespace yy
{
Parser::symbol_type yylex(silicon::compiler::Context &ctx);
}

}

// --------------------------------------------------
// Tokens
// --------------------------------------------------

%token EOF 0

%token SEMICOLON ";"
%token COLON ":"
%token OPEN_PAREN "("
%token CLOSE_PAREN ")"
%token OPEN_CURLY "{"
%token CLOSE_CURLY "}"

%token <std::string> IDENTIFIER

// --------------------------------------------------
// Tokens -> Literals
// --------------------------------------------------

%token NULL "null"
%token <bool> BOOLEAN_LITERAL "Boolean Literal"
%token <std::string> NUMBER_LITERAL "Number Literal"
%token <std::string> STRING_LITERAL "String Literal"

// --------------------------------------------------
// Tokens -> Keywords
// --------------------------------------------------

%token LET "let"
%token FUNCTION "fn"
%token RETURN "return"
%token IF "if"
%token INTERFACE "interface"
%token ELSE "else"
%token LOOP "loop"
%token WHILE "while"
%token DO "do"
%token FOR "for"
%token BREAK "break"
%token CONTINUE "continue"
%token EXPORT "export"
%token EXTERN "extern"

// --------------------------------------------------
// Tokens -> Operators
// --------------------------------------------------

%token COMMA ","
%token ASSIGN "="
%token STAR "*"
%token SLASH "/"
%token PERCENT "%"
%token PLUS "+"
%token MINUS "-"
%token CARET "^"
%token AND "&"
%token OR "|"
%token STAR_STAR "**"
%token DOUBLE_LESSER "<<"
%token DOUBLE_BIGGER ">>"
%token TRIPLE_BIGGER ">>>"
%token STAR_ASSIGN "*="
%token SLASH_ASSIGN "/="
%token PERCENT_ASSIGN "%="
%token PLUS_ASSIGN "+="
%token MINUS_ASSIGN "-="
%token CARET_ASSIGN "^="
%token AND_ASSIGN "&="
%token OR_ASSIGN "|="
%token STAR_STAR_ASSIGN "**="
%token DOUBLE_LESSER_ASSIGN "<<="
%token DOUBLE_BIGGER_ASSIGN ">>="
%token TRIPLE_BIGGER_ASSIGN ">>>="
%token PLUS_PLUS "++"
%token MINUS_MINUS "--"
%token AND_AND "&&"
%token OR_OR "||"
%token NOT "!"
%token LESSER "<"
%token LESSER_EQUAL "<="
%token EQUAL "=="
%token NOT_EQUAL "!="
%token BIGGER_EQUAL ">="
%token BIGGER ">"
%token TILDE "~"
%token QUESTION_MARK "?"
%token DOT "."
%token DELETE "delete"
%token AS "as"
%token INSTANCE_OF "instanceof"
%token AWAIT "await"
%token TYPE_OF "typeof"
%token NEW "new"
%token TRIPLE_DOT "..."

// --------------------------------------------------
// Types
// --------------------------------------------------

%type<std::vector<silicon::ast::Node *>> statements scoped_statements scoped_statements_ arguments arguments_
%type<std::vector<silicon::ast::Node *>> variable_definitions

%type<silicon::ast::Node *> statement export_statement extern_statement if_statement expression expression_ operation
%type<silicon::ast::Node *> binary_operation unary_operation variable_definition variable_definition_ literal

%type<silicon::ast::Interface *> interface_definition

%type<silicon::ast::Function *> function_definition

%type<silicon::ast::Prototype *> function_declaration variadic_function_declaration

%type<silicon::ast::If *> if_statement_

%type<silicon::ast::Loop *> loop_statement

%type<silicon::ast::While *> while_statement do_while_statement

%type<silicon::ast::For *> for_statement
%type<silicon::ast::Type *> type

%type<std::vector<std::pair<std::string, silicon::ast::Type *>>> arguments_definition arguments_definition_ interface_properties
%type<std::vector<std::pair<std::string, silicon::ast::Type *>>> variadic_arguments_declaration

%type<std::pair<std::string, silicon::ast::Type *>> interface_property

// --------------------------------------------------
// Precedences
// --------------------------------------------------

%expect 0

%precedence RETURN

%left COMMA

%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_STAR_ASSIGN STAR_ASSIGN SLASH_ASSIGN PERCENT_ASSIGN DOUBLE_LESSER_ASSIGN DOUBLE_BIGGER_ASSIGN TRIPLE_BIGGER_ASSIGN AND_ASSIGN CARET_ASSIGN OR_ASSIGN

%right QUESTION_MARK COLON

%left OR_OR

%left AND_AND

%left OR

%left CARET

%left AND

%left EQUAL NOT_EQUAL

%left LESSER LESSER_EQUAL BIGGER BIGGER_EQUAL IN INSTANCE_OF

%left DOUBLE_LESSER DOUBLE_BIGGER TRIPLE_BIGGER

%left PLUS MINUS

%left STAR SLASH PERCENT

%right STAR_STAR

%left AS

%right NOT TILDE TYPE_OF DELETE AWAIT

%precedence PLUS_PLUS MINUS_MINUS

%right NEW

%left DOT OPEN_PAREN OPEN_BRACKET

// --------------------------------------------------
// Grammar
// --------------------------------------------------

%%

library
: statements { ctx.statements($1); }
;

// --------------------------------------------------
// Grammar -> Statements
// --------------------------------------------------

statements
: statement { $$ = { $1 }; }
| statements statement { $1.push_back($2); $$ = $1; }
;

statement
: export_statement { $$ = $1; }
| extern_statement { $$ = $1; }
| interface_definition { $$ = $1; }
| function_definition { $$ = $1; }
;

scoped_statements
: %empty { $$ = { }; }
| scoped_statements scoped_statements_ { $1.insert($1.end(), $2.begin(), $2.end()); $$ = $1; }
;

scoped_statements_
: variable_definitions SEMICOLON { $$ = $1; }
| if_statement { $$ = { $1 }; }
| loop_statement { $$ = { $1 }; }
| while_statement { $$ = { $1 }; }
| do_while_statement { $$ = { $1 }; }
| for_statement { $$ = { $1 }; }
| expression SEMICOLON { $$ = { $1 }; }
;

// --------------------------------------------------
// Grammar -> Export Statements
// --------------------------------------------------

export_statement
: EXPORT function_definition { $$ = $2->externalLinkage(); }
;

// --------------------------------------------------
// Grammar -> Extern Statements
// --------------------------------------------------

extern_statement
: EXTERN function_declaration SEMICOLON { $$ = $2->makeExtern(); }
| EXTERN variadic_function_declaration SEMICOLON { $$ = $2->makeExtern(); }
;

// --------------------------------------------------
// Grammar -> Loop Statements
// --------------------------------------------------

loop_statement
: LOOP expression SEMICOLON { $$ = ctx.def_loop({ $2 }); }
| LOOP OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = ctx.def_loop($3); }
;

// --------------------------------------------------
// Grammar -> While Statements
// --------------------------------------------------

while_statement
: WHILE OPEN_PAREN expression_ CLOSE_PAREN expression SEMICOLON { $$ = ctx.def_while($3, { $5 }); }
| WHILE OPEN_PAREN expression_ CLOSE_PAREN OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = ctx.def_while($3, $6); }
;

do_while_statement
: DO expression SEMICOLON WHILE OPEN_PAREN expression_ CLOSE_PAREN SEMICOLON { $$ = ctx.def_while($6, { $2 })->makeDoWhile(); }
| DO OPEN_CURLY scoped_statements CLOSE_CURLY WHILE OPEN_PAREN expression_ CLOSE_PAREN SEMICOLON { $$ = ctx.def_while($7, $3)->makeDoWhile(); }
;

// --------------------------------------------------
// Grammar -> For Statements
// --------------------------------------------------

for_statement
: FOR OPEN_PAREN variable_definition SEMICOLON expression_ SEMICOLON expression_ CLOSE_PAREN expression SEMICOLON { $$ = ctx.def_for($3, $5, $7, { $9 }); }
| FOR OPEN_PAREN variable_definition SEMICOLON expression_ SEMICOLON expression_ CLOSE_PAREN OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = ctx.def_for($3, $5, $7, $10); }
;

// --------------------------------------------------
// Grammar -> If Statements
// --------------------------------------------------

if_statement
: if_statement_ { $$ = $1; }
| if_statement_ ELSE expression SEMICOLON { $$ = $1->setElse({ $3 }); }
| if_statement_ ELSE OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = $1->setElse($4); }
| if_statement_ ELSE if_statement { $$ = $1->setElse({ $3 }); }
;

if_statement_
: IF OPEN_PAREN expression_ CLOSE_PAREN expression SEMICOLON { $$ = ctx.def_if($3, { $5 }); }
| IF OPEN_PAREN expression_ CLOSE_PAREN OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = ctx.def_if($3, $6); }
;

// --------------------------------------------------
// Grammar -> Expressions
// --------------------------------------------------

expression
: RETURN expression_ { $$ = ctx.def_ret($2); }
| RETURN { $$ = ctx.def_ret(); }
| BREAK { $$ = { ctx.def_break() }; }
| CONTINUE { $$ = ctx.def_continue(); }
| expression_ { $$ = $1; }
;

expression_
: literal { $$ = $1; }
| operation { $$ = $1; }
| IDENTIFIER { $$ = ctx.var($1); }
| IDENTIFIER OPEN_PAREN arguments CLOSE_PAREN { $$ = ctx.call_func($1, $3); }
| OPEN_PAREN expression_ CLOSE_PAREN { $$ = $2; }
| expression_ QUESTION_MARK expression_ COLON expression_ { $$ = ctx.def_if($1, { $3 }, { $5 })->makeInline(); }
| expression_ AS type { $$ = ctx.def_cast($1, $3); }
| expression_ DOT IDENTIFIER { $$ = ctx.var($3, $1); }
;

// --------------------------------------------------
// Grammar -> Operations
// --------------------------------------------------

operation
: binary_operation { $$ = $1; }
| unary_operation { $$ = $1; }
;

binary_operation
: expression_ ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, $3); }
| expression_ STAR expression_ { $$ = ctx.def_op(silicon::binary_operation_t::STAR, $1, $3); }
| expression_ SLASH expression_ { $$ = ctx.def_op(silicon::binary_operation_t::SLASH, $1, $3); }
| expression_ PERCENT expression_ { $$ = ctx.def_op(silicon::binary_operation_t::PERCENT, $1, $3); }
| expression_ PLUS expression_ { $$ = ctx.def_op(silicon::binary_operation_t::PLUS, $1, $3); }
| expression_ MINUS expression_ { $$ = ctx.def_op(silicon::binary_operation_t::MINUS, $1, $3); }
| expression_ CARET expression_ { $$ = ctx.def_op(silicon::binary_operation_t::CARET, $1, $3); }
| expression_ AND expression_ { $$ = ctx.def_op(silicon::binary_operation_t::AND, $1, $3); }
| expression_ OR expression_ { $$ = ctx.def_op(silicon::binary_operation_t::OR, $1, $3); }
//| expression_ STAR_STAR expression_
| expression_ DOUBLE_LESSER expression_ { $$ = ctx.def_op(silicon::binary_operation_t::DOUBLE_LESSER, $1, $3); }
| expression_ DOUBLE_BIGGER expression_ { $$ = ctx.def_op(silicon::binary_operation_t::DOUBLE_BIGGER, $1, $3); }
| expression_ TRIPLE_BIGGER expression_ { $$ = ctx.def_op(silicon::binary_operation_t::TRIPLE_BIGGER, $1, $3); }
| expression_ STAR_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::STAR, $1, $3)); }
| expression_ SLASH_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::SLASH, $1, $3)); }
| expression_ PERCENT_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::PERCENT, $1, $3)); }
| expression_ PLUS_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::PLUS, $1, $3)); }
| expression_ MINUS_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::MINUS, $1, $3)); }
| expression_ CARET_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::CARET, $1, $3)); }
//| expression_ STAR_STAR_ASSIGN expression_
| expression_ AND_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::AND, $1, $3)); }
| expression_ OR_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::OR, $1, $3)); }
| expression_ DOUBLE_LESSER_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::DOUBLE_LESSER, $1, $3)); }
| expression_ DOUBLE_BIGGER_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::DOUBLE_BIGGER, $1, $3)); }
| expression_ TRIPLE_BIGGER_ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::TRIPLE_BIGGER, $1, $3)); }
//| expression_ AND_AND expression_
//| expression_ OR_OR expression_
| expression_ LESSER expression_ { $$ = ctx.def_op(silicon::binary_operation_t::LESSER, $1, $3); }
| expression_ LESSER_EQUAL expression_ { $$ = ctx.def_op(silicon::binary_operation_t::LESSER_EQUAL, $1, $3); }
| expression_ EQUAL expression_ { $$ = ctx.def_op(silicon::binary_operation_t::EQUAL, $1, $3); }
| expression_ NOT_EQUAL expression_ { $$ = ctx.def_op(silicon::binary_operation_t::NOT_EQUAL, $1, $3); }
| expression_ BIGGER_EQUAL expression_ { $$ = ctx.def_op(silicon::binary_operation_t::BIGGER_EQUAL, $1, $3); }
| expression_ BIGGER expression_ { $$ = ctx.def_op(silicon::binary_operation_t::BIGGER, $1, $3); }
;

unary_operation
: expression_ PLUS_PLUS { $$ = ctx.def_op(silicon::unary_operation_t::PLUS_PLUS, $1, true); }
| expression_ MINUS_MINUS { $$ = ctx.def_op(silicon::unary_operation_t::MINUS_MINUS, $1, true); }
| PLUS_PLUS expression_ %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::PLUS_PLUS, $2); }
| MINUS_MINUS expression_ %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::MINUS_MINUS, $2); }
//| AND expression_
//| PLUS expression_ %prec NOT
| MINUS expression_ %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::MINUS, $2); }
| NOT expression_ { $$ = ctx.def_op(silicon::unary_operation_t::NOT, $2); }
//| TILDE expression_
;

// --------------------------------------------------
// Grammar -> Interfaces
// --------------------------------------------------

interface_definition
: INTERFACE IDENTIFIER OPEN_CURLY interface_properties CLOSE_CURLY { $$ = ctx.def_interface($2, $4); }
;

interface_properties
: interface_property { $$ = { $1 }; }
| interface_properties interface_property { $1.push_back($2); $$ = $1; }
;

interface_property
: IDENTIFIER COLON type SEMICOLON { $$ = {$1, $3}; }
;

// --------------------------------------------------
// Grammar -> Variables
// --------------------------------------------------

variable_definitions
: variable_definition { $$ = { $1 }; }
| variable_definitions COMMA variable_definition_ { $1.push_back($3); $$ = $1; }
;

variable_definition
: LET variable_definition_ { $$ = $2; }
;

variable_definition_
: IDENTIFIER ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, ctx.def_var($1), $3); }
| IDENTIFIER COLON type ASSIGN expression_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, ctx.def_var($1, $3), $5); }
//| IDENTIFIER QUESTION_MARK COLON type
//| IDENTIFIER QUESTION_MARK COLON type ASSIGN expression_
;

// --------------------------------------------------
// Grammar -> Functions
// --------------------------------------------------

function_definition
: function_declaration OPEN_CURLY scoped_statements CLOSE_CURLY { $$ = ctx.def_func($1, $3); }
;

function_declaration
: FUNCTION IDENTIFIER OPEN_PAREN arguments_definition CLOSE_PAREN COLON type { $$ = ctx.def_proto($2, $4, $7); }
| FUNCTION IDENTIFIER OPEN_PAREN arguments_definition CLOSE_PAREN { $$ = ctx.def_proto($2, $4); }
;

variadic_function_declaration
: FUNCTION IDENTIFIER OPEN_PAREN variadic_arguments_declaration CLOSE_PAREN COLON type { $$ = ctx.def_proto($2, $4, $7)->makeVariadic(); }
| FUNCTION IDENTIFIER OPEN_PAREN variadic_arguments_declaration CLOSE_PAREN { $$ = ctx.def_proto($2, $4)->makeVariadic(); }
;

// --------------------------------------------------
// Grammar -> Arguments
// --------------------------------------------------

arguments_definition
: %empty { $$ = { }; }
| arguments_definition_ { $$ = $1; }
;

arguments_definition_
: IDENTIFIER COLON type { $$ = { ctx.def_arg($1, $3) }; }
| arguments_definition_ COMMA IDENTIFIER COLON type { $1.push_back(ctx.def_arg($3, $5)); $$ = $1; }
;

variadic_arguments_declaration
: TRIPLE_DOT { $$ = { }; }
| arguments_definition_ COMMA TRIPLE_DOT { $$ = $1; }
;

arguments
: %empty { $$ = { }; }
| arguments_ { $$ = $1; }
;

arguments_
: expression_ { $$ = { $1 }; }
| arguments_ COMMA expression_ { $1.push_back($3); $$ = $1; }
;

// --------------------------------------------------
// Grammar -> Literals
// --------------------------------------------------

literal
: NULL { $$ = ctx.null(); }
| BOOLEAN_LITERAL { $$ = ctx.bool_lit($1); }
| NUMBER_LITERAL { $$ = ctx.num_lit($1); }
| STRING_LITERAL { $$ = ctx.string_lit($1); }
;

// --------------------------------------------------
// Grammar -> Types
// --------------------------------------------------

type
: IDENTIFIER { $$ = ctx.type($1); }
;

%%

// --------------------------------------------------
// Lexer
// --------------------------------------------------

yy::Parser::symbol_type yy::yylex(silicon::compiler::Context &ctx)
{
    const char* anchor = ctx.cursor;

    ctx.loc.step();

    auto s = [&](auto func, auto&&... params)
    {
      ctx.loc.columns(ctx.cursor - anchor);

      return func(params..., ctx.loc);
    };

%{
re2c:yyfill:enable = 0;
re2c:define:YYCTYPE = "char";
re2c:define:YYCURSOR = "ctx.cursor";
re2c:define:YYMARKER = "ctx.cursor";

// Tokens

";" { return s(Parser::make_SEMICOLON); }
":" { return s(Parser::make_COLON); }
"(" { return s(Parser::make_OPEN_PAREN); }
")" { return s(Parser::make_CLOSE_PAREN); }
"{" { return s(Parser::make_OPEN_CURLY); }
"}" { return s(Parser::make_CLOSE_CURLY); }

// Tokens -> Literals

"null" { return s(Parser::make_NULL); }
"false" { return s(Parser::make_BOOLEAN_LITERAL, false); }
"true" { return s(Parser::make_BOOLEAN_LITERAL, true); }
[0-9]* [\.] [0-9]+ ("_" [0-9]+)* { return s(Parser::make_NUMBER_LITERAL, std::string(anchor, ctx.cursor)); }
[0-9]+ ("_" [0-9]+)* [\.] [0-9]+ ("_" [0-9]+)* { return s(Parser::make_NUMBER_LITERAL, std::string(anchor, ctx.cursor)); }
[0-9]+ ("_" [0-9]+)* { return s(Parser::make_NUMBER_LITERAL, std::string(anchor, ctx.cursor)); }
"\"" ("\\".|[^"\\])* "\"" { return s(Parser::make_STRING_LITERAL, std::string(anchor + 1, ctx.cursor - 1)); }

// Tokens -> Keywords

"let" { return s(Parser::make_LET); }
"fn" { return s(Parser::make_FUNCTION); }
"return" { return s(Parser::make_RETURN); }
"if" { return s(Parser::make_IF); }
"interface" { return s(Parser::make_INTERFACE); }
"else" { return s(Parser::make_ELSE); }
"loop" { return s(Parser::make_LOOP); }
"while" { return s(Parser::make_WHILE); }
"do" { return s(Parser::make_DO); }
"for" { return s(Parser::make_FOR); }
"break" { return s(Parser::make_BREAK); }
"continue" { return s(Parser::make_CONTINUE); }
"export" { return s(Parser::make_EXPORT); }
"extern" { return s(Parser::make_EXTERN); }

// Tokens -> Operators

"," { return s(Parser::make_COMMA); }
"=" { return s(Parser::make_ASSIGN); }
"*" { return s(Parser::make_STAR); }
"/" { return s(Parser::make_SLASH); }
"%" { return s(Parser::make_PERCENT); }
"+" { return s(Parser::make_PLUS); }
"-" { return s(Parser::make_MINUS); }
"^" { return s(Parser::make_CARET); }
"&" { return s(Parser::make_AND); }
"|" { return s(Parser::make_OR); }
"**" { return s(Parser::make_STAR_STAR); }
"<<" { return s(Parser::make_DOUBLE_LESSER); }
">>" { return s(Parser::make_DOUBLE_BIGGER); }
">>>" { return s(Parser::make_TRIPLE_BIGGER); }
"*=" { return s(Parser::make_STAR_ASSIGN); }
"/=" { return s(Parser::make_SLASH_ASSIGN); }
"%=" { return s(Parser::make_PERCENT_ASSIGN); }
"+=" { return s(Parser::make_PLUS_ASSIGN); }
"-=" { return s(Parser::make_MINUS_ASSIGN); }
"^=" { return s(Parser::make_CARET_ASSIGN); }
"&=" { return s(Parser::make_AND_ASSIGN); }
"|=" { return s(Parser::make_OR_ASSIGN); }
"**=" { return s(Parser::make_STAR_STAR_ASSIGN); }
"<<=" { return s(Parser::make_DOUBLE_LESSER_ASSIGN); }
">>=" { return s(Parser::make_DOUBLE_BIGGER_ASSIGN); }
">>>=" { return s(Parser::make_TRIPLE_BIGGER_ASSIGN); }
"++" { return s(Parser::make_PLUS_PLUS); }
"--" { return s(Parser::make_MINUS_MINUS); }
"&&" { return s(Parser::make_AND_AND); }
"||" { return s(Parser::make_OR_OR); }
"!" { return s(Parser::make_NOT); }
"<" { return s(Parser::make_LESSER); }
"<=" { return s(Parser::make_LESSER_EQUAL); }
"==" { return s(Parser::make_EQUAL); }
"!=" { return s(Parser::make_NOT_EQUAL); }
">=" { return s(Parser::make_BIGGER_EQUAL); }
">" { return s(Parser::make_BIGGER); }
"~" { return s(Parser::make_TILDE); }
"?" { return s(Parser::make_QUESTION_MARK); }
"." { return s(Parser::make_DOT); }
"delete" { return s(Parser::make_DELETE); }
"as" { return s(Parser::make_AS); }
"instanceof" { return s(Parser::make_INSTANCE_OF); }
"await" { return s(Parser::make_AWAIT); }
"typeof" { return s(Parser::make_TYPE_OF); }
"new" { return s(Parser::make_NEW); }
"..." { return s(Parser::make_TRIPLE_DOT); }

// Tokens -> Whitespace and Comments

"\000" { return s(Parser::make_EOF); }

"\r\n" | [\r\n] { ctx.loc.lines(); return yylex(ctx); }
"//" [^\r\n]* { return yylex(ctx); }
"/*" [.\r\n]* "*/" { return yylex(ctx); }
[\t\v\b\f ] { ctx.loc.columns(); return yylex(ctx); }

// Tokens -> Identifiers

[a-zA-Z_] [a-zA-Z_0-9]* { return s(Parser::make_IDENTIFIER, std::string(anchor, ctx.cursor)); }

%}
/* End lexer */

}
