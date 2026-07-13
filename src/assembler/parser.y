%language "c++"
%require "3.2"
%define api.value.type variant
%define api.token.constructor
%define api.namespace {assembler}
%define api.parser.class {Parser}

%define parse.trace false
%define parse.assert true
%define parse.error verbose

%parse-param {assembler::Context& ctx}
%lex-param   {assembler::Context& ctx}

%code requires
{
#include "assembler/argument.hpp"
#include "assembler/context.hpp"
#include "assembler/expression.hpp"
#include "cpu/isa/opcode.hpp"

// Hack to remove iostream
#define _GLIBCXX_IOSTREAM 1
}

%code
{
#ifdef __GNUC__
namespace std
{

struct Stream
{
};

template <typename T>
const Stream& operator<<(const Stream& os, T&&)
{
    return os;
}

// Replace std::cerr with dummy
constexpr static Stream cerr;

}  // namespace std
#endif

#include "assembler/glue.hpp"
#include "assembler/parser.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/printers.hpp"

#define yylex parseNext

namespace assembler
{

Parser::symbol_type parseNext(assembler::Context& ctx);

void Parser::error(const std::string& s)
{
    assembler::reportError(ctx, s);
}

}  // namespace assembler

}

%start Code

// Simple tokens
%token Newline "newline"
%token Comma "," Colon ":" Dollar "$"
%token Add "+" Sub "-" Mult "*" Div "/" Mod "%"
%token LeftSquareBracket "[" RightSquareBracket "]"
%token LeftParenthesis "(" RightParenthesis ")"

// Directive tokens
%token SectionDirective "SECTION"
%token IncludeDirective "INCLUDE"

// Typed tokens
%token <SectionType>  SectionType   "section type"
%token <std::string_view>        Label         "label"
%token <cpu::isa::Operand::Type> Operand       "operand"
%token <cpu::isa::Opcode::Type>  Instruction   "instruction"
%token <std::string_view>        IntLiteral    "integer"
%token <std::string_view>        StringLiteral "string"

%left Add Sub
%left Mult Div Mod

%type <Expression> Expression
%type <Argument>   Argument

%%

Code: Lines;

Lines:
      %empty
    | Lines Newline
    | Lines Statement
    | Lines Statement Newline
;

Statement:
      Instruction                         { assembler::handleInstruction(ctx, $1, {}); }
    | Instruction Argument                { assembler::handleInstruction(ctx, $1, {$2}); }
    | Instruction Argument Comma Argument { assembler::handleInstruction(ctx, $1, {$2, $4}); }
    | Label Colon                         { assembler::handleLabel(ctx, $1); }
    | Label Colon Colon                   { assembler::handleLabel(ctx, $1); }
    | Directive
;

Argument:
      Operand                                          { $$ = Argument($1); }
    | Label                                            { $$ = Argument($1); }
    | Expression                                       { $$ = Argument($1); }
    | LeftSquareBracket Operand RightSquareBracket     { $$ = Argument($2, true); }
    | LeftSquareBracket Expression RightSquareBracket  { $$ = Argument($2, true); }
    | LeftSquareBracket Operand Add RightSquareBracket { $$ = Argument($2, true, Argument::Action::Increment); }
    | LeftSquareBracket Operand Sub RightSquareBracket { $$ = Argument($2, true, Argument::Action::Decrement); }
    | Operand Add Expression                           { $$ = createSpPlusExpression(ctx, $1, $3); }
    | Operand Sub Expression                           { $$ = createSpPlusExpression(ctx, $1, $3.invert()); }
;

Expression:
      IntLiteral                                  { $$.fromIntLiteral(ctx, $1); }
    | Sub IntLiteral                              { $$.fromIntLiteral(ctx, $2, true); }
    | Expression Add Expression                   { $$.add($1, $3); }
    | Expression Sub Expression                   { $$.sub($1, $3); }
    | Expression Mult Expression                  { $$.mult($1, $3); }
    | Expression Div Expression                   { $$.div($1, $3); }
    | Expression Mod Expression                   { $$.mod($1, $3); }
    | LeftParenthesis Expression RightParenthesis { $$ = std::move($2); }
;

Directive:
    SectionDirective StringLiteral Comma SectionType LeftSquareBracket Expression RightSquareBracket {
        (void)yynerrs_; // WA for set but not used variable
        handleSectionDirective(ctx, $2, $4, $6.value());
    }
    | IncludeDirective StringLiteral {
        handleIncludeDirective(ctx, $2);
    }
%%
