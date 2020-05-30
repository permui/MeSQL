/*
 * MeSQL - Interpreter - scanner.hpp
 * 
 * This file declares the scanner, dirived from
 * yyFlexLexer that flex provided
 * 
 */

#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <FlexLexer.h>
#include "parser.hpp"
#include "interpreter.hpp"

#undef YY_DECL
#define YY_DECL MeInt::Parser::symbol_type MeInt::Scanner::get_next_token()

namespace MeInt {

	class Scanner : public yyFlexLexer {
	private:
		Interpreter &m_driver;
	public:
		Scanner(Interpreter &_m_driver) : m_driver(_m_driver) {}
		~Scanner() {}
		MeInt::Parser::symbol_type MeInt::Scanner::get_next_token();
	};

}

#endif