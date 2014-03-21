//
//  Symbols.cpp
//  CompilerTest
//
//  Created by Jonathan Fast on 2/9/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#include "Symbols.hpp"


void SymTable::add_symbol(SymbolPtr new_symbol) {
    this->symbol_list->push_back(new_symbol);
    this->to_latest();
}

void SymTable::go_into() {
    if (this->table_iter != this->symbol_list->end()) {
        if ((*this->table_iter)->get_symbol_type() == SYM_CALLABLE) {
            SymCallablePtr callable_obj = static_pointer_cast<SymCallable>(*this->table_iter);
            this->table_iter = callable_obj->return_sub_iterator();
            this->nesting_level++;
            this->last_callable = callable_obj;
        }
    }
}

void SymTable::return_from() {
    if (this->last_callable != nullptr)
        this->table_iter = this->last_callable->get_parent()->end() - 1;
    this->nesting_level--;
}

void SymTable::create_data(string name, VarType type) {
    Scope current_scope;
    if (nesting_level == 0) {
        current_scope = GLOBAL;
    } else {
        current_scope = LOCAL;
    }
    SymDataPtr p = SymDataPtr(
    new SymData(name, type, current_scope, this->nesting_level));
    this->symbol_list->push_back(p);
    this->to_latest();
}

void SymTable::create_callable(string name, VarType return_type, ArgumentListPtr args) {
    Scope current_scope;
    if (nesting_level == 0) {
        current_scope = GLOBAL;
    } else {
        current_scope = LOCAL;
    }
    SymCallablePtr c = SymCallablePtr(new SymCallable(
    name, current_scope, this->nesting_level, return_type,
    this->symbol_list, args));
    this->symbol_list->push_back(c);
    this->to_latest();
}

void SymTable::to_latest() {
    this->table_iter = this->symbol_list->end() - 1;
}

void SymTable::print() {
    SymbolListPtr first = this->symbol_list;
    print_internal(first);
}

void SymTable::print_internal(SymbolListPtr level_list) {
    for (auto i = level_list->begin(); i != level_list->end(); i++) {
        if ((*i)->get_symbol_type() == SYM_CALLABLE) {
            // cast to derived
            SymCallablePtr callable_obj = static_pointer_cast<SymCallable>(*i);
            // print its name and type, and also return type
            cout << callable_obj->get_symbol_name() << endl;
            cout << callable_obj->get_symbol_type() << endl;
            cout << callable_obj->get_return_type() << endl;
            // (void if procedure)
            // print its arguments
            for (auto j = callable_obj->get_argument_list()->begin();
                 j != callable_obj->get_argument_list()->end(); j++) {
                cout << (*j)->get_symbol_name() << endl;
                cout << (*j)->get_symbol_type() << endl;
                cout << (*j)->get_pass_type() << endl;
            }
            // print internal list...
            this->print_internal(callable_obj->get_child());
        } else {
            // print out the data part
            cout << (*i)->get_symbol_name() << endl;
            cout << (*i)->get_symbol_type() << endl;
        }
    }
}

SymbolIterator SymTable::position() {
    return this->table_iter;
}

SymType Symbol::get_symbol_type() {
    return this->symbol_type;
}

string Symbol::get_symbol_name() {
    return this->symbol_name;
}

Scope Symbol::get_symbol_scope() {
    return this->symbol_scope;
}

unsigned int Symbol::get_nesting_level() {
    return this->nesting_level;
}

SymbolIterator SymCallable::return_sub_iterator() {
    return this->child->begin();
}

SymbolListPtr SymCallable::get_parent() {
    return this->parent;
}

SymbolListPtr SymCallable::get_child() {
    return this->child;
}

VarType SymCallable::get_return_type() {
    return this->return_type;
}

ArgumentListPtr SymCallable::get_argument_list() {
    return this->argument_list;
}

PassType SymArgument::get_pass_type() {
    return this->pass_type;
}
