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

DIGIT [0-9]
ICHAR [a-zA-Z0-9_]

%%

";"			{ return MeInt::Parser::make_SEMICOLON(MeInt::location()); }

"*"			{ return MeInt::Parser::make_STAR(MeInt::location()); }

"create"	{ return MeInt::Parser::make_CREATE(MeInt::location()); }

"table"		{ return MeInt::Parser::make_TABLE(MeInt::location()); }

"unique"	{ return MeInt::Parser::make_UNIQUE(MeInt::location()); }

"primary"	{ return MeInt::Parser::make_PRIMARY(MeInt::location()); }

"key" 		{ return MeInt::Parser::make_KEY(MeInt::location()); }

"drop"		{ return MeInt::Parser::make_DROP(MeInt::location()); }

"index"		{ return MeInt::Parser::make_INDEX(MeInt::location()); }

"select"		{ return MeInt::Parser::make_SELECT(MeInt::location()); }

"from"		{ return MeInt::Parser::make_FROM(MeInt::location()); }

"where"		{ return MeInt::Parser::make_WHERE(MeInt::location()); }

"on"		{ return MeInt::Parser::make_ON(MeInt::location()); }

"and"		{ return MeInt::Parser::make_AND(MeInt::location()); }

"int" 		{ return MeInt::Parser::make_INT(MeInt::location()); }

"float"		{ return MeInt::Parser::make_FLOAT(MeInt::location()); }

"char"		{ return MeInt::Parser::make_CHAR(MeInt::location()); }

"quit"		{ return MeInt::Parser::make_QUIT(MeInt::location()); }

"insert"	{ return MeInt::Parser::make_INSERT(MeInt::location()); }

"into"		{ return MeInt::Parser::make_INTO(MeInt::location()); }

"values"	{ return MeInt::Parser::make_VALUES(MeInt::location()); }

"delete"	{ return MeInt::Parser::make_DELETE(MeInt::location()); }

"execfile"	{ return MeInt::Parser::make_EXECFILE(MeInt::location()); }

"show"		{ return MeInt::Parser::make_SHOW(MeInt::location()); }

"tables"	{ return MeInt::Parser::make_TABLES(MeInt::location()); }

"indexes"	{ return MeInt::Parser::make_INDEXES(MeInt::location()); }

"if"		{ return MeInt::Parser::make_IF(MeInt::location()); }

"exists"	{ return MeInt::Parser::make_EXISTS(MeInt::location()); }

"limit"		{ return MeInt::Parser::make_LIMIT(MeInt::location()); }

\"[^ \t\n"]+\"	{ return MeInt::Parser::make_FILENAME(yytext,MeInt::location()); }
		 
[-+]?{DIGIT}+	{ return MeInt::Parser::make_NUMBER(yytext,MeInt::location()); }

[-+]?(({DIGIT}+\.{DIGIT}*)|(\.{DIGIT}+))	{ return MeInt::Parser::make_FRACTION(yytext,MeInt::location()); }

'[^']*'		{ return MeInt::Parser::make_STRING_LIT(yytext,MeInt::location()); }

[a-zA-Z_]{ICHAR}*	{ return MeInt::Parser::make_IDENTIFIER(yytext,MeInt::location()); }

\(			{ return MeInt::Parser::make_LPAREN(MeInt::location()); }

\)			{ return MeInt::Parser::make_RPAREN(MeInt::location()); }

"="			{ return MeInt::Parser::make_EQ(MeInt::location()); }
"<>"		{ return MeInt::Parser::make_NE(MeInt::location()); }
"<"			{ return MeInt::Parser::make_LESS(MeInt::location()); }
">"			{ return MeInt::Parser::make_GREATER(MeInt::location()); }
"<="		{ return MeInt::Parser::make_LE(MeInt::location()); }
">="		{ return MeInt::Parser::make_GE(MeInt::location()); }

","			{ return MeInt::Parser::make_COMMA(MeInt::location()); }

[\n\r\t ]		{ }

.			{ 
				return MeInt::Parser::make_INVALID_CHAR(*yytext,MeInt::location()); 
			}
            
<<EOF>>     { return yyterminate(); }


%%
