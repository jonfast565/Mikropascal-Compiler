/*
 * SemanticAnalyzer.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#include "SemanticAnalyzer.hpp"

// AST Stuff
AbstractTree::AbstractTree() {
	this->root_node = AbstractNodePtr(new AbstractNode());
	this->iterable = this->root_node;
}

AbstractTree::AbstractTree(AbstractNodePtr root) {
	this->root_node = root;
	this->iterable = this->root_node;
}

void AbstractTree::add_move_child(AbstractNodePtr child_node) {
	this->iterable->add_child_node(child_node);
	child_node->set_parent(this->iterable);
	this->iterable = child_node;
}

void AbstractTree::goto_parent() {
	if (this->iterable->get_parent() != nullptr) {
		this->iterable = this->iterable->get_parent();
	}
}

void AbstractTree::push_children(AbstractNodePtr current_node, AbstractStackPtr current_symbols) {
    AbstractStackPtr reversal = AbstractStackPtr(new AbstractNodeStack());
    for (auto i = current_node->get_child_begin();
         i != current_node->get_child_end(); i++) {
        reversal->push(*i);
    }
    while(!reversal->empty()) {
        current_symbols->push(reversal->top());
        reversal->pop();
    }
}

AbstractNodePtr AbstractTree::get_root_node() {
    return this->iterable;
}

AbstractNodePtr AbstractTree::get_current_parent() {
	if (this->iterable->get_parent() != nullptr) {
		return this->iterable->get_parent();
	}
	return nullptr;
}

void AbstractTree::display_tree() {
    // display tree at the root, which is where it should be
    report_msg("AST Printable Tree");
    AbstractStackPtr loop = AbstractStackPtr(new AbstractNodeStack());
    loop->push(this->iterable);
    while (!loop->empty()) {
        AbstractNodePtr current = loop->top();
        loop->pop();
        if (current->get_is_rule()) {
            report_msg_type("AST Rule",
                            get_rule_info(current->get_parse_type()));
            this->push_children(current, loop);
        } else if (current->get_is_epsilon()) {
            report_msg("AST Epsilon");
        } else {
            report_msg_type("AST Token",
                            get_token_info(current->get_token()->get_token()).first
                            + ": " + current->get_token()->get_lexeme());
        }
    }
}

AbstractNode::AbstractNode() {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = ROOT;
	this->is_root = true;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(ParseType parse_type) {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = parse_type;
	this->is_root = false;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(AbstractNodePtr parent_node, ParseType parse_type) {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = parse_type;
	this->is_root = false;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(shared_ptr<Token> token) {
    this->child_nodes = nullptr;
    this->parse_type = LITERAL;
    this->is_root = false;
    this->is_rule = false;
    this->token = token;
}

void AbstractNode::add_child_node(AbstractNodePtr child_node) {
	this->child_nodes->push_back(child_node);
}

void AbstractNode::set_is_root(bool is_root) {
	this->is_root = is_root;
}

bool AbstractNode::get_is_root() {
    return this->is_root;
}

bool AbstractNode::get_is_rule() {
    return this->is_rule;
}

bool AbstractNode::get_is_epsilon() {
    return (this->parse_type == EPSILON);
}

ParseType AbstractNode::get_parse_type() {
	if (this->is_rule != false)
		return this->parse_type;
	else
		return NO_RULE;
}

TokenPtr AbstractNode::get_token() {
    if (is_rule) {
        return nullptr;
    } else {
        return this->token;
    }
}

void AbstractNode::set_parent(AbstractNodePtr parent_node) {
	this->parent_node = parent_node;
}

AbstractNodePtr AbstractNode::get_parent() {
	if (this->parent_node == nullptr) {
		return nullptr;
	} else {
		return this->parent_node;
	}
}

AbstractNodeList::iterator AbstractNode::get_child_begin() {
    return this->child_nodes->begin();
}

AbstractNodeList::iterator AbstractNode::get_child_end() {
    return this->child_nodes->end();
}

// semantic analyzer stuff
SemanticAnalyzer::SemanticAnalyzer() {
    this->ast = AbstractTreePtr(new AbstractTree());
    this->symbols = SymTablePtr(new SymTable());
    this->condensedst = CodeBlockPtr(new ProgramBlock());
    this->condensedst->set_analyzer(SemanticAnalyzerPtr(this));
    this->label_count = 0;
    this->block_stack = shared_ptr<stack<CodeBlockPtr>>(new stack<CodeBlockPtr>);
    this->block_stack->push(this->condensedst);
}

AbstractTreePtr SemanticAnalyzer::get_ast() {
    return this->ast;
}

SymTablePtr SemanticAnalyzer::get_symtable() {
    return this->symbols;
}

string SemanticAnalyzer::generate_label() {
    string label = "L" + conv_string(this->label_count);
    label_count++;
    return label;
}

bool SemanticAnalyzer::is_scoped_any(string id) {
    // get all symbols
    SymbolListPtr resolved_sym = this->get_symtable()->find(id);
    
    // if resolved < 0, then not defined in program
    if (resolved_sym->size() == 0) {
        report_msg_type("Semantic Error", "No such item named '" + id + "'");
        return false;
    } else {
        return true;
    }
}

bool SemanticAnalyzer::is_callable_scoped(string callable_id) {
    // resolve and filter
    SymbolListPtr resolved_sym = this->get_symtable()->find(callable_id);
    SymbolListPtr callables = SymTable::filter_callable(resolved_sym);
    
    // check for something in the list
    if (callables->size() > 0) {
        return true;
    } else {
        report_msg_type("Semantic Error", "'" + callable_id + "' proc/func definition is not scoped in program");
        return false;
    }
}

bool SemanticAnalyzer::is_data_scoped(string data_id) {
    // resolve and filter
    SymbolListPtr resolved_sym = this->get_symtable()->find(data_id);
    SymbolListPtr data = SymTable::filter_data(resolved_sym);
    
    // check for something in the list
    if (data->size() > 0) {
        return true;
    } else {
        report_msg_type("Semantic Error", "'" + data_id + "' data definition is not scoped in program");
        return false;
    }
}

void SemanticAnalyzer::generate_all() {
    // generate starting at the top
    generate_one(this->condensedst);
}

void SemanticAnalyzer::generate_one(CodeBlockPtr current) {
    // iterate through the blocks and
    // generate all code
    // generate pre inner code
    current->preprocess();
    if (current->validate()) {
        current->generate_pre();
        // get children and visit
        for (auto i = current->inner_begin(); i != current->inner_end(); i++) {
            generate_one(*i);
        }
        // generate post code
        current->generate_post();
    } else {
        // terminate the program
        report_msg_type("Semantic Error", "Block validation failed.");
        exit(0);
    }
}

void SemanticAnalyzer::append_block(CodeBlockPtr new_block) {
    if (new_block != nullptr) {
        // child of top block stack parent
        this->block_stack->top()->append(new_block);
        // parent is the top of the block stack
        new_block->set_parent(this->block_stack->top());
        // push this block to the stack top
        this->block_stack->push(new_block);
        // set this block's analyzer parent if null
        new_block->set_analyzer(SemanticAnalyzerPtr(this));
    }
}

void SemanticAnalyzer::rappel_block() {
    // pop the block stack back to an earlier block
    this->block_stack->pop();
}

CodeBlockPtr SemanticAnalyzer::get_top_block() {
    // get the block at the top of the stack
    return this->block_stack->top();
}

void SemanticAnalyzer::feed_token(TokenPtr token) {
    this->block_stack->top()->catch_token(token);
}

bool SemanticAnalyzer::is_data_in_callable(string data_id, string callable_id) {
    // get all symbols and filter them
    SymbolListPtr resolved_sym = this->get_symtable()->find(data_id);
    SymbolListPtr resolved_callable_sym = this->get_symtable()->find(callable_id);
    
    // merge data and callables
    for (auto i = resolved_callable_sym->begin(); i != resolved_callable_sym->end(); i++) {
        resolved_sym->push_back(*i);
    }
    
    // filter
    SymbolListPtr data = SymTable::filter_data(resolved_sym);
    SymbolListPtr callables = SymTable::filter_callable(resolved_sym);
    
    // new lists for matches
    SymbolListPtr matching_id = SymbolListPtr(new SymbolList);
    SymbolListPtr matching_callable = SymbolListPtr(new SymbolList);
    
    // get data by id
    for (auto i = data->begin(); i != data->end(); i++) {
        if ((*i)->get_symbol_name().compare(data_id) == 0) {
            matching_id->push_back(*i);
        }
    }
    
    // get callable by id
    for (auto i = callables->begin(); i != callables->end(); i++) {
        if ((*i)->get_symbol_name().compare(callable_id) == 0) {
            matching_callable->push_back(*i);
        }
    }
    
    // check for duplicates
    if (matching_callable->size() < 1) {
        return false;
    } else if (matching_callable->size() > 1) {
        // nesting level scope check needed? (error is primitive)
        report_msg_type("Semantic Error", "Redefinition of " + callable_id + " in multiple places");
        // isn't scoped because we don't know what it is
        return false;
    } else {
        // go through all matching ids and get their callable parents
        for (auto i = matching_id->begin(); i != matching_id->end(); i++) {
            // get the data object and its parent callable
            SymDataPtr data_obj = static_pointer_cast<SymData>(*i);
            SymCallablePtr owner_callable = data_obj->get_parent_callable();
            // ensure the data item is not global
            if (owner_callable != nullptr) {
                // get the first callable and check for name similarity
                for (auto i = callables->begin(); i != callables->end(); i++) {
                    if ((*i)->get_symbol_name().compare(owner_callable->get_symbol_name()) == 0) {
                        // it's there
                        return true;
                    }
                }
            }
        }
    }
    // no definition found
    report_msg_type("Semantic Error", "No definition of " + data_id + " in " + callable_id);
    return false;
}

void SemanticAnalyzer::print_symbols() {
    this->symbols->print();
}

// Code Block Stuff
void CodeBlock::append(CodeBlockPtr block) {
    this->block_list->push_back(block);
}

void CodeBlock::set_analyzer(SemanticAnalyzerPtr analyzer) {
    this->parent_analyzer = analyzer;
}

SemanticAnalyzerPtr CodeBlock::get_analyzer() {
    return this->parent_analyzer;
}

void CodeBlock::generate_pre() {
    // generate no code (virtual)
}

void CodeBlock::generate_post() {
    // generate no code (virtual)
}

void CodeBlock::catch_token(TokenPtr symbol) {
    this->unprocessed->push_back(symbol);
}

void CodeBlock::set_parent(CodeBlockPtr parent) {
    this->parent_block = parent;
}

CodeBlockPtr CodeBlock::get_parent() {
    return this->parent_block;
}

bool CodeBlock::validate() {
    return true;
}

void CodeBlock::preprocess() {
    // do nothing
}

CodeBlockList::iterator CodeBlock::inner_begin() {
    return this->block_list->begin();
}

CodeBlockList::iterator CodeBlock::inner_end() {
    return this->block_list->end();
}

// Program Block stuff
void ProgramBlock::generate_pre() {
    // generate program entry point
    write_raw("PUSH SP D0\n");
    // push begin symbols
    for (auto i = temp_symbols->begin(); i != temp_symbols->end(); i++) {
        if (static_pointer_cast<SymData>(*i)->get_var_type() == STRING) {
            write_raw("PUSH #\"\"");
        } else if (static_pointer_cast<SymData>(*i)->get_var_type() == FLOATING) {
            write_raw("PUSH #0.0");
        } else {
            write_raw("PUSH 0");
        }
    }
    write_raw("\n");
}

void ProgramBlock::generate_post() {
    // generate program exit point
    write_raw("HLT\n");
}

bool ProgramBlock::validate() {
    // do nothing... it's the program beginning
    return true;
}

void ProgramBlock::preprocess() {
    // get symbols of global vars
    SymbolListPtr global_vars = this->get_analyzer()->get_symtable()->get_global_vars();
    // copy into local table
    for (auto i = global_vars->begin(); i != global_vars->end(); i++) {
        this->temp_symbols->push_back(*i);
    }
}

// Assignment Block stuff
void AssignmentBlock::generate_pre() {

}

void AssignmentBlock::generate_post() {
    // generate program exit point
}

bool AssignmentBlock::validate() {
    // do nothing...
    return true;
}

void AssignmentBlock::preprocess() {
    // nothing
}

// IO Block stuff
void IOBlock::generate_pre() {
    // generate program entry point
}

void IOBlock::generate_post() {
    // generate program exit point
}

bool IOBlock::validate() {
    // do nothing...
    return true;
}

void IOBlock::preprocess() {
    
}

