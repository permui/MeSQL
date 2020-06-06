/*
 * MeSQL - catalog - catalog.cpp
 * 
 * This file defines the classes declared in catalog.hpp
 * 
 */

#include <sstream>
#include <fstream>
#include <vector>
#include "catalog.hpp"
#include "../config.hpp"

#define CATA_FILE DB_FILES "catalog/catalog.cat"

using namespace std;
using namespace MeCat;

static size_t file_len(istream &is) {
	auto pos = is.tellg();
	is.seekg(0,is.end);
	size_t ret = is.tellg();
	is.seekg(pos);
	return ret;
}

template<typename T> static void read_from_stream(T &val,istream &is) {
	is.read((char *)&val,sizeof (val));
}

template<typename T> static void write_to_stream(T &val,ostream &os) {
	os.write((const char *)&val,sizeof (val));
}

template<typename T> static void parse_from_stream(T &val,istream &is) {
	cerr << "Template Deduction Error : [original parse_from_stream] should not be called" << endl;
}

template<typename T> static void embed_to_stream(T &val,ostream &os) {
	cerr << "Template Deduction Error : [original embed_to_stream] should not be called" << endl;
}

template<> void parse_from_stream<string>(string &val,istream &is) {
	size_t len;
	read_from_stream(len,is);
	char *str = new char[len];
	is.read(str,len);
	val.assign(str);
	delete str;
}

template<> void embed_to_stream<string>(string &val,ostream &os) {
	size_t len = val.length();
	write_to_stream(len,os);
	os.write(val.c_str(),len);
}

template<typename T> static void parse_from_stream(vector<T> &vec,istream &is) {
	typename vector<T>::size_type len;
	read_from_stream(len,is);
	vec.resize(len);
	for (T &val:vec) parse_from_stream(val,is);
}

template<typename T> static void embed_to_stream(vector<T> &vec,ostream &os) {
	typename vector<T>::size_type len = vec.size();
	write_to_stream(len,os);
	for (T &val:vec) embed_to_stream(val,os);
}

template<> void parse_from_stream<IndexInfo>(IndexInfo &val,istream &is) {
	parse_from_stream(val.def.index_name,is);
	parse_from_stream(val.def.table_name,is);
	read_from_stream(val.def.col_ord,is);
	parse_from_stream(val.path,is);
}

template<> void embed_to_stream<IndexInfo>(IndexInfo &val,ostream &os) {
	embed_to_stream(val.def.index_name,os);
	embed_to_stream(val.def.table_name,os);
	write_to_stream(val.def.col_ord,os);
	embed_to_stream(val.path,os);
}

template<> void parse_from_stream<TableInfo>(TableInfo &val,istream &is) {
	parse_from_stream(val.def.table_name,is);
	parse_from_stream(val.def.col_def,is);
	read_from_stream(val.def.pk_ord,is);
	parse_from_stream(val.path,is);
}

template<> void embed_to_stream<TableInfo>(TableInfo &val,ostream &os) {
	embed_to_stream(val.def.table_name,os);
	embed_to_stream(val.def.col_def,os);
	write_to_stream(val.def.pk_ord,os);
	embed_to_stream(val.path,os);
}

template<> void parse_from_stream<TableColumnDef>(TableColumnDef &val,istream &is) {
	read_from_stream(val.ord,is);
	parse_from_stream(val.col_name,is);
	read_from_stream(val.col_spec,is);
}

template<> void embed_to_stream<TableColumnDef>(TableColumnDef &val,ostream &os) {
	write_to_stream(val.ord,os);
	embed_to_stream(val.col_name,os);
	write_to_stream(val.col_spec,os);
}

namespace MeCat {

    // implement class TableColumnSpec
	TableColumnSpec::TableColumnSpec() : data_type(DataType::UNKNOWN),len(0),is_unique(false),is_primary_key(false) {}
    TableColumnSpec::TableColumnSpec(DataType _data_type,char_size_t _len,bool _is_unique,bool _is_primary_key) :
            data_type(_data_type),len(_len),is_unique(_is_unique),is_primary_key(_is_primary_key) {}

    string TableColumnSpec::str() const {
        return "TableColumnSpec::str() is not implemented";
    }

    // implement class TableColumnDef
	TableColumnDef::TableColumnDef() : ord(0),col_name(),col_spec() {}
    TableColumnDef::TableColumnDef(col_num_t _ord,const string &_col_name,const TableColumnSpec &_col_spec) :
            ord(_ord),col_name(_col_name),col_spec(_col_spec) {}

    string TableColumnDef::str() const {
        stringstream ss;
        ss << ord << ": " << col_name << " ";
        ss << DataTypeStr(col_spec.data_type);
        if (col_spec.data_type==DataType::CHAR) ss << '(' << col_spec.len << ')';
        ss << ' ';
        if (col_spec.is_unique) ss << "[UNIQUE] ";
        if (col_spec.is_primary_key) ss << "[PK]";
        return ss.str();
    }

	// implement class TableDef
	TableDef::TableDef() : table_name(),col_def(),pk_ord(0) {}
	TableDef::TableDef(const string &_name,const vector<TableColumnDef> &_cols,const col_num_t &_pk_ord) :
		table_name(_name),col_def(_cols),pk_ord(_pk_ord) {}

	// implement class IndexDef
	IndexDef::IndexDef() : index_name(),table_name(),col_ord(0) {}
	IndexDef::IndexDef(const string &_index_name,const string &_table_name,const col_num_t &_col_ord) :
		index_name(_index_name),table_name(_table_name),col_ord(_col_ord) {}

	// implement class TableInfo
	TableInfo::TableInfo() : def(),path() {}
	TableInfo::TableInfo(const TableDef &_def,const string &_path) : def(_def),path(_path) {}

	IndexInfo::IndexInfo() : def(),path() {}
	IndexInfo::IndexInfo(const IndexDef &_def,const string &_path) : def(_def),path(_path) {}

	// implement class CatalogManager
	CatalogManager::CatalogManager() : tables(),indexes() {
		load();
	}
	CatalogManager::~CatalogManager() {
		dump();
	}
	void CatalogManager::dump() {
		stringstream ss;
		embed_to_stream(tables,ss);
		embed_to_stream(indexes,ss);
		ofstream f(CATA_FILE);
		f << ss.str();
		f.close();
	}
	void CatalogManager::load() {
		ifstream f(CATA_FILE);
		if (f.fail()) return; // file not exists, load nothing
		size_t len = file_len(f);
		char *str = new char[len];
		f.read(str,len);
		f.close();
		stringstream ss(str);
		delete str;
		parse_from_stream(tables,ss);
		parse_from_stream(indexes,ss);
	}
}