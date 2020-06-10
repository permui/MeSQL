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
#include <algorithm>

using namespace std;

namespace MeType {
    enum class DataType {
        INT,FLOAT,CHAR,UNKNOWN
    };
    enum class CompareOp {
        EQ,NE,L,G,LE,GE,UNKNOWN
    };
    typedef int char_size_t;
    typedef int col_num_t; // cannot be too small ; used in ResultManager

	int DataTypeLen(DataType dtype); // if unknown -1
    string DataTypeStr(DataType dtype);
    string CompareOpStr(CompareOp op);
	template<typename T> string to_str(const T &x);
}

namespace MeInfo {
    using namespace MeType;

    class Literal {
    public:
        DataType dtype;
        int int_val;
        float float_val;
        string char_val;

		Literal();
        Literal(int _val);
        Literal(float _val);
        Literal(const string &_val);
        string str() const;
		string str_unquoted() const;
		bool operator< (const Literal &b) const;
		bool operator== (const Literal &b) const;
		bool null() const;
    };

	bool CompareOpApply(const Literal &a,CompareOp op,const Literal &b);
	template<typename T> bool CompareOpSpec(const T &a,CompareOp op,const T &b);

    class WhereCondItem {
    public:
        string col_name;
        CompareOp op;
        Literal lit;

		WhereCondItem();
        WhereCondItem(const string &_col_name,CompareOp _op,const Literal &_lit);
        string str() const;
    };

	class WhereCondOtem {
	public:
		col_num_t col_ord;
		CompareOp op;
		Literal lit;
		bool has_index;

		WhereCondOtem(col_num_t _col_ord,CompareOp _op,const Literal &_lit);
	};

    class WhereCond {
    public:
        vector<WhereCondItem> items;
		vector<WhereCondOtem> otems;

        WhereCond();
        WhereCond(const vector<WhereCondItem> &_items);
		bool satisfy(const vector<Literal> &tup,bool logic_and);
        string str() const;
    };

    class InsertTuple : public vector<Literal> {
    public:
		using vector<Literal>::vector;
        string str() const;
    };

    class InsertTuples : public vector<InsertTuple> {
    public:
		using vector<InsertTuple>::vector;
        string str() const;
    };
}

namespace MeGad {
	
	template<typename T> bool check_unique(vector<T> vec);

}

#endif