/*
 * MeSQL - Interpreter - parser.y
 *
 * This file define the parser.
 *
 */


%skeleton "lalr1.cc" /* -*- C++ -*- */
%defines
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { MeInt }
%code requires
{
    #include <vector>
    #include <string>
    #include <sstream>
    #include <iostream>
    #include "command.hpp"
    #include "base.hpp"

    using namespace std;

    namespace MeInt {
        class Scanner;
        class Interpreter;
    }
}

%code provides
{
    const char greet_str[] = "MeSQL shell\n\n";
}

%code top
{
    #include "scanner.h"
    #include "parser.hpp"
    #include "interpreter.h"
    #include "location.hh"
    
    static MeInt::Parser::symbol_type yylex(MeInt::Scanner &scanner) {
        return scanner.get_next_token();
    }
    
    using namespace MeInt;
    using namespace MeType;
    using namespace MeInfo;
}

%lex-param { MeInt::Scanner &scanner }
%parse-param { MeInt::Scanner &scanner }
%parse-param { MeInt::Interpreter &driver }
%locations
%define parse.trace
%define parse.error verbose 

%define api.token.prefix {TOKEN_}

%token SEMICOLON ";"
%token CREATE "create"
%token TABLE "table"
%token UNIQUE "unique"
%token PRIMARY "primary"
%token KEY "key"
%token INT "int"
%token FLOAT "float"
%token CHAR "char"
%token <std::string> NUMBER
%token <std::string> IDENTIFIER "identifier"
%token LPAREN "("
%token RPAREN ")"
%token COMMA ","


%type <Statement*> statement;
%type <CreateTableStatement*> create_table_statement;
%type <DropTableStatement*> drop_table_statement;
%type <CreateIndexStatement*> create_index_statement;
%type <DropIndexStatement*> drop_index_statement;
%type <SelectStatement*> select_statement;
%type <InsertStatement*> insert_statement;
%type <DeleteStatement*> delete_statement;
%type <ExecfileStatement*> execfile_statement;
%type <std::vector<TableColumnDef> > create_table_col_defs;
%type <TableColumnDef> create_table_col_def;
%type <TableColumnSpec> create_table_col_spec;
%type <TableColumnSpec> create_table_col_type;

%start program

%%

program : 
        {
            *driver.os << greet_str;
            driver.prompt();
            driver.clear();
        }
    | program statement ";"
        {
            Statement *stat = $2;
            stat->execute();
            string res = stat->result_str(); // they are virtual function
            *driver.os << res << endl << endl;
            driver.prompt();
        }
    ;

statement:
    create_table_statement { $$ = dynamic_cast<Statement*>($1); }
    | drop_table_statement { $$ = dynamic_cast<Statement*>($1); }
    | create_index_statement { $$ = dynamic_cast<Statement*>($1); }
    | drop_index_statement { $$ = dynamic_cast<Statement*>($1); }
    | select_statement { $$ = dynamic_cast<Statement*>($1); }
    | insert_statement { $$ = dynamic_cast<Statement*>($1); }
    | delete_statement { $$ = dynamic_cast<Statement*>($1); }
    | execfile_statement { $$ = dynamic_cast<Statement*>($1); }
    ;

create_table_statement:
    "create" "table" IDENTIFIER "(" create_table_col_defs "," create_table_pk_def ")"
        {
            const string &table_name = $3; // IDENTIFIER
            const vector<TableColumnDef> &cols = $5; // create_table_col_defs
            const string &primary_key = $7; // create_table_pk_def
            $$ = new CreateTableStatement(table_name,cols,primary_key);
            $$->give_order();
        }

create_table_col_defs:
    create_table_col_def
        {
            $$ = vector<TableColumnDef>{$1};
        }
    | create_table_col_defs "," create_table_col_def
        {
            $$.push_back($3);
        }
    ;

create_table_col_def:
    IDENTIFIER create_table_col_spec
        {
            $$ = TableColumnDef(0,$1,$2);
        }
    ;

create_table_col_spec:
    create_table_col_type
        {
            $$ = $1;
            $$.is_unique = false;
        }
    | create_table_col_type "unique"
        {
            $$ = $1;
            $$.is_unique = true;
        }
    ;

create_table_col_type:
    "int" { $$ = TableColumnSpec(DataType::INT,0,false,false); }
    | "float" { $$ = TableColumnSpec(DataType::FLOAT,0,false,false); }
    | "char" "(" NUMBER ")" 
        {
            string num = $3;
            bool err = false;
            int tmp = 0;
            if (num.length() > 3) err = true; else {
                stringstream ss;
                ss << num;
                ss >> tmp;
                if (!(1<=tmp && tmp<=255)) err = true;
            }
            if (err) {
                stringstream ss;
                ss << "invalid char() length " << num;
                error(driver.location(),ss.str());
            } else {
                $$ = TableColumnSpec(DataType::CHAR,static_cast<char_size_t>(tmp),false,false);
            }
        }
    ;

%%

// Bison expects us to provide implementation - otherwise linker complains
void MeInt::Parser::error(const location &loc , const std::string &message) {
        
        // Location should be initialized inside scanner action, but is not in this example.
        // Let's grab location directly from driver class.
	// cout << "Error: " << message << endl << "Location: " << loc << endl;
	
        cout << "Error: " << message << endl << "Error location: " << driver.location() << endl;
}
