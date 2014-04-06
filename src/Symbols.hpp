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
#include "Helper.hpp"

class Symbol;
class SymCallable;
class SymData;
class SymArgument;
class SymTable;
struct BlockInfo;
class SymConstant;

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
using SymConstantPtr = shared_ptr<SymConstant>;
using SymArgumentPtr = shared_ptr<SymArgument>;
using SymTablePtr = shared_ptr<SymTable>;

// symbol types
enum SymType {
	SYM_CALLABLE,
	SYM_DATA,
    SYM_CONSTANT
};

static string sym_type_to_string(SymType type) {
    switch(type) {
        case SYM_CALLABLE:
            return "SYM_CALLABLE";
        case SYM_DATA:
            return "SYM_DATA";
        case SYM_CONSTANT:
            return "SYM_CONSTANT";
    }
}

// variable types
enum VarType {
	STRING,
	INTEGER,
	FLOATING,
	BOOLEAN,
    VOID,
    STRING_LITERAL,
	INTEGER_LITERAL,
	FLOATING_LITERAL,
    BOOLEAN_LITERAL_T,
    BOOLEAN_LITERAL_F,
    LPAREN,
    RPAREN,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    AND,
    OR,
    NOT,
    IEQ,
    ILT,
    IGT,
    ILE,
    IGE,
    INE,
};

static string var_type_to_string(VarType type) {
    switch(type) {
        case STRING:
            return "STRING";
        case INTEGER:
            return "INTEGER";
        case FLOATING:
            return "FLOATING";
        case BOOLEAN:
            return "BOOLEAN";
        case VOID:
            return "VOID";
        case STRING_LITERAL:
            return "STRING_LITERAL";
        case INTEGER_LITERAL:
            return "INTEGER_LITERAL";
        case FLOATING_LITERAL:
            return "FLOATING_LITERAL";
        case BOOLEAN_LITERAL_T:
            return "TRUE";
        case BOOLEAN_LITERAL_F:
            return "FALSE";
        case LPAREN:
            return "LPAREN";
        case RPAREN:
            return "RPAREN";
        case ADD:
            return "ADD";
        case SUB:
            return "SUB";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        case MOD:
            return "MOD";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case NOT:
            return "NOT";
        case IEQ:
            return "EQUAL";
        case ILT:
            return "LESS_THAN";
        case IGT:
            return "GREATER_THAN";
        case ILE:
            return "LESS_THAN_EQ";
        case IGE:
            return "GREATER_THAN_EQ";
        case INE:
            return "NOT_EQUAL";
    }
}

// operator validation functor
struct OPERATOR {
    OPERATOR(VarType op): op(op){}
    bool operator()() {
        if (op == ADD || op == SUB
            || op == MUL || op == DIV
            || op == MOD || op == AND
            || op == OR || op == NOT
            || op == IEQ || op == ILT
            || op == IGT || op == ILE
            || op == IGE || op == INE) {
            return true;
        } else {
            return false;
        }
    }
private:
    VarType op;
};

// pass types
enum PassType {
	VALUE,
	REFERENCE
};

static string pass_type_to_string(PassType type) {
    switch(type) {
        case VALUE:
            return "VALUE";
        case REFERENCE:
            return "REFERENCE";
    }
}

// scoping types
enum Scope {
	GLOBAL,
	LOCAL
};

static string scope_to_string(Scope type) {
    switch(type) {
        case GLOBAL:
            return "GLOBAL";
        case LOCAL:
            return "LOCAL";
    }
}

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
    unsigned int max_offset;
    shared_ptr<stack<unsigned int>> offset_scope;
    void print_internal(SymbolListPtr symbol_list);
public:
    SymTable(): nesting_level(0), max_offset(0) {
        this->symbol_list = SymbolListPtr(new SymbolList());
        this->table_iter = this->symbol_list->begin();
        this->last_callable = nullptr;
        this->offset_scope = shared_ptr<stack<unsigned int>>(new stack<unsigned int>);
    }
    virtual ~SymTable() = default;
    void add_symbol(SymbolPtr new_symbol);
    void create_callable(string name, VarType return_type, ArgumentListPtr args);
    void create_data(string name, VarType type);
    ArgumentPtr create_argument(string name, VarType type, PassType pass);
    void go_into();
    void return_from();
    void to_latest();
    void print();
    SymbolListPtr find(string id);
    SymbolListPtr get_global_vars();
    unsigned int get_level();
    unsigned int get_offset();
    SymbolIterator position();
    SymbolIterator get_first();
    SymbolIterator get_last();
    static SymbolListPtr filter_data(SymbolListPtr filterable);
    static SymbolListPtr filter_callable(SymbolListPtr filterable);
    static SymbolListPtr filter_nest_level(SymbolListPtr filterable, unsigned int nest_level);
    SymbolListPtr data_in_scope_at(string id, unsigned int level);
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
    SymbolIterator return_sub_end_iterator();
    SymbolListPtr get_parent();
    SymbolListPtr get_child();
    VarType get_return_type();
    ArgumentListPtr get_argument_list();
    unsigned int get_number_arguments();
    shared_ptr<vector<VarType>> get_argument_types();
    void dyn(){};
};

class SymData : public Symbol {
private:
	VarType variable_type;
    SymCallablePtr parent_callable;
    string address;
public:
	SymData(string name, VarType type, Scope scope, unsigned int nesting_level, SymCallablePtr parent_callable):
		Symbol(name, SYM_DATA, scope, nesting_level), variable_type(type), parent_callable(parent_callable), address(""){};
	virtual ~SymData() = default;
    VarType get_var_type();
    void set_address(unsigned int level, unsigned int offset);
    string get_address();
    SymCallablePtr get_parent_callable();
    void dyn(){};
};

class SymConstant : public Symbol {
private:
    VarType constant_type;
    string raw_data;
public:
    SymConstant(string data, VarType constant_type) :
    Symbol(data, SYM_CONSTANT, GLOBAL, 0), raw_data(data),
    constant_type(constant_type){};
    VarType get_constant_type();
    string get_data();
    void dyn(){};
};

class SymArgument : public SymData {
private:
	PassType pass_type;
public:
	SymArgument(string name, VarType type, Scope scope, unsigned int nesting_level, PassType pass_type, SymCallablePtr parent_callable):
    SymData(name, type, scope, nesting_level, parent_callable), pass_type(pass_type){};
	virtual ~SymArgument() = default;
    PassType get_pass_type();
    void dyn(){};
};

#endif
