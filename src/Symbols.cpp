//
//  Symbols.cpp
//  CompilerTest
//
//  Created by Jonathan Fast on 2/9/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#include "Symbols.hpp"

SymbolTable::SymbolTable() {
    this->current_level = 0;
    this->current_symbol = nullptr;
    this->table = shared_ptr<vector<shared_ptr<Symbol>>>(new vector<shared_ptr<Symbol>>);
}

// symbol table manipulation
void SymbolTable::add_symbol(shared_ptr<Symbol> new_symbol) {
    // push symbol onto the table
    this->table->push_back(new_symbol);
    // set the current symbol
    this->current_symbol = new_symbol;
}

void SymbolTable::go_into_callable() {
    if (this->current_symbol->get_callable() != nullptr) {
        // go into callable table here
    }
    //otherwise, is not a proc or function
}

void SymbolTable::return_from_callable() {
    // go out of callable, unlessd nesting level is 0
}

// data
Symbol::Symbol(string bound_name, SymbolScope scope, unsigned int nesting_level) {
    this->nesting_level = nesting_level;
    this->bound_name = bound_name;
    this->symbol_scope = symbol_scope;
    this->callable = nullptr;
}

// procedure
Symbol::Symbol(string bound_name,
               SymbolScope scope,
               unsigned int nesting_level,
               shared_ptr<vector<tuple<string, DataType, PassType>>> parameter_names) {
    this->nesting_level = nesting_level;
    this->bound_name = bound_name;
    this->symbol_scope = symbol_scope;
    this->parameter_names = parameter_names;
    this->return_type = return_type;
    this->callable = shared_ptr<SymbolTable>(new SymbolTable());
}
// function
Symbol::Symbol(string bound_name,
               SymbolScope scope,
               unsigned int nesting_level,
               shared_ptr<vector<tuple<string, DataType, PassType>>> parameter_names,
               DataType return_type) {
    this->nesting_level = nesting_level;
    this->bound_name = bound_name;
    this->symbol_scope = symbol_scope;
    this->parameter_names = parameter_names;
    this->return_type = return_type;
    this->callable = shared_ptr<SymbolTable>(new SymbolTable());
}

// getters
string Symbol::get_bound_name() {
    return this->bound_name;
}

SymbolType Symbol::get_symbol_type() {
    return this->symbol_type;
}

SymbolScope Symbol::get_symbol_scope() {
    return this->symbol_scope;
}

unsigned int Symbol::get_nesting_level() {
    return this->nesting_level;
}

unsigned int Symbol::get_number_parameters() {
    return this->parameter_number;
}

shared_ptr<vector<tuple<string, DataType, PassType>>> Symbol::get_parameter_names() {
    return this->parameter_names;
}

shared_ptr<SymbolTable> Symbol::get_callable() {
    if (this->callable == nullptr)
        return nullptr;
    else
        return this->callable;
}
