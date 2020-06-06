/*
 * MeSQL - config.hpp
 * 
 * This file stores basic config parameters as header.
 * 
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef> 

using namespace std;

// constant for buffer
static const size_t block_size = 4096; // 4KB
static const size_t buffer_size = 4 * 1024 * 1024; // 4MB
static const size_t block_num = buffer_size / block_size;
static const size_t hash_table_size = 100003; // a prime number

// path for directory db_files for code in one layer of folder
#define DB_FILES "db_files/"


#endif