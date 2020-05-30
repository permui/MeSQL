/*
 * MeSQL - Interpreter - scanner.hpp
 * 
 * This file declares the scanner, dirived from
 * yyFlexLexer that flex provided
 * 
 */

#ifndef SCANNER_HPP
#define SCANNER_HPP

#ifndef yyFlexLexerOnce
#undef yyFlexLexer
#define yyFlexLexer MeInt_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL MeInt::Parser::symbol_type MeInt::Scanner::get_next_token()

#include "parser.hpp"

namespace MeInt {

	class Interpreter;

	class Scanner : public yyFlexLexer {
	private:
		Interpreter &m_driver;
	public:
		Scanner(Interpreter &_m_driver) : m_driver(_m_driver) {}
		~Scanner() {}
		MeInt::Parser::symbol_type get_next_token();
	};

}

#endif