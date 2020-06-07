/*
 * MeSQL - catalog - catalog.hpp
 * 
 * This file declares class CatalogManager and other 
 * related classes.
 * 
 */

#ifndef CATALOG_HPP
#define CATALOG_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "../base/base.hpp"
#include "../config.hpp"
using namespace std;

namespace MeCat {

	using namespace MeType;

    /* Column specification : col_spec = (col_type col_constraint) + is_primary_key */
    class TableColumnSpec { // fixed length
    public:
        DataType data_type;
        char_size_t len; // 0 if not needed
        bool is_unique;
        bool is_primary_key;

		TableColumnSpec();
        TableColumnSpec(DataType _data_type,char_size_t _len,bool _is_unique,bool _is_primary_key);
        string str() const;
    };

    /* Column definition : col_defi = (col_name col_spec) */
    class TableColumnDef { // var length due to col_name
    public:
        col_num_t ord;
        string col_name;
        TableColumnSpec col_spec;

		TableColumnDef();
        TableColumnDef(col_num_t _ord,const string &_col_name,const TableColumnSpec &_col_spec);
		TableColumnDef(const string &_col_name,DataType _data_type,char_size_t _len);
        string str() const;
    };

	class TableDef {
	public:
		string table_name;
		vector<TableColumnDef> col_def;
		col_num_t pk_ord;

		TableDef();
		TableDef(const string &_name,const vector<TableColumnDef> &_cols,const col_num_t &_pk_ord);
	};

	class IndexDef {
	public:
		string index_name;
		string table_name;
		col_num_t col_ord;

		IndexDef();
		IndexDef(const string &_index_name,const string &_table_name,const col_num_t &_col_ord);
	};

	class TableInfo {
	public:
		TableDef def;
		string path;

		TableInfo();
		TableInfo(const TableDef &_def,const string &_path);
		void load_from_ss(stringstream&);
		void dump_to_ss(stringstream&);
	};

	class IndexInfo {
	public:
		IndexDef def;
		string path;

		IndexInfo();
		IndexInfo(const IndexDef &_def,const string &_path);
		void load_from_ss(stringstream&);
		void dump_to_ss(stringstream&);
	};

	class CatalogManager { // should be implemented in map<name,Info> TODO
	public:
		map<string,TableInfo> tables;
		map<string,IndexInfo> indexes;

		CatalogManager();
		~CatalogManager();
		void load(); // load catalog into class
		void dump(); // dump catalog to file
	};

}

#endif