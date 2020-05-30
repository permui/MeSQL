/*
 * MeSQL - Interpreter - scanner.l
 *
 * This file define the lexer.
 *
 */


%{
    #include <string>
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

%%

;			{ return MeInt::Parser::make_SEMICOLON(MeInt::location(m_driver.location())); }

creat		{ return MeInt::Parser::make_CREATE(MeInt::location(m_driver.location())); }

table		{ return MeInt::Parser::make_TABLE(MeInt::location(m_driver.location())); }

unique 		{ return MeInt::Parser::make_UNIQUE(MeInt::location(m_driver.location())); }

primary		{ return MeInt::Parser::make_PRIMARY(MeInt::location(m_driver.location())); }

key 		{ return MeInt::Parser::make_KEY(MeInt::location(m_driver.location())); }

int 		{ return MeInt::Parser::make_INT(MeInt::location(m_driver.location())); }

float 		{ return MeInt::Parser::make_FLOAT(MeInt::location(m_driver.location())); }

char		{ return MeInt::Parser::make_CHAR(MeInt::location(m_driver.location())); }

quit		{ return MeInt::Parser::make_QUIT(MeInt::location(m_driver.location())); }
		 
[0-9]+		{ return MeInt::Parser::make_NUMBER(yytext,MeInt::location(m_driver.location())); }

[a-z_][a-z0-9_]*	{ return MeInt::Parser::make_IDENTIFIER(yytext,MeInt::location(m_driver.location())); }

\(			{ return MeInt::Parser::make_LPAREN(MeInt::location(m_driver.location())); }

\)			{ return MeInt::Parser::make_RPAREN(MeInt::location(m_driver.location())); }

,			{ return MeInt::Parser::make_COMMA(MeInt::location(m_driver.location())); }

[\n\t ]		{ }

.			{ 
				stringstream ss;
				ss << "invalid character '" << *yytext << "'";
				MeInt::Parser::error(m_driver.location(),ss.str());
			}
            
<<EOF>>     { return yyterminate(); }


%%
