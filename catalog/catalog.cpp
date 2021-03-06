/*
 * MeSQL - catalog - catalog.cpp
 * 
 * This file defines the classes declared in catalog.hpp
 * 
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cassert>
#include "catalog.hpp"
#include "../config.hpp"

using namespace std;
using namespace MeCat;

static size_t file_len(istream &is) {
	auto pos = is.tellg();
	is.seekg(0,is.end);
	size_t ret = is.tellg();
	is.seekg(pos);
	return ret;
}

template<typename T> static void read_from_stream(T &val,stringstream &is) {
	is.read((char *)&val,sizeof (val));
}

template<typename T> static void write_to_stream(T &val,stringstream &os) {
	os.write((const char *)&val,sizeof (val));
}

template<typename T> static void parse_from_stream(T &val,stringstream &is);

template<typename T> static void embed_to_stream(T &val,stringstream &os);

template<> void parse_from_stream<string>(string &val,stringstream &is) {
	size_t len;
	read_from_stream(len,is);
	char *str = new char[len+1];
	is.read(str,len),str[len] = 0;
	val.assign(str);
	delete str;
}

template<> void embed_to_stream<const string>(const string &val,stringstream &os) {
	size_t len = val.length();
	write_to_stream(len,os);
	os.write(val.c_str(),len);
}

template<typename T> static void parse_from_stream(vector<T> &vec,stringstream &is) {
	typename vector<T>::size_type len;
	read_from_stream(len,is);
	vec.resize(len);
	for (T &val:vec) parse_from_stream(val,is);
}

template<typename T> static void embed_to_stream(const vector<T> &vec,stringstream &os) {
	typename vector<T>::size_type len = vec.size();
	write_to_stream(len,os);
	for (const T &val:vec) embed_to_stream(val,os);
}

template<> void parse_from_stream<IndexInfo>(IndexInfo &val,stringstream &is) {
	parse_from_stream(val.def.index_name,is);
	parse_from_stream(val.def.table_name,is);
	read_from_stream(val.def.col_ord,is);
	parse_from_stream(val.path,is);
}

template<> void embed_to_stream<const IndexInfo>(const IndexInfo &val,stringstream &os) {
	embed_to_stream(val.def.index_name,os);
	embed_to_stream(val.def.table_name,os);
	write_to_stream(val.def.col_ord,os);
	embed_to_stream(val.path,os);
}

template<> void parse_from_stream<TableInfo>(TableInfo &val,stringstream &is) {
	parse_from_stream(val.def.table_name,is);
	parse_from_stream(val.def.col_def,is);
	read_from_stream(val.def.pk_ord,is);
	parse_from_stream(val.path,is);
}

template<> void embed_to_stream<const TableInfo>(const TableInfo &val,stringstream &os) {
	embed_to_stream(val.def.table_name,os);
	embed_to_stream(val.def.col_def,os);
	write_to_stream(val.def.pk_ord,os);
	embed_to_stream(val.path,os);
}

template<> void parse_from_stream<TableColumnDef>(TableColumnDef &val,stringstream &is) {
	read_from_stream(val.ord,is);
	parse_from_stream(val.col_name,is);
	read_from_stream(val.col_spec,is);
}

template<> void embed_to_stream<const TableColumnDef>(const TableColumnDef &val,stringstream &os) {
	write_to_stream(val.ord,os);
	embed_to_stream(val.col_name,os);
	write_to_stream(val.col_spec,os);
}

template<> void parse_from_stream< map<string,TableInfo> >(map<string,TableInfo> &mp,stringstream &is) {
	typename map<string,TableInfo>::size_type len,i;
	TableInfo ti;
	read_from_stream(len,is);
	mp.clear();
	if (len == 0) return;
	typename map<string,TableInfo>::iterator it = mp.end();
	for (i=0;i<len;++i) {
		parse_from_stream(ti,is);
		it = mp.emplace_hint(it,ti.def.table_name,ti);
	}
}

template<> void embed_to_stream< map<string,TableInfo> >(map<string,TableInfo> &mp,stringstream &os) {
	typename map<string,TableInfo>::size_type len = mp.size();
	write_to_stream(len,os);
	for (const pair<string,TableInfo> &pr:mp) embed_to_stream(pr.second,os);
}

template<> void parse_from_stream< map<string,IndexInfo> >(map<string,IndexInfo> &mp,stringstream &is) {
	typename map<string,IndexInfo>::size_type len,i;
	IndexInfo di;
	read_from_stream(len,is);
	mp.clear();
	if (len == 0) return;
	typename map<string,IndexInfo>::iterator it = mp.end();
	for (i=0;i<len;++i) {
		parse_from_stream(di,is);
		it = mp.emplace_hint(it,di.def.index_name,di);
	}
}

template<> void embed_to_stream< map<string,IndexInfo> >(map<string,IndexInfo> &mp,stringstream &os) {
	typename map<string,IndexInfo>::size_type len = mp.size();
	write_to_stream(len,os);
	for (const pair<string,IndexInfo> &pr:mp) embed_to_stream(pr.second,os);
}

namespace MeCat {

    // implement class TableColumnSpec
	TableColumnSpec::TableColumnSpec() : data_type(DataType::UNKNOWN),len(0),is_unique(false),is_primary_key(false) {}
    TableColumnSpec::TableColumnSpec(DataType _data_type,char_size_t _len,bool _is_unique,bool _is_primary_key) :
            data_type(_data_type),len(_len),is_unique(_is_unique),is_primary_key(_is_primary_key) {}

    string TableColumnSpec::str() const {
        return "TableColumnSpec::str() is not implemented";
    }
	bool TableColumnSpec::try_to_fit(Literal &lit) const {
		if (data_type == DataType::FLOAT && lit.dtype == DataType::INT) {
			lit = Literal(static_cast<float>(lit.int_val));
		}
		if (data_type != lit.dtype) return false;
		if (data_type == DataType::CHAR) return static_cast<int>(lit.char_val.length()) <= len;
		return true;
	}

    // implement class TableColumnDef
	TableColumnDef::TableColumnDef() : ord(0),col_name(),col_spec() {}
    TableColumnDef::TableColumnDef(col_num_t _ord,const string &_col_name,const TableColumnSpec &_col_spec) :
            ord(_ord),col_name(_col_name),col_spec(_col_spec) {}
	TableColumnDef::TableColumnDef(const string &_col_name,DataType _data_type,char_size_t _len) :
		ord(0),col_name(_col_name),col_spec(_data_type,_len,false,false) {}

    string TableColumnDef::str() const {
        static stringstream ss;
		ss.str("");
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
	TableInfo::TableInfo() : def(),path(),index_on() {}
	TableInfo::TableInfo(const TableDef &_def,const string &_path) : def(_def),path(_path),index_on() {}
	bool TableInfo::try_to_fit_tuple(vector<Literal> &vec) const {
		size_t len = def.col_def.size();
		if (len != vec.size()) return false;
		for (size_t i=0;i<len;++i) if (!def.col_def[i].col_spec.try_to_fit(vec[i])) return false;
		return true;
	}

	// implement class IndexInfo
	IndexInfo::IndexInfo() : def(),path(),col() {}
	IndexInfo::IndexInfo(const IndexDef &_def,const string &_path) : def(_def),path(_path),col() {}


	// implement class CatalogManager
	CatalogManager::CatalogManager() : tables(),indexes() {
		load();
	}
	CatalogManager::~CatalogManager() {
		dump();
	}
	void CatalogManager::dump() {
		static stringstream ss;
		ss.str("");
		embed_to_stream(tables,ss);
		embed_to_stream(indexes,ss);
		ofstream f(CATA_FILE);
		f << ss.str();
		f.close();
	}
	void CatalogManager::load() {
		static stringstream ss;
		static string aux;
		ifstream f(CATA_FILE);
		if (f.fail()) return; // file not exists, load nothing
		size_t len = file_len(f);
		char *str = new char[len];
		f.read(str,len);
		f.close();
		aux.assign(str,str+len);
		ss.str(aux);
		delete str;
		parse_from_stream(tables,ss);
		parse_from_stream(indexes,ss);
		for (pair<const string,IndexInfo> &pr:indexes) {
			TableInfo &ti = tables.at(pr.second.def.table_name);
			pr.second.col = ti.def.col_def.at(pr.second.def.col_ord);
			ti.index_on.insert(pr.second.def.index_name);
		}
		for (pair<const string,TableInfo> &pr:tables) {
			for (const TableColumnDef &def:pr.second.def.col_def) 
				pr.second.name_to_ord.emplace(def.col_name,def.ord);
		}
	}
	TableInfo& CatalogManager::create_table_catalog(const string &table_name,const vector<TableColumnDef> &cols,col_num_t pk) {
		TableInfo ti(TableDef(table_name,cols,pk),"");
		for (const TableColumnDef &def:ti.def.col_def)
			ti.name_to_ord.emplace(def.col_name,def.ord);
		auto pr = tables.emplace(table_name,ti);
		assert(pr.second);
		return pr.first->second;
	}
	void CatalogManager::erase_table_catalog(const string &table_name) {
		assert(tables.at(table_name).index_on.empty());
		tables.erase(table_name);
	}
	IndexInfo& CatalogManager::create_index_catalog(const string &index_name,const TableInfo &ti,col_num_t ord) {
		IndexInfo di(IndexDef(index_name,ti.def.table_name,ord),"");
		di.col = ti.def.col_def.at(ord);
		IndexInfo &ret = indexes.emplace(index_name,di).first->second;
		tables.at(ti.def.table_name).index_on.insert(index_name);	
		return ret;
	}
	void CatalogManager::erase_index_catalog(const string &index_name) {
		assert(indexes.count(index_name) == 1);
		IndexInfo &di = indexes.at(index_name);
		tables.at(di.def.table_name).index_on.erase(di.def.index_name);
		indexes.erase(index_name);
	}

	pair<size_t,size_t> calc_len(const vector<TableColumnDef> &col_def) {
		pair<size_t,size_t> ret(0,0);
		size_t &in_len = ret.first;
		size_t &out_len = ret.second;
		for (const TableColumnDef &col:col_def) in_len += col.col_spec.len;
		out_len = in_len + sizeof (size_t) * 3;
		return ret;
	}
}