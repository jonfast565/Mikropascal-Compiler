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
class SymCallable;
class SymData;
class SymArgument;

// type predecls
using ArgumentPtr = shared_ptr<SymArgument>;
using ArgumentList = vector<ArgumentPtr>;
using ArgumentListPtr = shared_ptr<ArgumentList>;
using SymbolPtr = shared_ptr<Symbol>;
using SymbolList = vector<SymbolPtr>;
using SymbolListPtr = shared_ptr<SymbolList>;
using SymbolIterator = SymbolList::iterator;
using SymCallablePtr = shared_ptr<SymCallable>;
using SymDataPtr = shared_ptr<SymData>;

// symbol types
enum SymType {
	SYM_CALLABLE,
	SYM_DATA
};

// variable types
enum VarType {
	STRING,
	INTEGER,
	FLOATING,
	BOOLEAN,
    VOID
};

enum LiteralType {
	STRING_LITERAL,
	INTEGER_LITERAL,
	FLOATING_LITERAL
};

enum PassType {
	VALUE,
	REFERENCE
};

enum Scope {
	GLOBAL,
	LOCAL
};

// symbol class
class Symbol {
private:
	SymType symbol_type;
	string symbol_name;
	Scope symbol_scope;
    unsigned int nesting_level;
public:
	Symbol(string name, SymType type, Scope scope, unsigned int nesting_level):
		symbol_type(type), symbol_name(name), symbol_scope(scope), nesting_level(nesting_level){};
	virtual ~Symbol() = default;
    SymType get_symbol_type();
    string get_symbol_name();
    Scope get_symbol_scope();
    unsigned int get_nesting_level();
    virtual void dyn() = 0;
};

class SymTable {
private:
    SymbolListPtr symbol_list;
    SymbolIterator table_iter;
    SymCallablePtr last_callable;
    unsigned int nesting_level;
    void print_internal(SymbolListPtr symbol_list);
public:
    SymTable() {
        this->symbol_list = SymbolListPtr(new SymbolList());
        this->table_iter = this->symbol_list->begin();
        this->last_callable = nullptr;
        this->nesting_level = 0;
    }
    virtual ~SymTable() = default;
    void add_symbol(SymbolPtr new_symbol);
    void create_callable(string name, VarType return_type, ArgumentListPtr args);
    void create_data(string name, VarType type);
    ArgumentPtr create_argument(string name, VarType type, PassType pass);
    void go_into();
    void return_from();
    void to_latest();
    SymbolIterator position();
    void print();
};

class SymCallable : public Symbol {
private:
    VarType return_type;
    SymbolListPtr child;
    SymbolListPtr parent;
    ArgumentListPtr argument_list;
public:
	SymCallable(string name, Scope scope, unsigned int nesting_level, VarType return_type, SymbolListPtr parent):
		Symbol(name, SYM_CALLABLE, scope, nesting_level), return_type(return_type), parent(parent) {
		this->argument_list = ArgumentListPtr(new ArgumentList());
        this->child = SymbolListPtr(new SymbolList());
	}
    SymCallable(string name, Scope scope, unsigned int nesting_level, VarType return_type, SymbolListPtr parent, ArgumentListPtr argument_list):
    	Symbol(name, SYM_CALLABLE, scope, nesting_level), return_type(return_type), parent(parent), argument_list(argument_list) {
        this->child = SymbolListPtr(new SymbolList());
	}
	virtual ~SymCallable() = default;
    SymbolIterator return_sub_iterator();
    SymbolListPtr get_parent();
    SymbolListPtr get_child();
    VarType get_return_type();
    ArgumentListPtr get_argument_list();
    void dyn(){};
};

class SymData : public Symbol {
private:
	VarType variable_type;
public:
	SymData(string name, VarType type, Scope scope, unsigned int nesting_level):
		Symbol(name, SYM_DATA, scope, nesting_level), variable_type(type){};
	virtual ~SymData() = default;
    void dyn(){};
};

class SymArgument : public SymData {
private:
	PassType pass_type;
public:
	SymArgument(string name, VarType type, Scope scope, unsigned int nesting_level, PassType pass_type):
    SymData(name, type, scope, nesting_level), pass_type(pass_type){};
	virtual ~SymArgument() = default;
    PassType get_pass_type();
    void dyn(){};
};

#endif
