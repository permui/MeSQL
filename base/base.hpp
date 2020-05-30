/*
 * MeSQL - Base - base.hpp
 * 
 * This header contains basic class and type declarations.
 * 
 */

#ifndef BASE_HPP
#define BASE_HPP

#include <string>
#include <vector>

using namespace std;

namespace MeType {
    enum class DataType {
        INT,FLOAT,CHAR
    };
    enum class CompareOp {
        EQ,NE,L,G,LE,GE
    };
    typedef unsigned char char_size_t;
    typedef unsigned char col_num_t;

    string DataTypeStr(DataType dtype);
    string CompareOpStr(CompareOp op);
}

namespace MeInfo {
    using namespace MeType;

    /* Column specification : col_spec = (col_type col_constraint) + is_primary_key */
    class TableColumnSpec {
    public:
        DataType data_type;
        char_size_t len; // 0 if not needed
        bool is_unique;
        bool is_primary_key;

        TableColumnSpec(DataType _data_type,char_size_t _len,bool _is_unique,bool _is_primary_key);
        string str() const;
    };

    /* Column definition : col_defi = (col_name col_spec) */
    class TableColumnDef {
    public:
        col_num_t ord;
        string col_name;
        TableColumnSpec col_spec;

        TableColumnDef(col_num_t _ord,const string &_col_name,const TableColumnSpec &_col_spec);
        string str() const;
    };

    class Literal {
    public:
        DataType dtype;
        int int_val;
        float float_val;
        string char_val;

        Literal(int _val);
        Literal(float _val);
        Literal(const string &_val);
        string str() const;
    };

    class WhereCondItem {
    public:
        string col_name;
        CompareOp op;
        Literal lit;

        WhereCondItem(const string &_col_name,CompareOp _op,const Literal &_lit);
        string str() const;
    };

    class WhereCond {
    public:
        vector<WhereCondItem> items;

        WhereCond();
        WhereCond(const vector<WhereCondItem> &_items);
        string str() const;
    };

    class InsertTuple : public vector<Literal> {
    public:
        string str() const;
    };

    class InsertTuples : public vector<InsertTuple> {
    public:
        string str() const;
    };
}

#endif