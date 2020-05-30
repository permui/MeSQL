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
	#include <climits>
    #include "command.hpp"
    #include "../base/base.hpp"

	namespace MeInt {
		class Scanner;
		class Interpreter;
	}
    using namespace std;
}

%code provides
{
    const char greet_str[] = "MeSQL shell\n\n";
}

%code top
{
    #include "scanner.hpp"
    #include "parser.hpp"
    #include "interpreter.hpp"
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

%token END
%token <char> INVALID_CHAR "invalid character"
%token SEMICOLON ";"
%token STAR "*"
%token CREATE "create"
%token TABLE "table"
%token UNIQUE "unique"
%token PRIMARY "primary"
%token KEY "key"
%token DROP "drop"
%token INDEX "index"
%token SELECT "select"
%token FROM "from"
%token WHERE "where"
%token ON "on"
%token AND "and"
%token INT "int"
%token FLOAT "float"
%token CHAR "char"
%token QUIT "quit"
%token <std::string> NUMBER
%token <std::string> IDENTIFIER "identifier"
%token LPAREN "("
%token RPAREN ")"
%token COMMA ","


%type <Statement*> statement;

%type <CreateTableStatement*> create_table_statement;
%type <std::vector<TableColumnDef> > create_table_col_defs;
%type <TableColumnDef> create_table_col_def;
%type <TableColumnSpec> create_table_col_spec;
%type <TableColumnSpec> create_table_col_type;
%type <std::string> create_table_pk_def;

%type <DropTableStatement*> drop_table_statement;

%type <CreateIndexStatement*> create_index_statement;

%type <DropIndexStatement*> drop_index_statement;

%type <SelectStatement*> select_statement;
%type <std::string> select_proj_col;
%type <std::vector<string> > select_proj_cols;
%type <WhereCond> where_condition;
%type <WhereCondItem> where_condition_item;
%type <Literal> literal;


%type <InsertStatement*> insert_statement;
%type <DeleteStatement*> delete_statement;
%type <ExecfileStatement*> execfile_statement;

%%

program : 
        {
			/*
            *driver.os << greet_str;
            driver.prompt();
            driver.clear();
			*/
        }
    | program statement ";"
        {
            Statement *stat = $2;
            stat->execute();
            // string res = stat->result_str(); // they are virtual function
            // *driver.os << res << endl << endl;
			delete stat;
            //driver.prompt();
        }
	| program END 
		{
			return 0;
		}
	| program INVALID_CHAR
		{
			cout << "invalid char '" << $2 <<"'" << endl;
			return 0;
		}
    ;

statement:
    create_table_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | drop_table_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | create_index_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | drop_index_statement { $$ = dynamic_cast<Statement*>($1); } // written
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
        }

create_table_col_defs:
    create_table_col_def
        {
            $$ = vector<TableColumnDef>{$1};
        }
    | create_table_col_defs "," create_table_col_def
        {
			$$.swap($1);
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
                error(location(),ss.str());
            } else {
                $$ = TableColumnSpec(DataType::CHAR,static_cast<char_size_t>(tmp),false,false);
            }
        }
    ;

create_table_pk_def:
	"primary" "key" "(" IDENTIFIER ")"
		{
			$$ = $4;
		}
	;

drop_table_statement:
	"drop" "table" IDENTIFIER
		{
			$$ = new DropTableStatement($3);
		}
	;

create_index_statement:
	"create" "index" IDENTIFIER "on" IDENTIFIER "(" IDENTIFIER ")"
		{
			$$ = new CreateIndexStatement($3,$5,$7);
		}
	;

drop_index_statement:
	"drop" "index" IDENTIFIER
		{
			$$ = new DropIndexStatement($3);
		}
	;

select_statement:
	"select" select_proj_cols "from" IDENTIFIER
		{
			$$ = new SelectStatement($4,$2,WhereCond());
		}
	| "select" select_proj_cols "from" IDENTIFIER
	  "where" where_condition
		{
			$$ = new SelectStatement($4,$2,$6);
		}
	;

select_proj_col:
	"*" { $$ = string("*"); }
	| IDENTIFIER { $$ = $1; }
	;

select_proj_cols:
	select_proj_col { $$ = vector<string>{$1}; }
	| select_proj_cols "," select_proj_col
		{
			$$.swap($1);
			$$.push_back($3);
		}
	;

where_condition:
	where_condition_item
		{
			$$.items.push_back($1);
		}
	| where_condition "and" where_condition_item
		{
			$$.swap($1);
			$$.push_back($3);
		}
	;

where_condition_item:
	IDENTIFIER compare_op literal
		{
			$$ = WhereCondItem($1,$2,$3);
		}
	;

literal: // we don't check number size for now
	NUMBER
		{
			stringstream ss($1)"
			int x;
			ss >> x;
			if (ss.fail()) MyError("int number '" + $1 + "' out of range");
			else $$ = x;
		}

%%

// Bison expects us to provide implementation - otherwise linker complains
void MeInt::Parser::error(const location &loc , const std::string &message) {
        
        // Location should be initialized inside scanner action, but is not in this example.
        // Let's grab location directly from driver class.
	// cout << "Error: " << message << endl << "Location: " << loc << endl;
	
        cout << "Error: " << message << endl << "Error location: " << driver.location() << endl;
}
