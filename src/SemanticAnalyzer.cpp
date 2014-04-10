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
    report_msg("Printable AST: ");
    AbstractStackPtr loop = AbstractStackPtr(new AbstractNodeStack());
    loop->push(this->iterable);
    while (!loop->empty()) {
        AbstractNodePtr current = loop->top();
        loop->pop();
        if (current->get_is_rule() && !current->get_is_rule()) {
            report_msg_type("AST Rule",
                            get_rule_info(current->get_parse_type()));
            this->push_children(current, loop);
        } else if (current->get_is_epsilon()) {
            report_msg("AST Epsilon");
        } else {
            report_msg_type("AST Match",
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

AbstractNode::AbstractNode(TokenPtr token) {
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

void AbstractNode::set_parent(AbstractNodePtr new_parent_node) {
	this->parent_node = new_parent_node;
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
    this->block_stack = unique_ptr<stack<CodeBlockPtr>>(new stack<CodeBlockPtr>);
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
    CodeBlockPtr top = this->condensedst;
    generate_one(top);
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
                for (auto j = callables->begin(); j != callables->end(); j++) {
                    if ((*j)->get_symbol_name().compare(owner_callable->get_symbol_name()) == 0) {
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
        if (OPERATOR(op)()) {
            return true;
        }
    }
    return false;
}

bool CodeBlock::is_operand(SymbolPtr character) {
    if (character->get_symbol_type() == SYM_CONSTANT) {
        VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
        if (!OPERATOR(op)()
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
            return 4;
        } else if (op == NOT) {
            return 3;
        } else if (op == DIV || op == MUL
                   || op == MOD || op == AND) {
            return 2;
        } else if (op == ADD || op == SUB
                   || op == OR) {
            return 1;
        } else if (op == IEQ || op == INE
                   || op == ILT || op == ILE
                   || op == IGT || op == IGE) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

VarType CodeBlock::make_cast(VarType v1, VarType v2) {
    if (v1 != v2) {
        if (v1 == INTEGER && v2 == FLOATING) {
            // cast back to integer
            write_raw("CASTSI");
            return INTEGER;
        } else if (v1 == FLOATING && v2 == INTEGER) {
            // cast back to floating
            write_raw("CASTSF");
            return FLOATING;
        } else if ((v1 == STRING && (v2 == INTEGER || v2 == FLOATING))
                   || ((v1 == INTEGER || v1 == FLOATING) && v2 == STRING)
                   || (v1 == VOID || v2 == VOID)) {
            this->valid = false;
            report_msg_type("Semantic Error", "Unable to cast "
                            + var_type_to_string(v1)
                            + " to " + var_type_to_string(v2));
            return VOID;
        } else {
            // fine
            return v2;
        }
    } else {
        return v2;
    }
    return VOID;
}

// Code Block Stuff
void CodeBlock::append(CodeBlockPtr block) {
    this->block_list->push_back(block);
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

/* Buggy (will try to figure out why eventually) */
unsigned int CodeBlock::get_nesting_level() {
	// don't link a shared_ptr to THIS!!!!!!!!!
    CodeBlockPtr temp_parent_block = this->get_parent();
    unsigned int level_found = 0;
    // naive code, will have to fix this decl later...
    if (this->block_type == ACTIVATION_BLOCK) {
    	level_found++;
    }
    while(temp_parent_block != nullptr) {
        // if inside a function or procedure body
        if (temp_parent_block->get_block_type() == ACTIVATION_BLOCK) {
            level_found++;
        }
        temp_parent_block = parent_block->get_parent();
    }
    return level_found;
}

void CodeBlock::set_analyzer(SemanticAnalyzerPtr analyzer) {
    this->parent_analyzer = analyzer;
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

void CodeBlock::convert_postfix() {
    // data structs
    unique_ptr<stack<SymbolPtr>> op_stack = unique_ptr<stack<SymbolPtr>>(new stack<SymbolPtr>);
    SymbolListPtr unpostfix_symbols = SymbolListPtr(new SymbolList());
    
    // shunting yard
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

// generates an expression
// returns its last type
VarType CodeBlock::generate_expr(SymbolListPtr expr_list) {
    // generate expr (get first operand type)
    VarType expr_type = static_pointer_cast<SymData>(*expr_list->begin())->get_var_type();
    for (auto i = expr_list->begin();
         i != expr_list->end(); i++) {
        if (expr_type == VOID) {
            // error condition
            this->valid = false;
            break;
        }
        if ((*i)->get_symbol_type() == SYM_DATA) {
            SymDataPtr d = static_pointer_cast<SymData>(*i);
            write_raw("PUSH " + d->get_address());
            expr_type = make_cast(expr_type, d->get_var_type());
        } else {
            SymConstantPtr c = static_pointer_cast<SymConstant>(*i);
            if (i == this->temp_symbols->begin()) {
                expr_type = c->get_constant_type();
            }
            if (c->get_constant_type() == BOOLEAN_LITERAL_T) {
                // integer alias
                write_raw("PUSH #1");
            } else if (c->get_constant_type() == BOOLEAN_LITERAL_F) {
                // integer alias not
                write_raw("PUSH #0");
            } else if (c->get_constant_type() == FLOATING_LITERAL) {
                write_raw("PUSH #" + c->get_data());
                expr_type = make_cast(expr_type, FLOATING);
            } else if (c->get_constant_type() == INTEGER_LITERAL) {
                write_raw("PUSH #" + c->get_data());
                expr_type = make_cast(expr_type, INTEGER);
            } else if (c->get_constant_type() == STRING_LITERAL) {
                // remove single quotes, and replace with double
                string string_const = c->get_data();
                replace(string_const.begin(), string_const.end(), '\'', '"');
                write_raw("PUSH #" + string_const);
                expr_type = make_cast(expr_type, STRING);
            } else if (c->get_constant_type() == ADD) {
                if (expr_type == INTEGER)
                    write_raw("ADDS");
                else if (expr_type == FLOATING)
                    write_raw("ADDSF");
            } else if (c->get_constant_type() == SUB) {
                if (expr_type == INTEGER)
                    write_raw("SUBS");
                else if (expr_type == FLOATING)
                    write_raw("SUBSF");
            } else if (c->get_constant_type() == MUL) {
                if (expr_type == INTEGER)
                    write_raw("MULS");
                else if (expr_type == FLOATING)
                    write_raw("MULSF");
            } else if (c->get_constant_type() == DIV) {
                if (expr_type == INTEGER)
                    write_raw("DIVS");
                else if (expr_type == FLOATING)
                    write_raw("DIVSF");
            } else if (c->get_constant_type() == MOD) {
                write_raw("MODS");
            } else if (c->get_constant_type() == AND) {
                write_raw("ANDS");
            } else if (c->get_constant_type() == OR) {
                write_raw("ORS");
            } else if (c->get_constant_type() == NOT) {
                write_raw("NOTS");
            } else if (c->get_constant_type() == IEQ) {
                if (expr_type == INTEGER)
                    write_raw("CMPEQS");
                else if (expr_type == FLOATING)
                    write_raw("CMPEQSF");
                expr_type = BOOLEAN;
            } else if (c->get_constant_type() == IGT) {
                if (expr_type == INTEGER)
                    write_raw("CMPGTS");
                else if (expr_type == FLOATING)
                    write_raw("CMPGTSF");
                expr_type = BOOLEAN;
            } else if (c->get_constant_type() == IGE) {
                if (expr_type == INTEGER)
                    write_raw("CMPGES");
                else if (expr_type == FLOATING)
                    write_raw("CMPGESF");
                expr_type = BOOLEAN;
            } else if (c->get_constant_type() == ILT) {
                if (expr_type == INTEGER)
                    write_raw("CMPLTS");
                else if (expr_type == FLOATING)
                    write_raw("CMPLTSF");
                expr_type = BOOLEAN;
            } else if (c->get_constant_type() == ILE) {
                if (expr_type == INTEGER)
                    write_raw("CMPLES");
                else if (expr_type == FLOATING)
                    write_raw("CMPLESF");
                expr_type = BOOLEAN;
            } else if (c->get_constant_type() == INE) {
                if (expr_type == INTEGER)
                    write_raw("CMPNES");
                else if (expr_type == FLOATING)
                    write_raw("CMPNESF");
                expr_type = BOOLEAN;
            }
        }
    }
    return expr_type;
}

CodeBlockList::iterator CodeBlock::inner_begin() {
    return this->block_list->begin();
}

CodeBlockList::iterator CodeBlock::inner_end() {
    return this->block_list->end();
}

SymbolPtr CodeBlock::translate(TokenPtr token) {
    if (token->get_token() == MP_ID) {
    	string search_lexeme = token->get_lexeme();
    	unsigned int search_level = this->get_nesting_level();
        SymbolListPtr filtered_data = this->get_analyzer()->get_symtable()->data_in_scope_at(search_lexeme, search_level);
        if (this->check_filter_size(filtered_data)) {
            return *filtered_data->begin();
        } else {
            return nullptr;
        }
    	//return SymbolPtr(new SymData("None", VOID, GLOBAL, 0, nullptr));
    } else if (token->get_token() == MP_INT_LITERAL) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), INTEGER_LITERAL));
    } else if (token->get_token() == MP_STRING_LITERAL) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), STRING_LITERAL));
    } else if (token->get_token() == MP_FLOAT_LITERAL) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), FLOATING_LITERAL));
    } else if (token->get_token() == MP_TRUE) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), BOOLEAN_LITERAL_T));
    } else if (token->get_token() == MP_FALSE) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), BOOLEAN_LITERAL_F));
    } else if (token->get_token() == MP_LEFT_PAREN) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), LPAREN));
    } else if (token->get_token() == MP_RIGHT_PAREN) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), RPAREN));
    } else if (token->get_token() == MP_PLUS) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), ADD));
    } else if (token->get_token() == MP_MINUS) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), SUB));
    } else if (token->get_token() == MP_MULT) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), MUL));
    } else if (token->get_token() == MP_DIV) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), DIV));
    } else if (token->get_token() == MP_DIV_KW) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), DIV));
    } else if (token->get_token() == MP_MOD_KW) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), MOD));
    } else if (token->get_token() == MP_AND) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), AND));
    } else if (token->get_token() == MP_OR) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), OR));
    } else if (token->get_token() == MP_NOT) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), NOT));
    } else if (token->get_token() == MP_LESSTHAN) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), ILT));
    } else if (token->get_token() == MP_EQUALS) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), IEQ));
    } else if (token->get_token() == MP_LESSTHAN_EQUALTO) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), ILE));
    } else if (token->get_token() == MP_GREATERTHAN) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), IGT));
    } else if (token->get_token() == MP_GREATERTHAN_EQUALTO) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), IGE));
    } else if (token->get_token() == MP_NOT_EQUAL) {
        return SymbolPtr(new SymConstant(token->get_lexeme(), INE));
    } else {
        return nullptr;
    }
}

void CodeBlock::emit(InstructionType ins, vector<string> operands) {
    // does nothing yet
}

// Program Block stuff
void ProgramBlock::generate_pre() {
    // generate program entry point
    write_raw("MOV SP D0");
    // push begin symbols
    for (auto i = get_symbol_list()->begin(); i != get_symbol_list()->end(); i++) {
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
    return this->get_valid();
}

void ProgramBlock::preprocess() {
    // get symbols of global vars
    SymbolListPtr global_vars = this->get_analyzer()->get_symtable()->get_global_vars();
    // copy into local table
    for (auto i = global_vars->begin(); i != global_vars->end(); i++) {
        this->get_symbol_list()->push_back(*i);
    }
}

// Assignment Block stuff
void AssignmentBlock::generate_pre() {
    this->expr_type = this->generate_expr(this->get_symbol_list());
}

void AssignmentBlock::generate_post() {
    // pop into assigner
    SymDataPtr post_assigner = static_pointer_cast<SymData>(this->assigner);
    make_cast(this->expr_type, post_assigner->get_var_type());
    write_raw("POP " + post_assigner->get_address());
    write_raw("");
}

bool AssignmentBlock::validate() {
    return this->get_valid();
}

void AssignmentBlock::catch_token(TokenPtr symbol) {
	this->get_unprocessed()->push_back(symbol);
}

void AssignmentBlock::preprocess() {
    assert(this->get_unprocessed()->size());
    bool first_id = true;
    if (this->expr_only) {
        first_id = false;
    }
    for (auto i = this->get_unprocessed()->begin();
         i != this->get_unprocessed()->end(); i++) {
        if (first_id == true
            && (*i)->get_token() == MP_ID) {
        	TokenPtr t = *i;
            this->assigner = this->translate(t);
            first_id = false;
        } else if ((*i)->get_token() == MP_ASSIGNMENT) {
            continue;
        } else {
        	TokenPtr t = *i;
            SymbolPtr p = this->translate(t);
            if (p != nullptr) {
                this->get_symbol_list()->push_back(p);
            }
        }
    }
    // convert to postifx
    convert_postfix();
}

SymbolPtr AssignmentBlock::get_assigner() {
    return this->assigner;
}

VarType AssignmentBlock::get_expr_type() {
    return this->expr_type;
}

// IO Block stuff
void IOBlock::generate_pre() {
    assert(this->get_unprocessed()->size());
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
                report_msg_type("Semantic Error", "Constant value cannot be read to variable?");
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
    return this->get_valid();
}

void IOBlock::catch_token(TokenPtr token) {
    // filter by printable items
    if (token->get_token() == MP_ID
        || token->get_token() == MP_INT_LITERAL
        || token->get_token() == MP_STRING_LITERAL
        || token->get_token() == MP_FLOAT_LITERAL) {
        this->get_unprocessed()->push_back(token);
    }
}

void IOBlock::preprocess() {
    assert(this->get_unprocessed()->size());
    for (auto i = this->get_unprocessed()->begin();
         i != this->get_unprocessed()->end(); i++) {
    	TokenPtr t = *i;
        SymbolPtr p = translate(t);
        this->args->push_back(p);
    }
}

void LoopBlock::generate_pre() {
    if (this->type == RPTUNTLLOOP) {
        write_raw(this->body_label + ":\n");
    } else if (this->type == WHILELOOP) {
        write_raw(this->cond_label + ":\n");
        VarType result = this->generate_expr(this->get_symbol_list());
        if (result != BOOLEAN) {
            report_msg_type("Semantic Error",
                            "Conditional expression doesn't evaluate to boolean value.");
        }
        write_raw("\nBRTS " + this->body_label);
        write_raw("BR " + this->exit_label);
        write_raw(this->body_label + ":\n");
    } else if (this->type == FORLOOP) {
        // parse the assignment
        // process the assignment
        AssignmentBlockPtr assignment = AssignmentBlockPtr(new AssignmentBlock(false));
        assignment->set_analyzer(this->get_analyzer());
        for (auto i = 0; i < 3; i++) {
            assignment->catch_token((*this->get_unprocessed())[i]);
        }
        // generate its code
        assignment->preprocess();
        assignment->generate_pre();
        assignment->generate_post();
        // generate the condition label
        write_raw(this->cond_label + ":\n");
        // process the ordinal expression
        AssignmentBlockPtr ordinal_expr = AssignmentBlockPtr(new AssignmentBlock(true));
        ordinal_expr->set_analyzer(this->get_analyzer());
        for (unsigned int i = 4; i < this->get_unprocessed()->size(); i++) {
            ordinal_expr->catch_token((*this->get_unprocessed())[i]);
        }
        // get the comparison components for the ordinal expr
        TokenPtr incrementer = (*this->get_unprocessed())[3];
        if (incrementer->get_token() == MP_TO) {
            ordinal_expr->catch_token(TokenPtr(new Token(MP_EQUALS, ">", -1, -1)));
        } else if (incrementer->get_token() == MP_DOWNTO) {
            ordinal_expr->catch_token(TokenPtr(new Token(MP_EQUALS, "<", -1, -1)));
        }
        ordinal_expr->catch_token((*this->get_unprocessed())[0]);
        // generate its code
        ordinal_expr->preprocess();
        ordinal_expr->generate_pre();
        write_raw("\nBRFS " + this->body_label);
        write_raw("BR " + this->exit_label + "\n");
        write_raw(this->body_label + ":\n");
    }
}

void LoopBlock::generate_post() {
    if (this->type == RPTUNTLLOOP) {
        write_raw(this->cond_label + ":\n");
        VarType result = this->generate_expr(this->get_symbol_list());
        if (result != BOOLEAN) {
            report_msg_type("Semantic Error",
                            "Conditional expression doesn't evaluate to boolean value.");
        }
        write_raw("\nBRFS " + this->body_label);
        write_raw("BR " + this->exit_label + "\n");
        write_raw(this->exit_label + ":\n");
    } else if (this->type == WHILELOOP) {
        write_raw("BR " + this->cond_label);
        write_raw(this->exit_label + ":\n");
    } else if (this->type == FORLOOP) {
        // get the incrementer token
        TokenPtr incrementer = (*this->get_unprocessed())[3];
        if (incrementer->get_token() == MP_TO) {
            // generate an incrementer
            AssignmentBlockPtr inc = AssignmentBlockPtr(new AssignmentBlock(false));
            inc->set_analyzer(this->get_analyzer());
            inc->catch_token((*this->get_unprocessed())[0]);
            inc->catch_token((*this->get_unprocessed())[0]);
            inc->catch_token(TokenPtr(new Token(MP_PLUS, "+", -1, -1)));
            inc->catch_token(TokenPtr(new Token(MP_INT_LITERAL, "1", -1, -1)));
            inc->preprocess();
            inc->generate_pre();
            inc->generate_post();
        } else if (incrementer->get_token() == MP_DOWNTO) {
            // generate a decrementer
            AssignmentBlockPtr dec = AssignmentBlockPtr(new AssignmentBlock(false));
            dec->set_analyzer(this->get_analyzer());
            dec->catch_token((*this->get_unprocessed())[0]);
            dec->catch_token((*this->get_unprocessed())[0]);
            dec->catch_token(TokenPtr(new Token(MP_MINUS, "-", -1, -1)));
            dec->catch_token(TokenPtr(new Token(MP_INT_LITERAL, "1", -1, -1)));
            dec->preprocess();
            dec->generate_pre();
            dec->generate_post();
        }
        write_raw("BR " + this->cond_label + "\n");
        write_raw(this->exit_label + ":\n");
    }
}

void LoopBlock::preprocess() {
    this->cond_label = this->get_analyzer()->generate_label();
    this->body_label = this->get_analyzer()->generate_label();
    this->exit_label = this->get_analyzer()->generate_label();
    if (this->type == WHILELOOP
        || this->type == RPTUNTLLOOP) {
        // process the boolean statement
        for (auto i = this->get_unprocessed()->begin();
             i != this->get_unprocessed()->end(); i++) {
            this->get_symbol_list()->push_back(this->translate(*i));
        }
        this->convert_postfix();
    }
}

void LoopBlock::catch_token(TokenPtr symbol) {
    if (symbol->get_token() != MP_WHILE
        && symbol->get_token() != MP_REPEAT
        && symbol->get_token() != MP_UNTIL
        && symbol->get_token() != MP_DO
        && symbol->get_token() != MP_FOR
        && symbol->get_token() != MP_BEGIN
        && symbol->get_token() != MP_END
        && symbol->get_token() != MP_SEMI_COLON) {
        this->get_unprocessed()->push_back(symbol);
    }
}

bool LoopBlock::validate() {
    return true;
}

// Conditionals
void ConditionalBlock::catch_token(TokenPtr symbol) {
    if (symbol->get_token() != MP_IF
        && symbol->get_token() != MP_THEN
        && symbol->get_token() != MP_ELSE
        && symbol->get_token() != MP_BEGIN
        && symbol->get_token() != MP_END
        && symbol->get_token() != MP_SEMI_COLON) {
        this->get_unprocessed()->push_back(symbol);
    }
}

void ConditionalBlock::preprocess() {
    if (this->cond == COND_IF) {
        this->body_label = this->get_analyzer()->generate_label();
        this->exit_label = this->get_analyzer()->generate_label();
        if (this->connected != nullptr) {
            ConditionalBlockPtr else_block = static_pointer_cast<ConditionalBlock>(this->connected);
            if (else_block->get_conditional_type() == COND_ELSE) {
                else_block->set_else_label(this->exit_label);
                else_block->generate_exit_label();
            }
        }
    }
    for (auto i = this->get_unprocessed()->begin();
         i != this->get_unprocessed()->end(); i++) {
        this->get_symbol_list()->push_back(this->translate(*i));
    }
    this->convert_postfix();
}

void ConditionalBlock::generate_pre() {
    // generate condition if
    if (this->get_conditional_type() == COND_IF) {
        VarType result = this->generate_expr(this->get_symbol_list());
        if (result != BOOLEAN) {
            report_msg_type("Semantic Error",
                            "Conditional expression doesn't evaluate to boolean value.");
        }
        // true, jump to the body
        write_raw("\nBRTS " + this->body_label);
        if (this->connected != nullptr) {
            ConditionalBlockPtr extender = static_pointer_cast<ConditionalBlock>(this->connected);
            if (extender->get_conditional_type() == COND_ELSE) {
                // if there's an else part, jump on false to else
                write_raw("BR " + extender->else_label + "\n");
            }
        } else {
            // beginning of if statement with no else
            write_raw("BR " + this->exit_label + "\n");
        }
        // begin the if body part with a label
        write_raw(this->body_label + ":\n");
    }
}

void ConditionalBlock::generate_post() {
    // generate the final condition
    if (this->cond == COND_ELSE) {
        // if there's an else statement...
        // break from previous if statement to exit
        write_raw("BR " + this->exit_label + "\n");
        // write an exit label (end of the if statement)
        // beginning of the else statement
        write_raw(this->exit_label + ":\n");
    } else if (this->cond == COND_IF) {
        if (this->connected != nullptr) {
            // if there is an else statement
            ConditionalBlockPtr extender = static_pointer_cast<ConditionalBlock>(this->connected);
            if (extender->get_conditional_type() == COND_ELSE) {
                // at the end of the else, branch to the exit
                write_raw("BR " + extender->exit_label + "\n");
                // write the exit label
                write_raw(extender->else_label + ":\n");
            }
        } else {
            // end of if statement with no else
            write_raw(this->exit_label + ":\n");
        }
    }
}

void ConditionalBlock::set_connected(ConditionalBlockPtr new_connected) {
    this->connected = new_connected;
}

bool ConditionalBlock::validate() {
    return true;
}

CondType ConditionalBlock::get_conditional_type() {
    return this->cond;
}

void ConditionalBlock::set_else_label(string new_else_label) {
    this->else_label = new_else_label;
}

void ConditionalBlock::generate_exit_label() {
    this->exit_label = this->get_analyzer()->generate_label();
}

string ConditionalBlock::get_exit_label() {
    return this->exit_label;
}

// FP Decl part block
void FPDeclBlock::generate_pre() {
    write_raw("BR " + this->program_section + "\n");
}

void FPDeclBlock::generate_post() {
    write_raw("" + this->program_section + ":");
}

void FPDeclBlock::preprocess() {
    this->program_section = this->get_analyzer()->generate_label();
}

void FPDeclBlock::catch_token(TokenPtr symbol) {
	this->get_unprocessed()->push_back(symbol);
}

bool FPDeclBlock::validate() {
    if (this->get_analyzer() == nullptr
        || this->program_section.compare("") == 0) {
        this->set_valid(false);
        return false;
    } else {
        this->set_valid(true);
        return true;
    }
}

// Activation block types (body and call)
void ActivationBlock::generate_pre() {
    // do nothing at the moment
    write_raw(this->begin_label + ":");
}

void ActivationBlock::generate_post() {
    write_raw("RET");
}

void ActivationBlock::preprocess() {
    // get a label if declaration
    this->begin_label = this->get_analyzer()->generate_label();
}

void ActivationBlock::catch_token(TokenPtr symbol) {
	this->get_unprocessed()->push_back(symbol);
}

bool ActivationBlock::validate() {
    if (this->get_analyzer() != nullptr
        && this->record != nullptr) {
        this->set_valid(true);
        return true;
    } else {
        this->set_valid(false);
        return false;
    }
}
