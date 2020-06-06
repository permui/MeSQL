#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include "base.hpp"

using namespace std;

namespace MeType {

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

}

namespace MeInfo {
    using namespace MeType;

    // implement class Literal
	Literal::Literal() : dtype(DataType::UNKNOWN),int_val(0),float_val(0),char_val() {}
    Literal::Literal(int _val) : dtype(DataType::INT),int_val(_val),float_val(0),char_val() {}
    Literal::Literal(float _val) : dtype(DataType::FLOAT),int_val(0),float_val(_val),char_val() {}
    Literal::Literal(const string &_val) : dtype(DataType::CHAR),int_val(0),float_val(0),char_val(_val) {}

    string Literal::str() const {
        stringstream ss;
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

    // implement class WhereCondItem
	WhereCondItem::WhereCondItem() : col_name(),op(CompareOp::UNKNOWN),lit() {}
    WhereCondItem::WhereCondItem(const string &_col_name,CompareOp _op,const Literal &_lit) :
        col_name(_col_name),op(_op),lit(_lit) {}

    string WhereCondItem::str() const {
        stringstream ss;
        ss << col_name << ' ' << CompareOpStr(op) << ' ' << lit.str();
        return ss.str();
    }

    // implement class WhereCond
    WhereCond::WhereCond() : items() {}
    WhereCond::WhereCond(const vector<WhereCondItem> &_items) : items(_items) {}

    string WhereCond::str() const {
        stringstream ss;
        for (const WhereCondItem &item:items) ss << '[' << item.str() << ']' << ' ';
        return ss.str();
    }

    // implement class InsertTuple
    string InsertTuple::str() const {
        stringstream ss;
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