/*
 * MeSQL - Interpreter - scanner.l
 *
 * This file define the lexer.
 *
 */


%{
    #include <string>
	#include <cstdlib>
	#include "scanner.hpp"
	#include "interpreter.hpp"
	#include "parser.hpp"
	#include "location.hh"

	using namespace std;

	#define yyterminate() MeInt::Parser::make_END(MeInt::location());

	#define YY_USER_ACTION m_driver.increaseLocation(yyleng);
	
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="MeInt_"

%%

;			{ return MeInt::Parser::make_SEMICOLON(MeInt::location()); }

*			{ return MeInt::Parser::make_STAR(MeInt::location()); }

create		{ return MeInt::Parser::make_CREATE(MeInt::location()); }

table		{ return MeInt::Parser::make_TABLE(MeInt::location()); }

unique 		{ return MeInt::Parser::make_UNIQUE(MeInt::location()); }

primary		{ return MeInt::Parser::make_PRIMARY(MeInt::location()); }

key 		{ return MeInt::Parser::make_KEY(MeInt::location()); }

drop		{ return MeInt::Parser::make_DROP(MeInt::location()); }

index		{ return MeInt::Parser::make_INDEX(MeInt::location()); }

on			{ return MeInt::Parser::make_ON(MeInt::location()); }

and			{ return MeInt::Parser::make_AND(MeInt::location()); }

int 		{ return MeInt::Parser::make_INT(MeInt::location()); }

float 		{ return MeInt::Parser::make_FLOAT(MeInt::location()); }

char		{ return MeInt::Parser::make_CHAR(MeInt::location()); }

quit		{ return MeInt::Parser::make_QUIT(MeInt::location()); }

"insert"	{ return MeInt::Parser::make_INSERT(MeInt::location()); }

"into"		{ return MeInt::Parser::make_INTO(MeInt::location()); }

"values"	{ return MeInt::Parser::make_VALUES(MeInt::location()); }

		 
\d+		{ return MeInt::Parser::make_NUMBER(yytext,MeInt::location()); }

[-+]?((\d+\.\d*)|(\.\d+))	{ return MeInt::Parser::make_FRACTION(yytext,MeInt::location()); }

'.*'		{ return MeInt::Parser::make_STRING_LIT(yytext,MeInt::location()); }

[a-zA-Z_]\w*	{ return MeInt::Parser::make_IDENTIFIER(yytext,MeInt::location()); }

\(			{ return MeInt::Parser::make_LPAREN(MeInt::location()); }

\)			{ return MeInt::Parser::make_RPAREN(MeInt::location()); }

"="			{ return MeInt::Parser::make_EQ(MeInt::location()); }
"<>"		{ return MeInt::Parser::make_NE(MeInt::location()); }
"<"			{ return MeInt::Parser::make_LESS(MeInt::location()); }
">"			{ return MeInt::Parser::make_GREATER(MeInt::location()); }
"<="		{ return MeInt::Parser::make_LE(MeInt::location()); }
">="		{ return MeInt::Parser::make_GE(MeInt::location()); }

,			{ return MeInt::Parser::make_COMMA(MeInt::location()); }

[\n\t ]		{ }

.			{ 
				return MeInt::Parser::make_INVALID_CHAR(*yytext,MeInt::location()); 
			}
            
<<EOF>>     { return yyterminate(); }


%%
