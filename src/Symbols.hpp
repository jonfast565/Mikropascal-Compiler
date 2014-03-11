//
//  Symbols.h
//  CompilerTest
//
//  Created by Jonathan Fast on 2/9/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#ifndef symbols_h
#define symbols_h

#include "Standard.hpp"

class Symbol;

enum PassType {
    REFERENCE,
    VALUE,
    UNDEFPASS
};

enum SymbolType {
    DATA,
    FUNCTION,
    PROCEDURE
};

enum SymbolScope {
    LOCAL,
    GLOBAL, //static
    PARAM
};

enum DataType {
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN
};

class SymbolTable {
private:
    unsigned int current_level;
    shared_ptr<vector<shared_ptr<Symbol>>> table;
    shared_ptr<Symbol> current_symbol;
    
public:
    SymbolTable();
    void add_symbol(shared_ptr<Symbol> new_symbol);
    void go_into_callable();
    void return_from_callable();
    virtual ~SymbolTable() = default;
};

class Symbol {
private:
    string bound_name;
    SymbolType symbol_type;
    SymbolScope symbol_scope;
    DataType return_type;
    unsigned int nesting_level;
    unsigned int parameter_number;
    shared_ptr<vector<tuple<string, DataType, PassType>>> parameter_names;
    shared_ptr<SymbolTable> callable;
public:
    // data
    Symbol(string bound_name, SymbolScope scope, unsigned int nesting_level);
    // procedure
    Symbol(string bound_name,
           SymbolScope scope,
           unsigned int nesting_level,
           shared_ptr<vector<tuple<string, DataType, PassType>>> parameter_names);
    // function
    Symbol(string bound_name,
           SymbolScope scope,
           unsigned int nesting_level,
           shared_ptr<vector<tuple<string, DataType, PassType>>> parameter_names,
           DataType return_type);
    
    // getters
    string get_bound_name();
    SymbolType get_symbol_type();
    SymbolScope get_symbol_scope();
    unsigned int get_nesting_level();
    unsigned int get_number_parameters();
    shared_ptr<vector<tuple<string, DataType, PassType>>> get_parameter_names();
    shared_ptr<SymbolTable> get_callable();
    virtual ~Symbol() = default;
};

#endif
