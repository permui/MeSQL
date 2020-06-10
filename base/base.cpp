#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <cassert>
#include "base.hpp"

using namespace std;

namespace MeType {

	int DataTypeLen(DataType dtype) {
		switch (dtype) {
			case DataType::INT : return 4;
			case DataType::FLOAT : return 4;
			default : return -1;
		}
	}

    string DataTypeStr(DataType dtype) {
        switch (dtype) {
            case DataType::INT : return "INT";
            case DataType::FLOAT : return "FLOAT";
            case DataType::CHAR : return "CHAR";
			case DataType::UNKNOWN : return "UNKNOWN";
        }
        return "UNDEFINED DATATYPE";
    }

    string CompareOpStr(CompareOp op) {
        switch (op) {
            case CompareOp::EQ : return "=";
            case CompareOp::NE : return "<>";
            case CompareOp::L  : return "<";
            case CompareOp::G  : return ">";
            case CompareOp::LE : return "<=";
            case CompareOp::GE : return ">=";
			case CompareOp::UNKNOWN : return "UNKNOWN COMPARE OPERATOR";
        }
        return "UNDEFINED OPERATOR";
    }

	template<typename T> string to_str(const T &x) {
		static stringstream ss;
		ss.str("");
		ss << x;
		return ss.str();
	}

}

namespace MeInfo {
    using namespace MeType;

    // implement class Literal
	Literal::Literal() : dtype(DataType::UNKNOWN),int_val(0),float_val(0),char_val() {}
    Literal::Literal(int _val) : dtype(DataType::INT),int_val(_val),float_val(0),char_val() {}
    Literal::Literal(float _val) : dtype(DataType::FLOAT),int_val(0),float_val(_val),char_val() {}
    Literal::Literal(const string &_val) : dtype(DataType::CHAR),int_val(0),float_val(0),char_val(_val) {}

    string Literal::str() const {
        static stringstream ss;
		ss.str("");
        switch (dtype) {
            case DataType::INT:
                ss << int_val; break;
            case DataType::FLOAT:
                ss << showpoint << float_val; break;
            case DataType::CHAR:
                ss << "'" << char_val << "'"; break; 
        }
        return ss.str();
    }

	string Literal::str_unquoted() const {
		static stringstream ss;
		ss.str("");
		switch (dtype) {
			case DataType::INT:
				ss << int_val; break;
			case DataType::FLOAT:
				ss << showpoint << float_val; break;
			case DataType::CHAR:
				ss.str(char_val); break;
		}
		return ss.str();
	}

	bool Literal::operator< (const Literal &b) const {
		assert(dtype == b.dtype);
		switch (dtype) {
			case DataType::INT : return int_val < b.int_val;
			case DataType::FLOAT : return float_val < b.float_val;
			case DataType::CHAR : return char_val < b.char_val;
		}
		assert(false);
		return false;
	}
	bool Literal::operator== (const Literal &b) const {
		assert(dtype == b.dtype);
		switch (dtype) {
			case DataType::INT : return int_val == b.int_val;
			case DataType::FLOAT : return float_val == b.float_val;
			case DataType::CHAR : return char_val == b.char_val;
		}
		assert(false);
		return false;
	}

	bool Literal::null() const {
		return dtype == DataType::UNKNOWN;
	}

	bool CompareOpApply(const Literal &a,CompareOp op,const Literal &b) {
		assert(op != CompareOp::UNKNOWN);
		assert(a.dtype == b.dtype);
		switch (a.dtype) {
			case DataType::INT : return CompareOpSpec(a.int_val,op,b.int_val);
			case DataType::FLOAT : return CompareOpSpec(a.float_val,op,b.float_val);
			case DataType::CHAR : return CompareOpSpec(a.char_val,op,b.char_val);
		}
		assert(false);
		return false;
	}

	template<typename T> bool CompareOpSpec(const T &a,CompareOp op,const T &b) {
		switch (op) {
			case CompareOp::EQ : return a == b;
			case CompareOp::NE : return a != b;
			case CompareOp::L  : return a <  b;
			case CompareOp::G  : return a >  b;
			case CompareOp::LE : return a <= b;
			case CompareOp::GE : return a >= b;
		}
		return false;
	}

    // implement class WhereCondItem
	WhereCondItem::WhereCondItem() : col_name(),op(CompareOp::UNKNOWN),lit() {}
    WhereCondItem::WhereCondItem(const string &_col_name,CompareOp _op,const Literal &_lit) :
        col_name(_col_name),op(_op),lit(_lit) {}

    string WhereCondItem::str() const {
        static stringstream ss;
		ss.str("");
        ss << col_name << ' ' << CompareOpStr(op) << ' ' << lit.str();
        return ss.str();
    }

	WhereCondOtem::WhereCondOtem(col_num_t _col_ord,CompareOp _op,const Literal &_lit) :
		col_ord(_col_ord),op(_op),lit(_lit),has_index(false) {}

    // implement class WhereCond
    WhereCond::WhereCond() : items(),otems() {}
    WhereCond::WhereCond(const vector<WhereCondItem> &_items) : items(_items),otems() {}

	bool WhereCond::satisfy(const vector<Literal> &tup,bool logic_and) {
		if (logic_and) {
			for (const WhereCondOtem &ot:otems) {
				bool tmp = CompareOpApply(tup.at(ot.col_ord),ot.op,ot.lit);
				if (!tmp) return false;
			}
			return true;
		} else { // logic or
			for (const WhereCondOtem &ot:otems) {
				bool tmp = CompareOpApply(tup.at(ot.col_ord),ot.op,ot.lit);
				if (tmp) return true;
			}
			return false;
		}
	}

    string WhereCond::str() const {
        static stringstream ss;
		ss.str("");
        for (const WhereCondItem &item:items) ss << '[' << item.str() << ']' << ' ';
        return ss.str();
    }

    // implement class InsertTuple
    string InsertTuple::str() const {
        static stringstream ss;
		ss.str("");
        ss << '(';
        for (const Literal &lit:*this) ss << lit.str() << ',';
        ss << ')';
        return ss.str();
    }

    // implement class InsertTuples
    string InsertTuples::str() const {
        return "InsertTuples::str() is not implemented";
    }


}

namespace MeGad {

 	template<typename T> bool check_unique(vector<T> vec) {
		size_t siz = vec.size();
		if (siz <= 1) return true;
		sort(vec.begin(),vec.end());
		for (size_t i=0;i+1<siz;++i) if (vec[i] == vec[i+1]) return false;
		return true;
	}

}