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
%token EXTENDS "extends"
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

%type<std::vector<silicon::ast::Node *>>
 global_statements statements statement scope arguments arguments_ variable_definitions

%type<silicon::ast::Node *>
 global_statement export_statement extern_statement if_statement expression value value_ operation binary_operation
 unary_operation variable_definition variable_definition_ literal plain_object function_call inline_if

%type<silicon::ast::Interface *> interface_definition

%type<silicon::ast::Function *> function_definition

%type<silicon::ast::Prototype *> function_declaration variadic_function_declaration

%type<silicon::ast::If *> if_statement_

%type<silicon::ast::Loop *> loop_statement

%type<silicon::ast::While *> while_statement do_while_statement

%type<silicon::ast::For *> for_statement

%type<silicon::ast::Type *> type

%type<std::vector<std::pair<std::string, silicon::ast::Type *>>>
 arguments_definition arguments_definition_ interface_properties variadic_arguments_declaration variadic_arguments_declaration_

%type<std::pair<std::string, silicon::ast::Type *>> interface_property

%type<std::map<std::string, silicon::ast::Node *>> object_properties object_properties_

%type<std::pair<std::string, silicon::ast::Node *>> object_property

%type<std::vector<std::string>> interface_parents

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
: global_statements { ctx.statements($1); }
;

// --------------------------------------------------
// Grammar -> Statements
// --------------------------------------------------

global_statements
: global_statement { $$ = { $1 }; }
| global_statements global_statement { $1.push_back($2); $$ = $1; }
;

global_statement
: export_statement { $$ = $1; }
| extern_statement { $$ = $1; }
| interface_definition { $$ = $1; }
| function_definition { $$ = $1; }
;

statements
: %empty { $$ = { }; }
| statements statement { $1.insert($1.end(), $2.begin(), $2.end()); $$ = $1; }
;

statement
: variable_definitions SEMICOLON { $$ = $1; }
| if_statement { $$ = { $1 }; }
| loop_statement { $$ = { $1 }; }
| while_statement { $$ = { $1 }; }
| do_while_statement { $$ = { $1 }; }
| for_statement { $$ = { $1 }; }
| expression SEMICOLON { $$ = { $1 }; }
;

scope
: expression SEMICOLON { $$ = { $1 }; }
| OPEN_CURLY statements CLOSE_CURLY { $$ = $2; }
;

// --------------------------------------------------
// Grammar -> Expressions
// --------------------------------------------------

expression
: RETURN value_ { $$ = ctx.def_ret($2); }
| RETURN { $$ = ctx.def_ret(); }
| BREAK { $$ = { ctx.def_break() }; }
| CONTINUE { $$ = ctx.def_continue(); }
| operation { $$ = $1; }
| function_call { $$ = $1; }
;

value
: literal { $$ = $1; }
| operation { $$ = $1; }
| function_call { $$ = $1; }
| inline_if { $$ = $1; }
| OPEN_PAREN value CLOSE_PAREN { $$ = $2; }
| value AS type { $$ = ctx.def_cast($1, $3); }
| IDENTIFIER { $$ = ctx.var($1); }
| value DOT IDENTIFIER { $$ = ctx.var($3, $1); }
;

value_
: value %prec RETURN { $$ = $1; }
| plain_object { $$ = $1; }
;

// --------------------------------------------------
// Grammar -> Operations
// --------------------------------------------------

operation
: binary_operation { $$ = $1; }
| unary_operation { $$ = $1; }
;

binary_operation
: value ASSIGN value_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, $3); }
| value STAR value { $$ = ctx.def_op(silicon::binary_operation_t::STAR, $1, $3); }
| value SLASH value { $$ = ctx.def_op(silicon::binary_operation_t::SLASH, $1, $3); }
| value PERCENT value { $$ = ctx.def_op(silicon::binary_operation_t::PERCENT, $1, $3); }
| value PLUS value { $$ = ctx.def_op(silicon::binary_operation_t::PLUS, $1, $3); }
| value MINUS value { $$ = ctx.def_op(silicon::binary_operation_t::MINUS, $1, $3); }
| value CARET value { $$ = ctx.def_op(silicon::binary_operation_t::CARET, $1, $3); }
| value AND value { $$ = ctx.def_op(silicon::binary_operation_t::AND, $1, $3); }
| value OR value { $$ = ctx.def_op(silicon::binary_operation_t::OR, $1, $3); }
//| value STAR_STAR value
| value DOUBLE_LESSER value { $$ = ctx.def_op(silicon::binary_operation_t::DOUBLE_LESSER, $1, $3); }
| value DOUBLE_BIGGER value { $$ = ctx.def_op(silicon::binary_operation_t::DOUBLE_BIGGER, $1, $3); }
| value TRIPLE_BIGGER value { $$ = ctx.def_op(silicon::binary_operation_t::TRIPLE_BIGGER, $1, $3); }
| value STAR_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::STAR, $1, $3)); }
| value SLASH_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::SLASH, $1, $3)); }
| value PERCENT_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::PERCENT, $1, $3)); }
| value PLUS_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::PLUS, $1, $3)); }
| value MINUS_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::MINUS, $1, $3)); }
| value CARET_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::CARET, $1, $3)); }
//| value STAR_STAR_ASSIGN value
| value AND_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::AND, $1, $3)); }
| value OR_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::OR, $1, $3)); }
| value DOUBLE_LESSER_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::DOUBLE_LESSER, $1, $3)); }
| value DOUBLE_BIGGER_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::DOUBLE_BIGGER, $1, $3)); }
| value TRIPLE_BIGGER_ASSIGN value { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, $1, ctx.def_op(silicon::binary_operation_t::TRIPLE_BIGGER, $1, $3)); }
//| value AND_AND value
//| value OR_OR value
| value LESSER value { $$ = ctx.def_op(silicon::binary_operation_t::LESSER, $1, $3); }
| value LESSER_EQUAL value { $$ = ctx.def_op(silicon::binary_operation_t::LESSER_EQUAL, $1, $3); }
| value EQUAL value { $$ = ctx.def_op(silicon::binary_operation_t::EQUAL, $1, $3); }
| value NOT_EQUAL value { $$ = ctx.def_op(silicon::binary_operation_t::NOT_EQUAL, $1, $3); }
| value BIGGER_EQUAL value { $$ = ctx.def_op(silicon::binary_operation_t::BIGGER_EQUAL, $1, $3); }
| value BIGGER value { $$ = ctx.def_op(silicon::binary_operation_t::BIGGER, $1, $3); }
;

unary_operation
: value PLUS_PLUS { $$ = ctx.def_op(silicon::unary_operation_t::PLUS_PLUS, $1, true); }
| value MINUS_MINUS { $$ = ctx.def_op(silicon::unary_operation_t::MINUS_MINUS, $1, true); }
| PLUS_PLUS value %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::PLUS_PLUS, $2); }
| MINUS_MINUS value %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::MINUS_MINUS, $2); }
//| AND value
//| PLUS value %prec NOT
| MINUS value %prec NOT { $$ = ctx.def_op(silicon::unary_operation_t::MINUS, $2); }
| NOT value { $$ = ctx.def_op(silicon::unary_operation_t::NOT, $2); }
//| TILDE value
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
: LOOP scope { $$ = ctx.def_loop($2); }
;

// --------------------------------------------------
// Grammar -> While Statements
// --------------------------------------------------

while_statement
: WHILE OPEN_PAREN value CLOSE_PAREN scope { $$ = ctx.def_while($3, $5); }
;

do_while_statement
: DO scope WHILE OPEN_PAREN value CLOSE_PAREN SEMICOLON { $$ = ctx.def_while($5, $2)->makeDoWhile(); }
;

// --------------------------------------------------
// Grammar -> For Statements
// --------------------------------------------------

for_statement
: FOR OPEN_PAREN variable_definition SEMICOLON value SEMICOLON value CLOSE_PAREN scope { $$ = ctx.def_for($3, $5, $7, $9); }
;

// --------------------------------------------------
// Grammar -> If Statements
// --------------------------------------------------

if_statement
: if_statement_ { $$ = $1; }
| if_statement_ ELSE scope { $$ = $1->setElse($3); }
| if_statement_ ELSE if_statement { $$ = $1->setElse({ $3 }); }
;

if_statement_
: IF OPEN_PAREN value CLOSE_PAREN scope { $$ = ctx.def_if($3, $5); }
;

inline_if
: value QUESTION_MARK value_ COLON value_ { $$ = ctx.def_if($1, { $3 }, { $5 })->makeInline(); }
;

// --------------------------------------------------
// Grammar -> Interfaces
// --------------------------------------------------

interface_definition
: INTERFACE IDENTIFIER OPEN_CURLY interface_properties CLOSE_CURLY { $$ = ctx.def_interface($2, $4); }
| INTERFACE IDENTIFIER EXTENDS interface_parents OPEN_CURLY interface_properties CLOSE_CURLY { $$ = ctx.def_interface($2, $6, $4); }
;

interface_parents
: IDENTIFIER { $$ = { $1 }; }
| interface_parents COMMA IDENTIFIER { $1.push_back($3); $$ = $1; }
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
: IDENTIFIER ASSIGN value_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, ctx.def_var($1), $3); }
| IDENTIFIER COLON type ASSIGN value_ { $$ = ctx.def_op(silicon::binary_operation_t::ASSIGN, ctx.def_var($1, $3), $5); }
//| IDENTIFIER QUESTION_MARK COLON type
//| IDENTIFIER QUESTION_MARK COLON type ASSIGN value_
;

// --------------------------------------------------
// Grammar -> Functions
// --------------------------------------------------

function_definition
: function_declaration OPEN_CURLY statements CLOSE_CURLY { $$ = ctx.def_func($1, $3); }
;

function_declaration
: FUNCTION IDENTIFIER OPEN_PAREN arguments_definition CLOSE_PAREN COLON type { $$ = ctx.def_proto($2, $4, $7); }
| FUNCTION IDENTIFIER OPEN_PAREN arguments_definition CLOSE_PAREN { $$ = ctx.def_proto($2, $4); }
;

variadic_function_declaration
: FUNCTION IDENTIFIER OPEN_PAREN variadic_arguments_declaration CLOSE_PAREN COLON type { $$ = ctx.def_proto($2, $4, $7)->makeVariadic(); }
| FUNCTION IDENTIFIER OPEN_PAREN variadic_arguments_declaration CLOSE_PAREN { $$ = ctx.def_proto($2, $4)->makeVariadic(); }
;

function_call
: IDENTIFIER OPEN_PAREN arguments CLOSE_PAREN { $$ = ctx.call_func($1, $3); }
;

// --------------------------------------------------
// Grammar -> Arguments
// --------------------------------------------------

arguments_definition
: %empty { $$ = { }; }
| arguments_definition_ { $$ = $1; }
| arguments_definition_ COMMA { $$ = $1; }
;

arguments_definition_
: IDENTIFIER COLON type { $$ = { ctx.def_arg($1, $3) }; }
| arguments_definition_ COMMA IDENTIFIER COLON type { $1.push_back(ctx.def_arg($3, $5)); $$ = $1; }
;

variadic_arguments_declaration
: variadic_arguments_declaration_ { $$ = $1; }
| variadic_arguments_declaration_ COMMA { $$ = $1; }
;

variadic_arguments_declaration_
: TRIPLE_DOT { $$ = { }; }
| arguments_definition_ COMMA TRIPLE_DOT { $$ = $1; }
;

arguments
: %empty { $$ = { }; }
| arguments_ { $$ = $1; }
| arguments_ COMMA { $$ = $1; }
;

arguments_
: value_ { $$ = { $1 }; }
| arguments_ COMMA value_ { $1.push_back($3); $$ = $1; }
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
// Grammar -> Literals -> Objects
// --------------------------------------------------

plain_object
: OPEN_CURLY object_properties CLOSE_CURLY { $$ = ctx.plain_object($2); }
;

object_properties
: object_properties_ { $$ = $1; }
| object_properties_ COMMA { $$ = $1; }
;

object_properties_
: object_property { $$ = { $1 }; }
| object_properties_ COMMA object_property { $1[$3.first] = $3.second; $$ = $1; }
;

object_property
: IDENTIFIER COLON value_ { $$ = {$1, $3}; }
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
"extends" { return s(Parser::make_EXTENDS); }
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
