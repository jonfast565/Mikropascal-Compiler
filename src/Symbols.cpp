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
            this->offset_scope->push(this->max_offset);
            this->max_offset = 0;
        }
    }
}

void SymTable::return_from() {
    if (this->last_callable != nullptr) {
        this->table_iter = this->last_callable->get_parent()->begin();
        this->nesting_level--;
        this->to_latest();
        this->last_callable = nullptr;
        this->max_offset = this->offset_scope->top();
        this->offset_scope->pop();
    }
}

SymbolIterator SymTable::get_first() {
    return this->symbol_list->begin();
}

SymbolIterator SymTable::get_last() {
    return this->symbol_list->end();
}

shared_ptr<vector<SymbolPtr>> SymTable::find(string id) {
    // get a vector of symbols to return
    shared_ptr<vector<SymbolPtr>> sym_ptrs =
    shared_ptr<vector<SymbolPtr>>(new vector<SymbolPtr>);
    
    // iterator stack for moving within contexts
    shared_ptr<stack<SymbolIterator>> sym_stack =
    shared_ptr<stack<SymbolIterator>>(new stack<SymbolIterator>);
    
    // global table last pointer
    SymbolIterator glbl_pos = this->get_first();
    SymbolIterator glbl_last = this->get_last();
    
    // wait until we're at the end of the table
    while (true) {
        // check to see if we're at the end of the list
        if (glbl_pos == glbl_last) {
            // if stack empty
            if (sym_stack->empty()) {
                // no other context to go back to
                break;
            } else {
                // pop and set top to previous
                glbl_last = sym_stack->top();
                sym_stack->pop();
                glbl_pos = sym_stack->top();
                glbl_pos++;
                sym_stack->pop();
            }
        } else {
            // get a callable obj
            if ((*glbl_pos)->get_symbol_type() == SYM_CALLABLE) {
                
                // look at the function name
                SymCallablePtr callable_obj = static_pointer_cast<SymCallable>(*glbl_pos);
                if (callable_obj->get_symbol_name().compare(id) == 0) {
                    sym_ptrs->push_back(callable_obj);
                }
                
                // look at the argument names
                if (callable_obj->get_argument_list()->size() > 0) {
                    for (auto i = callable_obj->get_argument_list()->begin();
                         i != callable_obj->get_argument_list()->end(); i++) {
                        // push if same as id
                        if ((*i)->get_symbol_name().compare(id) == 0) {
                            sym_ptrs->push_back(*i);
                        }
                    }
                }
                
                // get the new start, end for the inner table
                sym_stack->push(glbl_pos);
                sym_stack->push(glbl_last);
                glbl_pos = callable_obj->return_sub_iterator();
                glbl_last = callable_obj->return_sub_end_iterator();
                
            } else {
                // glbl_pos == SYM_DATA, not callable
                if ((*glbl_pos)->get_symbol_name().compare(id) == 0) {
                    sym_ptrs->push_back(*glbl_pos);
                }
                
                // move the iterator forward
                glbl_pos++;
            }
        }
    }
    return sym_ptrs;
}

void SymTable::create_data(string name, VarType type) {
    Scope current_scope;
    if (nesting_level == 0) {
        current_scope = GLOBAL;
    } else {
        current_scope = LOCAL;
    }
    SymDataPtr p = SymDataPtr(
    new SymData(name, type, current_scope, this->get_level(), last_callable));
    p->set_address(this->get_level(), this->get_offset());
    this->symbol_list->push_back(p);
    this->max_offset++;
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

ArgumentPtr SymTable::create_argument(string name, VarType type, PassType pass) {
	Scope current_scope;
	if (this->nesting_level == 0) {
		current_scope = GLOBAL;
	} else {
		current_scope = LOCAL;
	}
	return ArgumentPtr(new SymArgument(name, type, current_scope, this->get_level(), pass, last_callable));
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
            report_msg_type("Routine", callable_obj->get_symbol_name() + ", " +
                            sym_type_to_string(callable_obj->get_symbol_type()) + ", " +
                            var_type_to_string(callable_obj->get_return_type()) + ", " +
                            conv_string(callable_obj->get_nesting_level()));
            // (void if procedure)
            // print its arguments
            for (auto j = callable_obj->get_argument_list()->begin();
                 j != callable_obj->get_argument_list()->end(); j++) {
                SymArgumentPtr arg_obj = static_pointer_cast<SymArgument>(*j);
                report_msg_type("Argument", arg_obj->get_symbol_name() + ", " +
                                sym_type_to_string(arg_obj->get_symbol_type()) + ", " +
                                var_type_to_string(arg_obj->get_var_type()) + ", " +
                                pass_type_to_string(arg_obj->get_pass_type()));
            }
            // print internal list...
            this->print_internal(callable_obj->get_child());
        } else {
            SymDataPtr data_obj = static_pointer_cast<SymData>(*i);
            // print out the data part
            report_msg_type("Data", data_obj->get_symbol_name() + ", " +
                            sym_type_to_string(data_obj->get_symbol_type()) + ", " +
                            var_type_to_string(data_obj->get_var_type()) + ", " +
                            conv_string(data_obj->get_nesting_level()) + ", " +
                            conv_string(data_obj->get_address()));
        }
    }
}

unsigned int SymTable::get_level() {
    return this->nesting_level;
}

unsigned int SymTable::get_offset() {
    return this->max_offset;
}

SymbolIterator SymTable::position() {
    return this->table_iter;
}

SymbolListPtr SymTable::filter_data(SymbolListPtr filterable) {
    SymbolListPtr filtered = SymbolListPtr(new SymbolList());
    for (auto i = filterable->begin(); i != filterable->end(); i++) {
        if ((*i)->get_symbol_type() == SYM_DATA) {
            filtered->push_back(*i);
        }
    }
    return filtered;
}

SymbolListPtr SymTable::filter_callable(SymbolListPtr filterable) {
    SymbolListPtr filtered = SymbolListPtr(new SymbolList());
    for (auto i = filterable->begin(); i != filterable->end(); i++) {
        if ((*i)->get_symbol_type() == SYM_CALLABLE) {
            filtered->push_back(*i);
        }
    }
    return filtered;
}

SymbolListPtr SymTable::get_global_vars() {
    SymbolListPtr filtered = SymbolListPtr(new SymbolList());
    for (auto i = this->get_first(); i != this->get_last(); i++) {
        if ((*i)->get_symbol_type() == SYM_DATA &&
            (*i)->get_symbol_scope() == GLOBAL) {
            filtered->push_back(*i);
        }
    }
    return filtered;
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

SymbolIterator SymCallable::return_sub_end_iterator() {
    return this->child->end();
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

unsigned int SymCallable::get_number_arguments() {
    unsigned int num_arguments = 0;
    for (auto i = this->argument_list->begin();
         i != this->argument_list->end(); i++) {
        num_arguments++;
    }
    return num_arguments;
}

shared_ptr<vector<VarType>> SymCallable::get_argument_types() {
    shared_ptr<vector<VarType>> argument_types =
    shared_ptr<vector<VarType>>(new vector<VarType>);
    for (auto i = this->argument_list->begin();
         i != this->argument_list->end(); i++) {
        argument_types->push_back((*i)->get_var_type());
    }
    return argument_types;
}

PassType SymArgument::get_pass_type() {
    return this->pass_type;
}

VarType SymData::get_var_type() {
    return this->variable_type;
}

SymCallablePtr SymData::get_parent_callable() {
    return this->parent_callable;
}

void SymData::set_address(unsigned int level, unsigned int offset) {
    this->address = conv_string(offset) + "(D" + conv_string(level) + ")";
}

string  SymData::get_address() {
    return this->address;
}
