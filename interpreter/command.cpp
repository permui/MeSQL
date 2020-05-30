/* 
 * MeSQL - Interpreter Component - command.cpp
 * 
 * This file implements the classes defined in command.hpp .
 */


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../base/base.hpp"
#include "../base/color.hpp"
#include "command.hpp"

using namespace std;

static const COLOR default_color = COLOR::blue;

namespace MeInt {
    using namespace MeType;
    using namespace MeInfo;

    // implement class Statement
    Statement::Statement() : content(),printer(&cerr) {} // note this printer initialization

    // implement class CreateTableStatement
    CreateTableStatement::CreateTableStatement(const string &_table_name,const vector<TableColumnDef> &_cols,const string &_primary_key) :
        table_name(_table_name),cols(_cols),primary_key(_primary_key) {}

    string CreateTableStatement::str() const {
        stringstream ss;
        ss << colorful("create table statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "columns definition :" << endl;
        for (const TableColumnDef &c:cols) ss << '\t' << c.str() << endl;
        ss << "primary key : " << primary_key << endl;
        return ss.str();
    }
    void CreateTableStatement::print() const {
        *printer << str() << endl;
    }
    void CreateTableStatement::execute() {
        // TODO
        // currently just print
        print();
    }
    void CreateTableStatement::give_order() {
        for (size_t i=0;i<cols.size();++i) cols[i].ord = i;
    }

    // implement class DropTableStatement
    DropTableStatement::DropTableStatement(const string &_table_name) : table_name(_table_name) {}
    string DropTableStatement::str() const {
        stringstream ss;
        ss << colorful("drop table statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        return ss.str();
    }
    void DropTableStatement::print() const {
        *printer << str() << endl;
    }
    void DropTableStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class CreateIndexStatement
    CreateIndexStatement::CreateIndexStatement(const string &_index_name,const string &_table_name,const string &_col_name) :
        index_name(_index_name),table_name(_table_name),col_name(_col_name) {}
    string CreateIndexStatement::str() const {
        stringstream ss;
        ss << colorful("create index statement :",default_color) << endl;
        ss << "index name : " << index_name << endl;
        ss << "table name : " << table_name << endl;
        ss << "column name : " << col_name << endl;
        return ss.str();
    }
    void CreateIndexStatement::print() const {
        *printer << str() << endl;
    }
    void CreateIndexStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class DropIndexStatement
    DropIndexStatement::DropIndexStatement(const string &_index_name) : index_name(_index_name) {}
    string DropIndexStatement::str() const {
        stringstream ss;
        ss << colorful("drop index statement :",default_color) << endl;
        ss << "index name : " << index_name << endl;
        return ss.str();
    }
    void DropIndexStatement::print() const {
        *printer << str() << endl;
    }
    void DropIndexStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class SelectStatement
    SelectStatement::SelectStatement(const string &_table_name,const vector<string> &_proj_cols,const WhereCond &_cond) :
        table_name(_table_name),proj_cols(_proj_cols),cond(_cond) {}
    string SelectStatement::str() const {
        stringstream ss;
        ss << colorful("select statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "project to columns : ";
        for (const string &col_name:proj_cols) ss << col_name << ' ';
        ss << endl;
        ss << "where conditions : " << cond.str() << endl;
        return ss.str();
    }
    void SelectStatement::print() const {
        *printer << str() << endl;
    }
    void SelectStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class InsertStatement
    InsertStatement::InsertStatement(const string &_table_name,const InsertTuples &_tps) :
        table_name(_table_name),tps(_tps) {}

    string InsertStatement::str() const {
        stringstream ss;
        ss << colorful("insert statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "insert tuples :" << endl;
        for (const InsertTuple &tup:tps) ss << '\t' << tup.str() << endl;
        return ss.str();
    }
    void InsertStatement::print() const {
        *printer << str() << endl;
    }
    void InsertStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class DeleteStatement
    DeleteStatement::DeleteStatement(const string &_table_name,const WhereCond &_cond) :
        table_name(_table_name),cond(_cond) {}

    string DeleteStatement::str() const {
        stringstream ss;
        ss << colorful("delte statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "where condition : " << cond.str() << endl;
        return ss.str();
    }
    void DeleteStatement::print() const {
        *printer << str() << endl;
    }
    void DeleteStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class ExecfileStatement
    ExecfileStatement::ExecfileStatement(const string &_file_name) : file_name(_file_name) {}

    string ExecfileStatement::str() const {
        stringstream ss;
        ss << colorful("execfile statement :",default_color) << endl;
        ss << "file name : " << file_name << endl;
        return ss.str();
    }
    void ExecfileStatement::print() const {
        *printer << str() << endl;
    }
    void ExecfileStatement::execute() {
        // TODO
        // now just print
        print();
    }
}