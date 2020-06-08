/* 
 * MeSQL - Command Component - command.hpp
 * 
 * This file declares the important Statement class family.
 * Here we define numerous components of MeSQL statement, 
 * includin base class Statement, derived class for various 
 * kind of statements, column type definition, etc.
 * These classes are used extensively in the interpreter.
 * Derived statement classes also encapsulate the internal
 * implementation of different statement.
 */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include "../base/base.hpp"
#include "../base/manager.hpp"

using namespace std;

namespace MeInt {
    using namespace MeType;
    using namespace MeInfo;
	using namespace MeMan;

    /* Base class Statement */
    class Statement {
    protected:
        string content;
        ostream &printer;
		Manager *man;
    public:
        Statement();
        virtual ~Statement() {}
        virtual string str() const = 0;
        virtual void print() const = 0;
        virtual void _execute() = 0; // _execute statement and output result to stdout
		virtual void execute();
		void set_manager(Manager *_man);
    };

    class CreateTableStatement : public Statement {
    private:
        string table_name;
        vector<TableColumnDef> cols;
        string primary_key;
    public:
        CreateTableStatement(const string &_table_name,const vector<TableColumnDef> &_cols,const string &_primary_key);
        string str() const;
        void print() const;
        void _execute();
        void give_order();
    };

    class DropTableStatement : public Statement {
    private:
        string table_name;
    public:
        DropTableStatement(const string &_table_name);
        string str() const;
        void print() const;
        void execute();
    };

    class CreateIndexStatement : public Statement {
    private:
        string index_name;
        string table_name;
        string col_name;
    public:
        CreateIndexStatement(const string &_index_name,const string &_table_name,const string &_col_name);
        string str() const;
        void print() const;
        void execute();
    };

    class DropIndexStatement : public Statement {
    private:
        string index_name;
    public:
        DropIndexStatement(const string &_index_name);
        string str() const;
        void print() const;
        void execute();
    };

    class SelectStatement : public Statement {
    private:
        string table_name;
        vector<string> proj_cols;
        WhereCond cond;
    public:
        SelectStatement(const string &_table_name,const vector<string> &_proj_cols,const WhereCond &_cond);
        string str() const;
        void print() const;
        void execute();
    };

    class InsertStatement : public Statement {
    private:
        string table_name;
        InsertTuples tps;
    public:
        InsertStatement(const string &_table_name,const InsertTuples &_tps);
        string str() const;
        void print() const;
        void execute();
    };

    class DeleteStatement : public Statement {
    private:
        string table_name;
        WhereCond cond;
    public:
        DeleteStatement(const string &_table_name,const WhereCond &_cond);
        string str() const;
        void print() const;
        void execute();
    };

    class ExecfileStatement : public Statement {
    private:
        string file_name;
    public:
        ExecfileStatement(const string &_file_name);
        string str() const;
        void print() const;
        void execute();
    };

	class ShowTablesStatement : public Statement {
	private:
		void _execute();
	public:
		ShowTablesStatement();
		string str() const;
		void print() const;
		void _execute();
		void execute();
	};

	class ShowIndexesStatement : public Statement {
	public:
		ShowIndexesStatement();
		string str() const;
		void print() const;
		void _execute();
		void execute();
	};
}

#endif