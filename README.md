# MeSQL

This is the course project of _**Database System**_ in Zhejiang University, completed independently in my first undergraduate year. The primary aim of this project is to design a basic Database Management System that is **efficient, modular, low coupling and extensible**.

## Some Short Stories

To me, the real highlight of this project is neither the *B+ Tree* nor the *Buffer Manager*, which are straight forward and not hard to implement. The real highlight is the **interpreter**. I spent quite a lot of time learning the [Flex](https://en.wikipedia.org/wiki/Flex_(lexical_analyser_generator)) & [Bison](https://en.wikipedia.org/wiki/GNU_Bison) toolset, since I want the extensibility brought by formal grammar specification. And finally I made it! Here I must express my gratitude to [Chris Narkiewicz](https://github.com/ezaquarii) for [his example](https://github.com/ezaquarii/bison-flex-cpp-example), without which I will never use Flex & Bison with C++ properly (which, in my opinion, is much harder than using them with C). 

This project is named as _MeSQL_ for two reasons. The first one is that the project is the first large software engineering experience of my own, so it starts with "Me". Second, it is interesting to have a name resembles the famous _MySQL_ DBMS, differing only in the form of pronoun.

Anyway, working on MeSQL is definitely one of the experiences in my undergraduate years that I will never forget.

## How to Build

The project generally works on Linux systems. Building prerequisites are to have `make` `g++` `flex` and `bison` installed and callable from shell. To download, build and run it, just execute the following commands in an appropriate folder:

``` bash
git clone https://github.com/permui/MeSQL.git
cd MeSQL
make release
./ui
```

Boom! An interactive MeSQL Shell is running.

## Commands

The following is a description of all the commands supported, with examples for you to try. If you use a command incorrectly, there will probably be some human readable lines to tell you why.

In the following description, `<something>` stands for something to be filled in, `[abc]` stands for an optional `abc` to be there, and `a | b` stands for `a` _or_ `b`.

### Supported Types

* `int` : Same as C++ `int`
* `float`: Same as C++ `float`
* `char(n)` : A string with fixed length. Length `n` should be in `[0,255]` .

In the following description, `<type_name_...>` refers to one of the above type.

### Create or Drop Table

Create a new table in the database.

Drop an existing table.

Informal grammar:

``` sql
create table <table_name> (
    <col_name_1> <type_name_1> [unique],
    <col_name_2> <type_name_2> [unique],
    ...
    <col_name_n> <type_name_n> [unique],
    primary key (<col_name_k>)
);

drop table <table_name>;
```

Example:

``` sql
create table tab (
    x int,
    y float,
    z char(10) unique,
    u int,
    primary key (x)
);

drop table tab;
```

### Insert

Insert tuples into a table.

Informal grammar:

``` sql
insert into <table_name> values <insert_tuple_1>[, <insert_tuple_2> [...]];

insert_tuple: ( <literal_1>[, <literal_2>[...]] )
```

Example:

``` sql
create table tab (x int, y float, z char(10), primary key (x));
insert into tab values (1, 2.3, 'abc'), (2, 3.4, 'efg');
select * from tab;

/* output on my computer:
+---+---------+-----+
| x | y       | z   |
+---+---------+-----+
| 2 | 3.40000 | efg |
| 1 | 2.30000 | abc |
+---+---------+-----+
2 rows in result ( 0.109 ms )
*/
```


### Select

Select rows satisfying some conditions from a table.

Informal grammar:

``` sql
select <an_asterisk_or_column_names_separated_by_comma> from <table_name>
[ where <where_conditions> ]
[ limit <int_literal> ];

where_conditions: <condition_1>[ and <condition_2> [...]]]
condition: <column_name> <compare_operator> <literal>
compare_operator: = | <> | < | > | <= | >=
```

Example:

``` sql
select * from students;
select * from students where id = 108000;
select id, name from students where name < 'Bob';
select id, name from students where name < 'Bob' and id < 108000 limit 10;
```

### Create or Drop Index

Create a **B+ Tree** index that will greatly speed up querying. Only works on columns with the `unique` specifier.

Drop an existing index. An error will be returned if the index is not exist and `if exists` is not included.

Informal grammar:

``` sql
create index idx on <table_name> (<column_name>);

drop index <index_name> [if exists];
```

Example:

``` sql
create index idx on tab (y);

drop index idx;
drop index idx if exists;
```


### Delete

Delete rows from a table.

``` sql
delete from <table_name>
[ where <where_conditions> ]
```

`where_conditions` is the same as in [*select*](#select) statement.

Example:

``` sql
delete from students where id > 10800;
delete from books;
```

### Execute SQL Script File

Informal grammar:

``` sql
execfile "<path_to_script_file>";
```

Both relative and absolute path applies.

Example:

``` sql
execfile "a.sql";
execfile "./folder/b.sql";
execfile "/home/me/other/c.sql";
```

## Clean the Build

The bash command

``` bash
make clean
```

will get rid of all the intermediate files generated by `make` .

However, the `./db_files` directory, where all the database storage resides, will not be cleaned. This means that the next time you build it, all the tables are still there.

