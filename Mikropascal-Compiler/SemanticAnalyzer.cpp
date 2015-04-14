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
		if (current->get_is_rule()) {
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
SemanticAnalyzer::SemanticAnalyzer(string filedir) {
	this->ast = AbstractTreePtr(new AbstractTree());
	this->symbols = SymTablePtr(new SymTable());
	this->condensedst = CodeBlockPtr(new ProgramBlock());
	this->condensedst->set_analyzer(SemanticAnalyzerPtr(this));
	this->label_count = 0;
	this->block_stack = unique_ptr<stack<CodeBlockPtr>>(new stack<CodeBlockPtr>);
	this->block_stack->push(this->condensedst);
	this->filedir = filedir;
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

void SemanticAnalyzer::write_tof(string raw) {
	if (this->file_writer.is_open() && this->file_writer.good()) {
		this->file_writer << raw << endl;
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
		this->close_file();
		report_msg_type("Compilation Failed", "Validation failure.");
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
		report_msg_type("Semantic Error", "Redefinition of '" + callable_id + "' in multiple places");
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
	report_msg_type("Semantic Error", "No definition of '" + data_id + "' in " + callable_id);
	return false;
}

void SemanticAnalyzer::print_symbols() {
	this->symbols->print();
}

void SemanticAnalyzer::open_file(string program_name) {
	size_t npos = 0;
	#if __APPLE__
		npos = this->filedir.find_last_of('/');
	#elif _WIN32
		npos = this->filedir.find_last_of('\\');
	#endif
	string directory = filedir.substr(0, npos + 1);
	string new_path = directory + program_name + ".il";
	this->file_writer = ofstream(new_path);
}

void SemanticAnalyzer::close_file() {
	this->file_writer.close();
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

bool CodeBlock::is_unary(SymbolPtr character) {
	VarType op = static_pointer_cast<SymConstant>(character)->get_constant_type();
	if (op == NOT) {
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

VarType CodeBlock::make_cast(SymbolPtr p, VarType v1, VarType v2) {
	if (v1 != v2) {
		if (v1 == INTEGER && v2 == FLOATING) {
			// cast back to integer
			write_raw("CASTSI");
			return INTEGER;
		} else if (v1 == FLOATING && v2 == INTEGER) {
			// cast back to floating
			write_raw("CASTSF");
			return FLOATING;
		} else if ((v1 == STRING && v2 != STRING)
				   || (v1 != STRING && v2 == STRING)
				   || (v1 == VOID || v2 == VOID)) {
			if ((v1 == STRING_LITERAL && v2 == STRING) || (v1 == STRING && v2 == STRING_LITERAL)) {
				return STRING;
			}
			// need to catch boolean casting issues here...
			this->valid = false;
			report_error_lc("Semantic Error", "Unable to cast "
							+ var_type_to_string(v1)
							+ " to " + var_type_to_string(v2),
							p->get_row(), p->get_col());
			return VOID;
		} else if ((v1 == BOOLEAN && v2 != BOOLEAN)
				   || (v1 != BOOLEAN && v2 == BOOLEAN)) {
			return v2;
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

void CodeBlock::write_raw(string raw) {
	this->get_analyzer()->write_tof(raw);
}

bool CodeBlock::check_filter_size(SymbolListPtr filtered) {
	if (filtered == nullptr) {
		// too few ids
		return false;
	} else if (filtered->size() > 1
		|| filtered->size() == 0) {
		// too many ids found
		return false;
	} else {
		// correct
		return true;
	}
}

unsigned int CodeBlock::get_nesting_level() {
	// don't link a shared_ptr to this!
	CodeBlock* temp_parent_block = this;
	unsigned int level_found = 0;
	while(temp_parent_block != nullptr) {
		// if inside a function or procedure body
		if (temp_parent_block->get_block_type() == ACTIVATION_BLOCK) {
			level_found++;
		}
		temp_parent_block = temp_parent_block->get_parent().get();
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
	
	// shunting yard (modified for unary operators!)
	for (auto i = this->temp_symbols->begin(); i !=
		 this->temp_symbols->end(); i++) {
		if (is_operand(*i)) {
			unpostfix_symbols->push_back(*i);
			if (!op_stack->empty()) {
				if (is_unary(op_stack->top())) {
					unpostfix_symbols->push_back(op_stack->top());
					op_stack->pop();
				}
			}
		} else if (is_operator(*i)) {
			if (is_unary((*i))) {
				op_stack->push(*i);
			} else {
				while (!op_stack->empty()
					   && !is_lparen(op_stack->top())
					   && compare_ops(*i, op_stack->top()) <= 0) {
					unpostfix_symbols->push_back(op_stack->top());
					op_stack->pop();
				}
				op_stack->push(*i);
			}
		} else if (is_lparen(*i)) {
			op_stack->push(*i);
		} else if (is_rparen(*i)) {
			while (!op_stack->empty()) {
				if (is_lparen(op_stack->top())) {
					op_stack->pop();
					if (!op_stack->empty()) {
						if (is_unary(op_stack->top())) {
							unpostfix_symbols->push_back(op_stack->top());
							op_stack->pop();
						}
					}
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

SymbolListPtr CodeBlock::convert_postfix(SymbolListPtr p) {
	// data structs
	unique_ptr<stack<SymbolPtr>> op_stack = unique_ptr<stack<SymbolPtr>>(new stack<SymbolPtr>);
	SymbolListPtr unpostfix_symbols = SymbolListPtr(new SymbolList());
	
	// shunting yard (modified for unary operators!)
	for (auto i = p->begin(); i !=
		 p->end(); i++) {
		if (is_operand(*i)) {
			unpostfix_symbols->push_back(*i);
			if (!op_stack->empty()) {
				if (is_unary(op_stack->top())) {
					unpostfix_symbols->push_back(op_stack->top());
					op_stack->pop();
				}
			}
		} else if (is_operator(*i)) {
			if (is_unary((*i))) {
				op_stack->push(*i);
			} else {
				while (!op_stack->empty()
					   && !is_lparen(op_stack->top())
					   && compare_ops(*i, op_stack->top()) <= 0) {
					unpostfix_symbols->push_back(op_stack->top());
					op_stack->pop();
				}
				op_stack->push(*i);
			}
		} else if (is_lparen(*i)) {
			op_stack->push(*i);
		} else if (is_rparen(*i)) {
			while (!op_stack->empty()) {
				if (is_lparen(op_stack->top())) {
					op_stack->pop();
					if (!op_stack->empty()) {
						if (is_unary(op_stack->top())) {
							unpostfix_symbols->push_back(op_stack->top());
							op_stack->pop();
						}
					}
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
	return unpostfix_symbols;
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
		// if its data, then push an address
		if ((*i)->get_symbol_type() == SYM_DATA) {
			SymDataPtr d = static_pointer_cast<SymData>(*i);
			write_raw("PUSH " + d->get_address());
			expr_type = make_cast(d, expr_type, d->get_var_type());
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
				expr_type = make_cast(c, expr_type, FLOATING);
			} else if (c->get_constant_type() == INTEGER_LITERAL) {
				write_raw("PUSH #" + c->get_data());
				expr_type = make_cast(c, expr_type, INTEGER);
			} else if (c->get_constant_type() == STRING_LITERAL) {
				// remove single quotes, and replace with double
				string string_const = c->get_data();
				replace(string_const.begin(), string_const.end(), '\'', '"');
				write_raw("PUSH #" + string_const);
				expr_type = STRING;
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
			SymbolPtr found = *filtered_data->begin();
			found->set_col(token->get_column());
			found->set_row(token->get_line());
			return found;
		} else {
			SymbolListPtr global_data = this->get_analyzer()->get_symtable()->data_in_scope_at(search_lexeme, 0);
			if (this->check_filter_size(global_data)) {
				SymbolPtr found = *global_data->begin();
				found->set_col(token->get_column());
				found->set_row(token->get_line());
				return found;
			} else {
				if (global_data == nullptr) {
					report_error_lc("Semantic Error", "ID '" + search_lexeme + "' not found",
									token->get_line(), token->get_column());
				} else {
					report_error("Semantic Error", "ID '" + search_lexeme + "' redefined as...");
					for (auto it = global_data->begin(); it != global_data->end(); it++) {
						SymDataPtr dptr = static_pointer_cast<SymData>(*it);
						report_error_lc("Definition @", (*it)->get_symbol_name() + " as " + var_type_to_string(dptr->get_var_type()), (*it)->get_row(), (*it)->get_col());
					}
				}
				this->valid = false;
				return nullptr;
			}
		}
	} else if (token->get_token() == MP_INT_LITERAL) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), INTEGER_LITERAL));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_STRING_LITERAL) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), STRING_LITERAL));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_FLOAT_LITERAL) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), FLOATING_LITERAL));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_TRUE) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), BOOLEAN_LITERAL_T));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_FALSE) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), BOOLEAN_LITERAL_F));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_LEFT_PAREN) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), LPAREN));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_RIGHT_PAREN) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), RPAREN));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_PLUS) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), ADD));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_MINUS) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), SUB));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_MULT) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), MUL));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_DIV) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), DIV));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_DIV_KW) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), DIV));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_MOD_KW) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), MOD));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_AND) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), AND));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_OR) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), OR));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_NOT) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), NOT));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_LESSTHAN) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), ILT));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_EQUALS) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), IEQ));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_LESSTHAN_EQUALTO) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), ILE));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_GREATERTHAN) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), IGT));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_GREATERTHAN_EQUALTO) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), IGE));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else if (token->get_token() == MP_NOT_EQUAL) {
		SymbolPtr s = SymbolPtr(new SymConstant(token->get_lexeme(), INE));
		s->set_row(token->get_line());
		s->set_col(token->get_column());
		return s;
	} else {
		return nullptr;
	}
}

// Program Block stuff
void ProgramBlock::generate_pre() {
	// get the program id and set it as the open file
	TokenPtr p = *this->get_unprocessed()->begin();
	// open a file to write to
	this->get_analyzer()->open_file(p->get_lexeme());
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
	// close the file
	this->get_analyzer()->close_file();
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
	make_cast(post_assigner, post_assigner->get_var_type(), this->expr_type);
	write_raw("POP " + post_assigner->get_address());
	write_raw("");
}

bool AssignmentBlock::validate() {
	return this->get_valid();
}

void AssignmentBlock::catch_token(TokenPtr symbol) {
	if (symbol->get_token() != MP_COMMENT) {
		this->get_unprocessed()->push_back(symbol);
	}
}

void AssignmentBlock::preprocess() {
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
	if (this->get_valid() == true)
		this->convert_postfix();
}

SymbolPtr AssignmentBlock::get_assigner() {
	return this->assigner;
}

VarType AssignmentBlock::get_expr_type() {
	return this->expr_type;
}

// IO Block stuff
void IOBlock::generate_pre() {
	if (this->action == IO_WRITE) {
		for (auto i = this->expressions->begin();
			 i != this->expressions->end(); i++) {
			SymbolListPtr postfixes = this->convert_postfix(*i);
			this->generate_expr(postfixes);
			write_raw("WRTS");
		}
	} else if (this->action == IO_READ) {
		for (auto i = this->get_symbol_list()->begin();
			 i != this->get_symbol_list()->end(); i++) {
			if ((*i)->get_symbol_type() == SYM_DATA) {
				SymDataPtr p = static_pointer_cast<SymData>(*i);
				if (p->get_var_type() == INTEGER || p->get_var_type() == BOOLEAN) {
					write_raw("RD " + p->get_address());
				} else if (p->get_var_type() == FLOATING) {
					write_raw("RDF " + p->get_address());
				} else if (p->get_var_type() == STRING) {
					write_raw("RDS " + p->get_address());
				}
			} else if ((*i)->get_symbol_type() == SYM_CONSTANT) {
				report_error_lc("Semantic Error", "Cannot read to a constant.",
								(*i)->get_row(), (*i)->get_col());
			}
		}
	}
	if (this->line_terminator) {
		write_raw("PUSH #\"\"");
		write_raw("WRTLNS");
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
	// no filtering, since the capture is for
	// comma separated expressions
	if (token->get_token() != MP_WRITE
		&& token->get_token() != MP_WRITELN
		&& token->get_token() != MP_READ
		&& token->get_token() != MP_READLN)
	this->get_unprocessed()->push_back(token);
}

void IOBlock::preprocess() {
	// get rid of the beginning and end parens
	this->get_unprocessed()->erase(this->get_unprocessed()->begin());
	this->get_unprocessed()->erase(this->get_unprocessed()->end() - 1);
	if (this->action == IO_WRITE) {
		SymbolListPtr pre = SymbolListPtr(new SymbolList());
		for (auto i = this->get_unprocessed()->begin();
			 i != this->get_unprocessed()->end(); i++) {
			TokenPtr t = *i;
			if (t->get_token() != MP_COMMA) {
				SymbolPtr p = translate(t);
				pre->push_back(p);
			} else {
				// reset
				this->expressions->push_back(pre);
				pre = SymbolListPtr(new SymbolList());
			}
		}
		this->expressions->push_back(pre);
	} else if (this->action == IO_READ) {
		for (auto i = this->get_unprocessed()->begin();
			 i != this->get_unprocessed()->end(); i++) {
			TokenPtr t = *i;
			if (t->get_token() != MP_COMMA) {
				SymbolPtr p = translate(t);
				this->get_symbol_list()->push_back(p);
			}
		}
	}
}

void LoopBlock::generate_pre() {
	if (this->type == RPTUNTLLOOP) {
		write_raw(this->body_label + ":\n");
	} else if (this->type == WHILELOOP) {
		write_raw(this->cond_label + ":\n");
		VarType result = this->generate_expr(this->get_symbol_list());
		if (result != BOOLEAN) {
			report_error_lc("Semantic Error",
							"Conditional expression doesn't evaluate to boolean value.",
							(*this->get_symbol_list()->begin())->get_row(),
							(*this->get_symbol_list()->begin())->get_col());
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
			ordinal_expr->catch_token(TokenPtr(new Token(MP_EQUALS, "=", -1, -1)));
		} else if (incrementer->get_token() == MP_DOWNTO) {
			ordinal_expr->catch_token(TokenPtr(new Token(MP_EQUALS, "=", -1, -1)));
		}
		ordinal_expr->catch_token((*this->get_unprocessed())[0]);
		if (incrementer->get_token() == MP_TO) {
			ordinal_expr->catch_token(TokenPtr(new Token (MP_MINUS, "-", -1, -1)));
			ordinal_expr->catch_token(TokenPtr(new Token (MP_INT_LITERAL, "1", -1, -1)));
		} else if (incrementer->get_token() == MP_DOWNTO) {
			ordinal_expr->catch_token(TokenPtr(new Token (MP_PLUS, "+", -1, -1)));
			ordinal_expr->catch_token(TokenPtr(new Token (MP_INT_LITERAL, "1", -1, -1)));
		}
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
			report_error_lc("Semantic Error",
							"Conditional expression doesn't evaluate to boolean value.",
							(*this->get_symbol_list()->begin())->get_row(),
							(*this->get_symbol_list()->begin())->get_col());
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
		if (this->get_valid() == true)
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
	return this->get_valid();
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
	if (this->get_valid())
		this->convert_postfix();
}

void ConditionalBlock::generate_pre() {
	// generate condition if
	if (this->get_conditional_type() == COND_IF) {
		VarType result = this->generate_expr(this->get_symbol_list());
		if (result != BOOLEAN) {
			report_error_lc("Semantic Error",
							"Conditional expression doesn't evaluate to boolean value.",
							(*this->get_symbol_list()->begin())->get_row(),
							(*this->get_symbol_list()->begin())->get_col());
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
	return this->get_valid();
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

// JumpBlock part block
void JumpBlock::generate_pre() {
	if (!this->get_block_list()->empty()) {
		write_raw("BR " + this->program_section + "\n");
	}
}

void JumpBlock::generate_post() {
	if (!this->get_block_list()->empty()) {
		write_raw("" + this->program_section + ":\n");
	}
}

void JumpBlock::preprocess() {
	if (!this->get_block_list()->empty()) {
		this->program_section = this->get_analyzer()->generate_label();
	}
}

void JumpBlock::catch_token(TokenPtr symbol) {
	this->get_unprocessed()->push_back(symbol);
}

bool JumpBlock::validate() {
	if (!this->get_block_list()->empty()) {
		if (this->get_analyzer() == nullptr
			|| this->program_section.compare("") == 0) {
			this->set_valid(false);
			return false;
		} else {
			this->set_valid(true);
			return true;
		}
	} else {
		return true;
	}
}

// Activation block types (body and call)
void ActivationBlock::generate_pre() {
	// do nothing at the moment
	if (this->activity == DEFINITION) {
		// write begin label
		write_raw(this->begin_label + ":\n");
		// get locals
		string name = this->record->get_symbol_name();
		SymbolListPtr named_items = this->get_analyzer()->get_symtable()->find(name);
		SymbolListPtr locals = SymTable::filter_nest_level(SymTable::filter_data(named_items), this->get_nesting_level());
		// generate code to push them
		if (locals->size() > 0) {
			for (auto i = locals->begin(); i != locals->end(); i++) {
				if (static_pointer_cast<SymData>(*i)->get_var_type() == STRING) {
					write_raw("PUSH #\"\"");
				} else if (static_pointer_cast<SymData>(*i)->get_var_type() == FLOATING) {
					write_raw("PUSH #0.0");
				} else {
					write_raw("PUSH #0");
				}
			}
		}
	} else {
		// call
		if (this->activation == PROCEDURE) {
			
		} else if (this->activation == FUNCTION) {
			
		}
	}
}

void ActivationBlock::generate_post() {
	if (this->activity == DEFINITION) {
		// get locals
		string name = this->record->get_symbol_name();
		SymbolListPtr named_items = this->get_analyzer()->get_symtable()->find(name);
		SymbolListPtr locals = SymTable::filter_nest_level(SymTable::filter_data(named_items), this->get_nesting_level());
		unsigned long locals_size = locals->size();
		// move stack ptr minus local variables
		if (locals_size > 0) {
			write_raw("PUSH SP");
			write_raw("PUSH #" + conv_string(locals_size));
			write_raw("SUBS ");
			write_raw("POP SP");
		}
		write_raw("RET\n");
	} else {
		// call
		if (this->activation == PROCEDURE) {
			
		} else if (this->activation == FUNCTION) {
			
		}
	}
}

void ActivationBlock::preprocess() {
	if (this->activity == DEFINITION) {
		// get a label if declaration
		this->begin_label = this->get_analyzer()->generate_label();
	} else if (this->activity == CALL) {
		unsigned int level = this->get_nesting_level();
		// perform lookup
		SymbolListPtr lookup = this->get_analyzer()->get_symtable()->find(this->caller_name);
		SymbolListPtr call_lookup = SymTable::filter_nest_level(SymTable::filter_callable(lookup), level);
		if (this->check_filter_size(call_lookup)) {
			this->record = static_pointer_cast<SymCallable>(*call_lookup->begin());
		} else {
			report_msg_type("Semantic Error", "Caller not declared");
			this->set_valid(false);
		}
		// process other ids, etc.
		for (auto i = this->get_unprocessed()->begin();
			 i != this->get_unprocessed()->end(); i++) {
			this->get_symbol_list()->push_back(this->translate(*i));
		}
	}
}

void ActivationBlock::catch_token(TokenPtr symbol) {
	if (this->activity == DEFINITION) {
		if (symbol->get_token() != MP_BEGIN
			&& symbol->get_token() != MP_END
			&& symbol->get_token() != MP_SEMI_COLON) {
			this->get_unprocessed()->push_back(symbol);
		}
	} else if (this->activity == CALL) {
		bool first_id = true;
		if (first_id == true && symbol->get_token() == MP_ID) {
			this->caller_name = symbol->get_lexeme();
			first_id = false;
		} else {
			if (symbol->get_token() == MP_ID
				|| symbol->get_token() == MP_INT_LITERAL
				|| symbol->get_token() == MP_STRING_LITERAL
				|| symbol->get_token() == MP_FLOAT_LITERAL) {
				this->get_unprocessed()->push_back(symbol);
			}
		}
	}
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

string ActivationBlock::get_start() {
	return this->begin_label;
}

ActivityType ActivationBlock::get_activity() {
	return this->activity;
}

void ActivationBlock::set_caller_name(string id) {
	this->caller_name = id;
}
