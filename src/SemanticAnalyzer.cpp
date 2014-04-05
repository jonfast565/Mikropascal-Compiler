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

// Semantic analyzer stuff
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
    string label = "L" + conv_string(this->label_count) + ":";
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

bool CodeBlock::is_operator(SymbolPtr character) {
    if (character->get_symbol_type() == SYM_CONSTANT) {
        VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
        if (op == ARITH_OPERATOR
            || op == RELAT_OPERATOR
            || op == COMP_OPERATOR) {
            return true;
        }
    }
    return false;
}

bool CodeBlock::is_operand(SymbolPtr character) {
    if (character->get_symbol_type() == SYM_CONSTANT) {
        VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
        if (op != ARITH_OPERATOR
            && op != RELAT_OPERATOR
            && op != COMP_OPERATOR
            && op != LPAREN
            && op != RPAREN) {
            return true;
        } else {
            return false;
        }
    } else if (character->get_symbol_type() == SYM_DATA) {
        return true;
    } else {
        return false;
    }
}

bool CodeBlock::is_lparen(SymbolPtr character) {
    VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
    if (op == LPAREN) {
        return true;
    } else {
        return false;
    }
}

bool CodeBlock::is_rparen(SymbolPtr character) {
    VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
    if (op == RPAREN) {
        return true;
    } else {
        return false;
    }
}

int CodeBlock::compare_ops(SymbolPtr c1, SymbolPtr c2) {
    if (op_precendence(c1) > op_precendence(c2)) {
        return 1;
    } else if (op_precendence(c1) == op_precendence(c2)) {
        return 0;
    } else {
        return -1;
    }
}

int CodeBlock::op_precendence(SymbolPtr c1) {
    if (c1 ->get_symbol_type() == SYM_CONSTANT) {
        VarType op = static_pointer_cast<SymConstant>(c1)->get_constant_type();
        if (op == LPAREN
            || op == RPAREN) {
            return 5;
        } else if (op == ARITH_OPERATOR) {
            if (static_pointer_cast<SymConstant>(c1)->get_data() == "*"
                || static_pointer_cast<SymConstant>(c1)->get_data() == "/") {
                return 4;
            } else {
                return 3;
            }
        } else if (op == RELAT_OPERATOR) {
            return 2;
        } else if (op == COMP_OPERATOR) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}

void CodeBlock::make_cast(VarType v1, VarType v2) {
    if (v1 != v2) {
        if ((v1 == INTEGER && v2 == FLOATING)
            || (v1 == FLOATING && v2 == INTEGER)) {
            if (v1 == INTEGER) {
                // cast v2 to v1's type
                write_raw("CASTSF");
            } else {
                // cast v2 to v1's type (float)
                write_raw("CASTSI");
            }
        } else if ((v1 == STRING && (v2 == INTEGER || v2 == FLOATING))
                   || ((v1 == INTEGER || v1 == FLOATING) && v2 == STRING)) {
            this->valid = false;
            report_msg_type("Semantic Error", "Unable to cast operand");
        }
    }
}

// Code Block Stuff
void CodeBlock::append(CodeBlockPtr block) {
    this->block_list->push_back(block);
}

BlockType CodeBlock::get_block_type() {
    return this->block_type;
}

bool CodeBlock::check_filter_size(SymbolListPtr filtered) {
    if (filtered->size() > 1
        || filtered->size() == 0) {
        // too many ids found
        this->valid = false;
        report_msg_type("Semantic Error", "Invalid filter size");
        return false;
    } else {
        // correct
        return true;
    }
}

unsigned int CodeBlock::get_nesting_level() {
    CodeBlockPtr parent_block = this->get_parent();
    unsigned int level_found = 0;
    while(parent_block != nullptr) {
        // if inside a function or procedure body
        if (parent_block->get_block_type() == ACTIVATION_BLOCK) {
            level_found++;
        }
        parent_block = parent_block->get_parent();
    }
    return level_found;
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
    write_raw("MOV SP D0");
    // push begin symbols
    for (auto i = temp_symbols->begin(); i != temp_symbols->end(); i++) {
        if (static_pointer_cast<SymData>(*i)->get_var_type() == STRING) {
            write_raw("PUSH #\"\"");
        } else if (static_pointer_cast<SymData>(*i)->get_var_type() == FLOATING) {
            write_raw("PUSH #0.0");
        } else {
            write_raw("PUSH #0");
        }
    }
    write_raw("");
}

void ProgramBlock::generate_post() {
    // generate program exit point
    write_raw("HLT\n");
}

bool ProgramBlock::validate() {
    // do nothing... it's the program beginning
    return this->valid;
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
    VarType old_type = VOID;
    //generate assignment code straight up
    for (auto i = this->temp_symbols->begin();
         i != this->temp_symbols->end(); i++) {
        if ((*i)->get_symbol_type() == SYM_DATA) {
            SymDataPtr d = static_pointer_cast<SymData>(*i);
            if (i == this->temp_symbols->begin()) {
                old_type = d->get_var_type();
            }
            write_raw("PUSH " + d->get_address());
            // make_cast(old_type, d->get_var_type());
            old_type = static_pointer_cast<SymData>(*i)->get_var_type();
        } else {
            SymConstantPtr c = static_pointer_cast<SymConstant>(*i);
            if (i == this->temp_symbols->begin()) {
                old_type = c->get_constant_type();
            }
            if (c->get_constant_type() == BOOL_VALUE) {
                // not implemented
            } else if (c->get_constant_type() == FLOATING_LITERAL) {
                write_raw("PUSH #" + c->get_data());
                // make_cast(old_type, c->get_constant_type());
                old_type = FLOATING;
            } else if (c->get_constant_type() == INTEGER_LITERAL) {
                write_raw("PUSH #" + c->get_data());
                // make_cast(old_type, c->get_constant_type());
                old_type = INTEGER;
            } else if (c->get_constant_type() == STRING_LITERAL) {
                // remove single quotes, and replace with double
                string string_const = c->get_data();
                replace(string_const.begin(), string_const.end(), '\'', '"');
                write_raw("PUSH #" + string_const);
                // make_cast(old_type, c->get_constant_type());
                old_type = STRING;
            } else if (c->get_constant_type() == ARITH_OPERATOR) {
                if (c->get_data().compare("+") == 0) {
                    write_raw("ADDS");
                } else if (c->get_data().compare("-") == 0) {
                    write_raw("SUBS");
                } else if (c->get_data().compare("*") == 0) {
                    write_raw("MULS");
                } else if (c->get_data().compare("/") == 0) {
                    write_raw("DIVS");
                }
            }
        }
    }
}

void AssignmentBlock::generate_post() {
    // pop into assigner
    write_raw("POP " + static_pointer_cast<SymData>(this->assigner)->get_address());
    write_raw("");
}

bool AssignmentBlock::validate() {
    // do nothing...
    return this->valid;
}

void AssignmentBlock::preprocess() {
    bool first_id = true;
    for (auto i = this->unprocessed->begin();
         i != this->unprocessed->end(); i++) {
        if ((*i)->get_token() == MP_ID) {
            SymbolListPtr filtered_data = this->get_analyzer()->get_symtable()->data_in_scope_at((*i)->get_lexeme(), this->get_nesting_level());
            if (this->check_filter_size(filtered_data)) {
                if (first_id == true) {
                    // just right
                    this->assigner = static_pointer_cast<SymData>(*filtered_data->begin());
                    first_id = false;
                } else {
                    // just right
                    this->temp_symbols->push_back(static_pointer_cast<SymData>(*filtered_data->begin()));
                }
            } else {
                // filter size incorrect
                this->valid = false;
            }
        } else if ((*i)->get_token() == MP_INT_LITERAL) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), INTEGER_LITERAL)));
        } else if ((*i)->get_token() == MP_STRING_LITERAL) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), STRING_LITERAL)));
        } else if ((*i)->get_token() == MP_FLOAT_LITERAL) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), FLOATING_LITERAL)));
        } else if ((*i)->get_token() == MP_TRUE
                   || (*i)->get_token() == MP_FALSE) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), BOOL_VALUE)));
        } else if ((*i)->get_token() == MP_LEFT_PAREN) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), LPAREN)));
        } else if ((*i)->get_token() == MP_RIGHT_PAREN) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), RPAREN)));
        } else if ((*i)->get_token() == MP_PLUS
                   || (*i)->get_token() == MP_MINUS
                   || (*i)->get_token() == MP_MULT
                   || (*i)->get_token() == MP_DIV
                   || (*i)->get_token() == MP_DIV_KW
                   || (*i)->get_token() == MP_MOD_KW) {
            this->temp_symbols->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), ARITH_OPERATOR)));
        } else if ((*i)->get_token() == MP_ASSIGNMENT) {
            // skip
        } else {
            report_msg_type("Semantic Error", "Symbol type is not valid in assignment");
            this->valid = false;
        }
    }
    // convert to postifx
    convert_postfix();
}

void AssignmentBlock::convert_postfix() {
    // data structs
    shared_ptr<stack<SymbolPtr>> op_stack = shared_ptr<stack<SymbolPtr>>(new stack<SymbolPtr>);
    SymbolListPtr unpostfix_symbols = SymbolListPtr(new SymbolList());
    
    // go through the infix symbols
    for (auto i = this->temp_symbols->begin(); i !=
         this->temp_symbols->end(); i++) {
        if (is_operand(*i)) {
            unpostfix_symbols->push_back(*i);
        } else if (is_operator(*i)) {
            while (!op_stack->empty()
                   && !is_lparen(op_stack->top())
                   && compare_ops(*i, op_stack->top()) <= 0) {
                unpostfix_symbols->push_back(op_stack->top());
                op_stack->pop();
            }
            op_stack->push(*i);
        } else if (is_lparen(*i)) {
            op_stack->push(*i);
        } else if (is_rparen(*i)) {
            while (!op_stack->empty()) {
                if (is_lparen(op_stack->top())) {
                    op_stack->pop();
                    break;
                }
                unpostfix_symbols->push_back(op_stack->top());
                op_stack->pop();
            }
        }
    }
    
    while(!op_stack->empty()) {
        unpostfix_symbols->push_back(op_stack->top());
        op_stack->pop();
    }
    
    // set the temp symbols as the postfix stuff
    this->temp_symbols = unpostfix_symbols;
}

void AssignmentBlock::catch_token(TokenPtr symbol) {
    this->unprocessed->push_back(symbol);
}

// IO Block stuff
void IOBlock::generate_pre() {
    for (auto i = this->args->begin(); i !=
         this->args->end(); i++) {
        if ((*i)->get_symbol_type() != SYM_CONSTANT) {
            // get the address of the data to be read/written
            string addr = static_pointer_cast<SymData>(*i)->get_address();
            // for a read action we will need to know the type
            if (this->action == IO_READ) {
                VarType data_type = static_pointer_cast<SymData>(*i)->get_var_type();
                // can't read a boolean
                if (data_type == BOOLEAN) {
                    report_msg_type("Semantic Error", "Boolean cannot be read by VM.");
                } else if (data_type == INTEGER) {
                    write_raw("RD " + addr);
                } else if (data_type == FLOATING) {
                    write_raw("RDF " + addr);
                } else {
                    // assume string
                    write_raw("RDS " + addr);
                }
                // assume print a line terminator if readln or something
                if (this->line_terminator) {
                    write_raw("PUSH #\"\n\"");
                    write_raw("WRT");
                }
            } else {
                // no type necessary, just push and write
                write_raw("PUSH " + addr);
                // line terminator determines write call
                if (this->line_terminator) {
                    write_raw("WRTLNS");
                } else {
                    write_raw("WRTS");
                }
            }
        } else {
            // is a constant
            if (this->action == IO_READ) {
                report_msg_type("Semantic Error", "Constant value cannot be read to variable.");
            } else {
                SymConstantPtr constant = static_pointer_cast<SymConstant>(*i);
                if (constant->get_constant_type() == STRING_LITERAL) {
                    // remove single quotes, and replace with double
                    string string_const = constant->get_data();
                    replace(string_const.begin(), string_const.end(), '\'', '"');
                    write_raw("PUSH #" + string_const);
                    if (this->line_terminator) {
                        write_raw("WRTLNS");
                    } else {
                        write_raw("WRTS");
                    }
                } else {
                    // generate numeric stuff
                    write_raw("PUSH #" + constant->get_data());
                    if (this->line_terminator) {
                        write_raw("WRTLNS");
                    } else {
                        write_raw("WRTS");
                    }
                }
            }
        }
    }
}

void IOBlock::generate_post() {
    // generate nothing (no nesting)
    write_raw("");
}

bool IOBlock::validate() {
    // do nothing...
    return this->valid;
}

void IOBlock::catch_token(TokenPtr token) {
    // filter by printable items
    if (token->get_token() == MP_ID
        || token->get_token() == MP_INT_LITERAL
        || token->get_token() == MP_STRING_LITERAL
        || token->get_token() == MP_FLOAT_LITERAL) {
        this->unprocessed->push_back(token);
    }
}

void IOBlock::preprocess() {
    for (auto i = this->unprocessed->begin();
         i != this->unprocessed->end(); i++) {
        // assume ids, get lexemes and determine scoping
        if ((*i)->get_token() == MP_ID) {
            SymbolListPtr filtered_data = this->get_analyzer()->get_symtable()->data_in_scope_at((*i)->get_lexeme(), this->get_nesting_level());
            if (filtered_data->size() == 1) {
                // just right
                this->args->push_back(static_pointer_cast<SymData>(*filtered_data->begin()));
            } else if (filtered_data->size() > 1) {
                // too many ids found
                this->valid = false;
            } else {
                // no ids found
                this->valid = false;
            }
        } else if ((*i)->get_token() == MP_STRING_LITERAL) {
            this->args->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), STRING_LITERAL)));
        } else if ((*i)->get_token() == MP_INT_LITERAL) {
            this->args->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), INTEGER_LITERAL)));
        } else {
            // assume float literal
            this->args->push_back(SymbolPtr(new SymConstant((*i)->get_lexeme(), FLOATING_LITERAL)));
        }
    }
}

