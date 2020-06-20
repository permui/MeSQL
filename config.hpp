/*
 * MeSQL - config.hpp
 * 
 * This file stores basic config parameters as header.
 * 
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef> 
#include <limits>
#include "base/base.hpp"

using namespace std;

// constant for buffer
static const size_t block_size = 4096; // 4KB
static const size_t block_num = 1024;
static const size_t buffer_size = block_size * block_num;
static const size_t max_block_ord = numeric_limits<size_t>::max() / block_size + 1;
static const size_t hash_table_size = 100003; // a prime number

// path for directory db_files for code in one layer of folder
#define DB_FILES "db_files/"
#define CATA_FILE DB_FILES "catalog/catalog.cat"
#define TABLE_DIR DB_FILES "table/"
#define TABLE_SUF ".tab"
#define INDEX_DIR DB_FILES "index/"
#define INDEX_SUF ".idx"

// this should not be modified
static const MeType::char_size_t max_CHAR_len = 255;

static const MeType::char_size_t max_tablename = 255 - 3;
static const MeType::char_size_t max_indexname = 255;

static const MeType::col_num_t max_col_num = 32;


#define InternalError "InternalError"


#endif