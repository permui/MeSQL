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
	#include "../command/command.hpp"
	#include "../base/error.hpp"
	#include "../base/base.hpp"
	#include "../catalog/catalog.hpp"

	namespace MeInt {
		class Scanner;
		class Interpreter;
	}
    using namespace std;
}

%code provides
{
    const char greet_str[] = "MeSQL shell\n\n";
	#define ERROR_RETURN return 1
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
	using namespace MeError;
	using namespace MeCat;
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
%token INSERT "insert"
%token INTO "into"
%token VALUES "values"
%token DELETE "delete"
%token EXECFILE "execfile"
%token SHOW "show"
%token TABLES "tables"
%token INDEXES "indexes"
%token IF "if"
%token EXISTS "exists"
%token LIMIT "limit"
%token <std::string> NUMBER "number"
%token <std::string> FRACTION "fraction"
%token <std::string> STRING_LIT "string literal"
%token <std::string> IDENTIFIER "identifier"
%token <std::string> FILENAME "file name"
%token EQ "="
%token NE "<>"
%token LESS "<"
%token GREATER ">"
%token LE "<="
%token GE ">="
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
%type <SelectStatement*> select_statement_sub;
%type <std::string> select_proj_col;
%type <std::vector<string> > select_proj_cols;
%type <WhereCond> where_condition;
%type <WhereCondItem> where_condition_item;
%type <Literal> literal;
%type <CompareOp> compare_op;

%type <InsertStatement*> insert_statement;
%type <InsertTuples> insert_tuples;
%type <InsertTuple> insert_tuple;
%type <InsertTuple> insert_tuple_content;

%type <DeleteStatement*> delete_statement;
%type <ExecfileStatement*> execfile_statement;

%type <ShowTablesStatement*> show_tables_statement;
%type <ShowIndexesStatement*> show_indexes_statement;

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
			stat->set_manager(&driver.m_man);
            stat->execute();
			delete stat;
			*driver.os << endl;
        }
	| program "quit" ";"
		{
			driver.set_state(-1);
			return -1;
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
    | select_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | insert_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | delete_statement { $$ = dynamic_cast<Statement*>($1); } // written
    | execfile_statement { $$ = dynamic_cast<Statement*>($1); } // written
	| show_tables_statement { $$ = dynamic_cast<Statement*>($1); } // written
	| show_indexes_statement { $$ = dynamic_cast<Statement*>($1); } // written
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
    "int" { $$ = TableColumnSpec(DataType::INT,4,false,false); }
    | "float" { $$ = TableColumnSpec(DataType::FLOAT,4,false,false); }
    | "char" "(" NUMBER ")" 
        {
            string num = $3;
            bool err = false;
            int tmp = 0;
			stringstream ss($3);
			ss >> tmp;
			if (ss.fail() || !(1<=tmp && tmp<=255)) err = true;
            if (err) {
				ss.str("");
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
	"drop" "table" IDENTIFIER "if" "exists"
		{
			$$ = new DropTableStatement($3,true);
		}
	| "drop" "table" IDENTIFIER
		{
			$$ = new DropTableStatement($3,false);
		}
	;

create_index_statement:
	"create" "index" IDENTIFIER "on" IDENTIFIER "(" IDENTIFIER ")"
		{
			$$ = new CreateIndexStatement($3,$5,$7);
		}
	;

drop_index_statement:
	"drop" "index" IDENTIFIER "if" "exists"
		{
			$$ = new DropIndexStatement($3,true);
		}
	| "drop" "index" IDENTIFIER
		{
			$$ = new DropIndexStatement($3,false);
		}
	;

select_statement:
	select_statement_sub "limit" literal
		{
			$$ = $1;
			$$->set_lim($3);
		}
	| select_statement_sub
		{
			$$ = $1;
		}
	;

select_statement_sub:
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
			$$.items.swap($1.items);
			$$.items.push_back($3);
		}
	;

where_condition_item:
	IDENTIFIER compare_op literal
		{
			$$ = WhereCondItem($1,$2,$3);
		}
	;

literal:
	NUMBER
		{
			stringstream ss1;
			ss1.str($1);
			int x;
			ss1 >> x;
			if (ss1.fail()) {
				MyError("cannot convert '" + $1 + "' to INT");
				ERROR_RETURN;
			} else $$ = Literal(x);
		}
	| FRACTION
		{
			stringstream ss2;
			ss2.str($1);
			float x;
			ss2 >> x;
			if (ss2.fail()) {
				MyError("cannot convert '" + $1 + "' to FLOAT");
				ERROR_RETURN;
			} else $$ = Literal(x);
		}
	| STRING_LIT
		{
			const string &str = $1;
			size_t len = str.length();
			if (len<2 || str.front()!='\'' || str.back()!='\'') {
				MyError("string literal scan internal error: got " + str);
				ERROR_RETURN;
			} else {
				string tmp = str.substr(1,len-2);
				$$ = Literal(tmp);
			}
		}
	;

compare_op:
	EQ { $$ = CompareOp::EQ; }
	| NE { $$ = CompareOp::NE; }
	| LESS { $$ = CompareOp::L; }
	| GREATER { $$ = CompareOp::G; }
	| LE { $$ = CompareOp::LE; }
	| GE { $$ = CompareOp::GE; }
	;

insert_statement:
	"insert" "into" IDENTIFIER "values" insert_tuples
		{
			$$ = new InsertStatement($3,$5);
		}
	;

insert_tuples:
	insert_tuple
		{
			$$ = InsertTuples{$1};
		}
	| insert_tuples "," insert_tuple
		{
			$$.swap($1);
			$$.push_back($3);
		}
	;

insert_tuple:
	"(" insert_tuple_content ")"
		{
			$$.swap($2);
		}
	;

insert_tuple_content:
	literal { $$ = InsertTuple{$1}; }
	| insert_tuple_content "," literal
		{
			$$.swap($1);
			$$.push_back($3);
		}
	;

delete_statement:
	"delete" "from" IDENTIFIER
		{
			$$ = new DeleteStatement($3,WhereCond());
		}
	| "delete" "from" IDENTIFIER "where" where_condition
		{
			$$ = new DeleteStatement($3,$5);
		}
	;

execfile_statement:
	"execfile" FILENAME
		{
			const string &str = $2;
			size_t len = str.length();
			if (len<2 || str.front()!='"' || str.back()!='"') {
				MyError("invalid file name " + str);
				ERROR_RETURN;
			} else {
				string tmp = str.substr(1,len-2);
				$$ = new ExecfileStatement(tmp);
			}
		}
	;

show_tables_statement:
	"show" "tables" { $$ = new ShowTablesStatement; }
	;

show_indexes_statement:
	"show" "indexes" { $$ = new ShowIndexesStatement; }
	;

%%

void MeInt::Parser::error(const location &loc , const std::string &message) {
	cout << "Error: " << message << endl << "Error location: " << driver.location() << endl;
}
