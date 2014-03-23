/*
 * Parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#include "Parser.hpp"

// AST Stuff
AbstractTree::AbstractTree() {
	this->root_node = shared_ptr<AbstractNode>(new AbstractNode());
	this->iterable = this->root_node;
}

AbstractTree::AbstractTree(shared_ptr<AbstractNode> root) {
	this->root_node = root;
	this->iterable = this->root_node;
}

void AbstractTree::add_move_child(shared_ptr<AbstractNode> child_node) {
	this->iterable->add_child_node(child_node);
	child_node->set_parent(this->iterable);
	this->iterable = child_node;
}

void AbstractTree::goto_parent() {
	if (this->iterable->get_parent() != nullptr) {
		this->iterable = this->iterable->get_parent();
	}
}

shared_ptr<AbstractNode> AbstractTree::get_current_parent() {
	if (this->iterable->get_parent() != nullptr) {
		return this->iterable->get_parent();
	}
	return nullptr;
}

void AbstractTree::display_tree() {
    // display tree at the root, which is where it should be
    report_msg("AST Printable Tree");
    display_tree_rec();
}

void AbstractTree::display_tree_rec() {
    // naturally... recursive
    shared_ptr<stack<shared_ptr<AbstractNode>>> loop =
    shared_ptr<stack<shared_ptr<AbstractNode>>>(new stack<shared_ptr<AbstractNode>>);
    shared_ptr<stack<shared_ptr<AbstractNode>>> reversal =
    shared_ptr<stack<shared_ptr<AbstractNode>>>(new stack<shared_ptr<AbstractNode>>);
    loop->push(this->iterable);
    while (!loop->empty()) {
        // get the top and pop
        shared_ptr<AbstractNode> current = loop->top();
        loop->pop();
        if (current->get_is_rule()) {
            report_msg_type("AST Rule", get_rule_info(current->get_parse_type()));
            for (auto i = current->get_child_begin();
                 i != current->get_child_end(); i++) {
                reversal->push(*i);
            }
            while(!reversal->empty()) {
                loop->push(reversal->top());
                reversal->pop();
            }
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
	this->child_nodes = shared_ptr<vector<shared_ptr<AbstractNode>>>(new vector<shared_ptr<AbstractNode>>);
	this->parse_type = ROOT;
	this->is_root = true;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(ParseType parse_type) {
	this->child_nodes = shared_ptr<vector<shared_ptr<AbstractNode>>>(new vector<shared_ptr<AbstractNode>>);
	this->parse_type = parse_type;
	this->is_root = false;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(shared_ptr<AbstractNode> parent_node, ParseType parse_type) {
	this->child_nodes = shared_ptr<vector<shared_ptr<AbstractNode>>>(new vector<shared_ptr<AbstractNode>>);
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

void AbstractNode::add_child_node(shared_ptr<AbstractNode> child_node) {
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

shared_ptr<Token> AbstractNode::get_token() {
    if (is_rule)
        return nullptr;
    else
        return this->token;
}

void AbstractNode::set_parent(shared_ptr<AbstractNode> parent_node) {
	this->parent_node = parent_node;
}

shared_ptr<AbstractNode> AbstractNode::get_parent() {
	if (this->parent_node == nullptr) {
		return nullptr;
	} else {
		return this->parent_node;
	}
}

vector<shared_ptr<AbstractNode>>::iterator AbstractNode::get_child_begin() {
    return this->child_nodes->begin();
}

vector<shared_ptr<AbstractNode>>::iterator AbstractNode::get_child_end() {
    return this->child_nodes->end();
}

// Parser Stuff
Parser::Parser(shared_ptr<vector<shared_ptr<Token>>> token_list) {
	this->token_list = token_list;
	this->fromList = true;
	this->parse_depth = 0;
    this->program_syntax = shared_ptr<AbstractTree>(new AbstractTree());
    this->error_reported = false;

}

// USE ONLY THIS CONSTRUCTOR FOR NOW!!!!
Parser::Parser(shared_ptr<Scanner> scanner) {
	this->scanner = scanner;
	this->fromList = false;
	this->parse_depth = 0;
	this->program_syntax = shared_ptr<AbstractTree>(new AbstractTree());
    this->error_reported = false;
}

bool Parser::try_match(TokType expected) {
	if (this->lookahead->get_token() != expected)
		return false;
	else
		return true;
}

void Parser::match(TokType expected) {
	if (this->lookahead->get_token() != expected) {
        this->error_reported = true;
		report_error_lc("Parse Error",
                     string(
                            "Expected "
                            + get_token_info(expected).first + " but got '"
                            + get_token_info(this->lookahead->get_token()).first
                            + "' instead. Fail!"), this->lookahead->get_line(), this->lookahead->get_column());
        // scan in attempt to find a match?
        if (this->lookahead->get_token() != TokType::MP_EOF)
            // error checking assumes 'off-by-one'
            this->lookahead = this->scanner->scan_one();
	} else {
        // add token as a literal to the ast
        this->go_into_lit(this->lookahead);
        this->return_from();
		// consume the token and get the next
		if (this->fromList == false) {
            // report a match!
            report_parse(string("Match: " + get_token_info(expected).first + ": " + this->lookahead->get_lexeme()), this->parse_depth);
            // get the next token from the dispatcher
            if (this->lookahead->get_token() != TokType::MP_EOF)
                this->lookahead = this->scanner->scan_one();
		} else if (this->fromList == true) {
			// implement this later... with detaching token list
		}
	}
}

void Parser::parse_me() {
	// put the next token in the global buffer
	// parse the system goal!
	this->next_token();
	this->parse_system_goal();
}

void Parser::parse_system_goal() {
    this->more_indent();
    this->go_into(SYSTEM_GOAL);
	report_parse("PARSE_SYSTEM_GOAL", this->parse_depth);
	// parse system goal
    if (this->try_match(MP_PROGRAM))
        this->parse_program();
    this->parse_eof();
    this->return_from();
    this->less_indent();
    if (!this->error_reported)
        report_msg_type("Notice", "Parse was successful!");
    else
        report_msg_type("Notice", "Parse failed. Yuck!");
}

void Parser::parse_eof() {
    this->more_indent();
    this->go_into(EOF_RULE);
	report_parse("PARSE_EOF", this->parse_depth);
	bool is_eof = this->try_match(MP_EOF);
	if (!is_eof) {
		report_error("Parse Error", "No end-of-file detected. Missing newline at end of file?");
	} else {
        this->match(MP_EOF);
    }
	this->return_from();
    this->less_indent();
}

void Parser::parse_program() {
    this->more_indent();
    this->go_into(PROGRAM);
	report_parse("PARSE_PROGRAM", this->parse_depth);
	this->parse_program_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_PERIOD);
	this->return_from();
    this->less_indent();
}

void Parser::parse_block() {
    this->more_indent();
    this->go_into(BLOCK);
	report_parse("PARSE_BLOCK", this->parse_depth);
	this->parse_variable_declaration_part();
	this->parse_procedure_and_function_declaration_part();
	this->parse_statement_part();
	this->return_from();
    this->less_indent();
}

void Parser::parse_program_heading() {
    this->more_indent();
    this->go_into(PROGRAM_HEADING);
	report_parse("PARSE_PROGRAM_HEADING", this->parse_depth);
	this->match(MP_PROGRAM);
	this->parse_program_identifier();
	this->return_from();
    this->less_indent();
}

void Parser::parse_variable_declaration_part() {
    this->more_indent();
    this->go_into(VARIABLE_DECL_PART);
	report_parse("PARSE_VARIABLE_DECLARATION_PART", this->parse_depth);
	// is an identifier clause
	if (this->try_match(MP_VAR)) {
        this->match(MP_VAR);
		this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		this->parse_variable_declaration_tail();
	} else {
		// or matches epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
	this->return_from();
    this->less_indent();
}

void Parser::parse_variable_declaration_tail() {
    this->more_indent();
    this->go_into(VARIABLE_DECL_TAIL);
	report_parse("PARSE_VARIABLE_DECL_TAIL", this->parse_depth);
    if (this->try_match(MP_VAR)) {
        this->match(MP_VAR);
        this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		// recursive so watch out!
		this->parse_variable_declaration_tail();
    } else {
 		// or epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
    }
    this->return_from();
    this->less_indent();
}

void Parser::parse_variable_declaration() {
    this->more_indent();
    this->go_into(VARIABLE_DECL);
	report_parse("PARSE_VARIABLE_DECL", this->parse_depth);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
    this->return_from();
    this->less_indent();
}

void Parser::parse_type() {
    this->more_indent();
    this->go_into(TYPE);
	report_parse("PARSE_TYPE", this->parse_depth);
	if (try_match(MP_INTEGER)) {
        this->match(MP_INTEGER);
	} else if (try_match(MP_FLOAT)) {
        this->match(MP_FLOAT);
	} else if (try_match(MP_STRING)) {
        this->match(MP_STRING);
	} else if (try_match(MP_BOOLEAN)) {
        this->match(MP_BOOLEAN);
	} else {
		report_error("Parse Error",
        "Syntax is incorrect when matching type.");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_and_function_declaration_part() {
    this->more_indent();
    this->go_into(PROCEDURE_AND_FUNCTION_DECL_PART);
	report_parse("PARSE_PROCEDURE_AND_FUNCTION_DECL_PART", this->parse_depth);
	if (this->try_match(MP_PROCEDURE)) {
		this->parse_procedure_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else if (this->try_match(MP_FUNCTION)) {
		this->parse_function_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else {
		// or epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_declaration() {
    this->more_indent();
    this->go_into(PROCEDURE_DECL);
	report_parse("PARSE_PROCEDURE_DECL", this->parse_depth);
	this->parse_procedure_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
    this->return_from();
    this->less_indent();
}

void Parser::parse_function_declaration() {
    this->more_indent();
    this->go_into(FUNCTION_DECL);
	report_parse("PARSE_FUNCTION_DECL", this->parse_depth);
	this->parse_function_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_heading() {
    this->more_indent();
    this->go_into(PROCEDURE_HEAD);
	report_parse("PARSE_PROCEDURE_HEADING", this->parse_depth);
	this->match(MP_PROCEDURE);
	this->parse_procedure_identifier();
	this->parse_optional_formal_parameter_list();
    this->return_from();
    this->less_indent();
}

void Parser::parse_function_heading() {
    this->more_indent();
    this->go_into(FUNCTION_HEAD);
	report_parse("PARSE_FUNCTION_HEADING", this->parse_depth);
	this->match(MP_FUNCTION);
	this->parse_function_identifier();
	this->parse_optional_formal_parameter_list();
    this->match(MP_COLON);
	this->parse_type();
    this->return_from();
    this->less_indent();
}

void Parser::parse_optional_formal_parameter_list() {
    this->more_indent();
    this->go_into(OPT_FORMAL_PARAM_LIST);
	report_parse("PARSE_OPT_FORMAL_PARAM_LIST", this->parse_depth);
	if (this->try_match(MP_LEFT_PAREN)) {
		this->match(MP_LEFT_PAREN);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
		this->match(MP_RIGHT_PAREN);
	} else {
		// or match epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_formal_parameter_section_tail() {
    this->more_indent();
    this->go_into(FORMAL_PARAM_SECTION_TAIL);
	report_parse("PARSE_FORMAL_PARAM_SECTION_TAIL", this->parse_depth);
	if (this->try_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
	} else {
		// or match epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_formal_parameter_section() {
    this->more_indent();
    this->go_into(FORMAL_PARAM);
	report_parse("PARSE_FORMAL_PARAM_SECTION", this->parse_depth);
	if (this->try_match(MP_ID)) {
		this->parse_value_parameter_section();
	} else if (this->try_match(MP_VAR)) {
		this->parse_variable_parameter_section();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_value_parameter_section() {
    this->more_indent();
    this->go_into(VALUE_PARAM_SECTION);
	report_parse("PARSE_VAL_PARAM_SECTION", this->parse_depth);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
    this->return_from();
    this->less_indent();
}

void Parser::parse_variable_parameter_section() {
    this->more_indent();
    this->go_into(VARIABLE_PARAM_SECTION);
	report_parse("PARSE_VAL_PARAM_SECTION", this->parse_depth);
	this->match(MP_VAR);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
    this->return_from();
    this->less_indent();
}

void Parser::parse_statement_part() {
    this->more_indent();
    this->go_into(STATEMENT_PART);
	report_parse("PARSE_STATEMENT_PART", this->parse_depth);
	this->parse_compound_statement();
    this->return_from();
    this->less_indent();
}

void Parser::parse_compound_statement() {
    this->more_indent();
    this->go_into(COMPOUND_STATEMENT);
	report_parse("PARSE_COMPOUND_STATEMENT", this->parse_depth);
	this->match(MP_BEGIN);
	this->parse_statement_sequence();
	this->match(MP_END);
    this->return_from();
    this->less_indent();
}

void Parser::parse_statement_sequence() {
    this->more_indent();
    this->go_into(STATEMENT_SEQUENCE);
	report_parse("PARSE_STATEMENT_SEQUENCE", this->parse_depth);
	this->parse_statement();
	this->parse_statement_tail();
    this->return_from();
    this->less_indent();
}

void Parser::parse_statement_tail() {
    this->more_indent();
    this->go_into(STATEMENT_TAIL);
	report_parse("PARSE_STATEMENT_TAIL", this->parse_depth);
	if (this->try_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_statement();
		this->parse_statement_tail();
	} else {
		// or match epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_statement() {
    this->more_indent();
    this->go_into(STATEMENT);
	report_parse("PARSE_STATEMENT", this->parse_depth);
	// parsing these will be trial and
	// error, so will have to add things for it?
	if (this->try_match(MP_READ) || this->try_match(MP_READLN))
		this->parse_read_statement();
	else if (this->try_match(MP_WRITE) || this->try_match(MP_WRITELN))
		this->parse_write_statement();
	else if (this->try_match(MP_ID))
		this->parse_assignment_statement();
	else if (this->try_match(MP_IF))
		this->parse_if_statement();
	else if (this->try_match(MP_WHILE))
		this->parse_while_statement();
	else if (this->try_match(MP_REPEAT))
		this->parse_repeat_statement();
	else if (this->try_match(MP_FOR))
		this->parse_for_statement();
	else if (this->try_match(MP_PROCEDURE))
		this->parse_procedure_statement();
	else if (this->try_match(MP_BEGIN))
		this->parse_compound_statement();
	else
		this->parse_empty_statement();
    this->return_from();
    this->less_indent();
}

void Parser::parse_empty_statement() {
    this->more_indent();
    this->go_into(EMPTY_STATEMENT);
	report_parse("PARSE_EMPTY_STATEMENT", this->parse_depth);
    this->return_from();
    this->less_indent();
}

void Parser::parse_read_statement() {
    this->more_indent();
    this->go_into(READ_STATEMENT);
	report_parse("PARSE_READ_STATEMENT", this->parse_depth);
    if (this->try_match(MP_READ))
        this->match(MP_READ);
    else if (this->try_match(MP_READLN))
        this->match(MP_READLN);
	this->match(MP_LEFT_PAREN);
	this->parse_read_parameter();
	this->parse_read_parameter_tail();
	this->match(MP_RIGHT_PAREN);
    this->return_from();
    this->less_indent();
}

void Parser::parse_read_parameter_tail() {
    this->more_indent();
    this->go_into(READ_PARAMETER_TAIL);
	report_parse("PARSE_READ_PARAM_TAIL", this->parse_depth);
	if (this->try_match(MP_COMMA)) {
		this->parse_read_parameter();
		this->parse_read_parameter_tail();
	} else {
		// matched epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_read_parameter() {
    this->more_indent();
    this->go_into(READ_PARAMETER);
	report_parse("PARSE_READ_PARAM", this->parse_depth);
	this->parse_variable_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_write_statement() {
    this->more_indent();
    this->go_into(WRITE_STATEMENT);
	report_parse("PARSE_WRITE_STATEMENT", this->parse_depth);
	if (this->try_match(MP_WRITELN)) {
		this->match(MP_WRITELN);
	} else if (this->try_match(MP_WRITE)) {
		this->match(MP_WRITE);
	}
	this->match(MP_LEFT_PAREN);
	this->parse_write_parameter();
	this->parse_write_parameter_tail();
	this->match(MP_RIGHT_PAREN);
    this->return_from();
    this->less_indent();
}

void Parser::parse_write_parameter_tail() {
    this->more_indent();
    this->go_into(WRITE_PARAMETER_TAIL);
	report_parse("PARSE_WRITE_PARAM_TAIL", this->parse_depth);
	if (this->try_match(MP_COMMA)) {
		this->match(MP_COMMA);
		this->parse_write_parameter();
		this->parse_write_parameter_tail();
	} else {
		// matched epsilon
        report_parse("EPSILON_MATCHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_write_parameter() {
    this->more_indent();
    this->go_into(WRITE_PARAMETER);
	report_parse("PARSE_WRITE_PARAM", this->parse_depth);
	this->parse_ordinal_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_assignment_statement() {
    this->more_indent();
    this->go_into(ASSIGNMENT_STATEMENT);
	report_parse("PARSE_ASSIGN_STATEMENT", this->parse_depth);
	if (this->try_match(MP_ID)) {
		this->parse_variable_identifier();
		this->match(MP_ASSIGNMENT);
		this->parse_expression();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_if_statement() {
    this->more_indent();
    this->go_into(IF_STATEMENT);
	report_parse("PARSE_IF_STATEMENT", this->parse_depth);
	if (this->try_match(MP_IF)) {
		this->match(MP_IF);
		this->parse_boolean_expression();
		this->match(MP_THEN);
		this->parse_statement();
		this->parse_optional_else_part();
	} else {
		report_error("Parse Error", "If statement invalid.");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_optional_else_part() {
    this->more_indent();
    this->go_into(OPTIONAL_ELSE_PART);
	report_parse("PARSE_OPTIONAL_ELSE_PART", this->parse_depth);
	if (this->try_match(MP_ELSE)) {
		// we have an else part
		this->match(MP_ELSE);
		this->parse_statement();
	} else {
		// or epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_repeat_statement() {
    this->more_indent();
    this->go_into(REPEAT_STATEMENT);
	report_parse("PARSE_REPEAT_STATEMENT", this->parse_depth);
	if (this->try_match(MP_REPEAT)) {
		this->match(MP_REPEAT);
		this->parse_statement_sequence();
		this->match(MP_UNTIL);
		this->parse_boolean_expression();
	} else {
		// report not the statement we
		// were looking for
		report_msg("Weird...");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_while_statement() {
    this->more_indent();
    this->go_into(WHILE_STATEMENT);
	report_parse("PARSE_WHILE_STATEMENT", this->parse_depth);
	if (this->try_match(MP_WHILE)) {
		this->match(MP_WHILE);
		this->parse_boolean_expression();
		this->match(MP_DO);
		this->parse_statement();
	} else {
		// report not the statement we
		// were looking for
		report_msg("Weird...");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_for_statement() {
    this->more_indent();
    this->go_into(FOR_STATEMENT);
	report_parse("PARSE_FOR_STATEMENT", this->parse_depth);
	if (this->try_match(MP_FOR)) {
		this->match(MP_FOR);
		this->parse_control_variable();
		this->match(MP_ASSIGNMENT);
		this->parse_initial_value();
		this->parse_step_value();
		this->parse_final_value();
		this->match(MP_DO);
		this->parse_statement();
	} else {
		// report not the statement we
		// were looking for
		report_msg("Weird...");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_control_variable() {
    this->more_indent();
    this->go_into(CONTROL_VARIABLE);
	report_parse("PARSE_CONTROL_VARIABLE", this->parse_depth);
	this->parse_variable_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_initial_value() {
    this->more_indent();
    this->go_into(INITIAL_VALUE);
	report_parse("PARSE_INITIAL_VALUE", this->parse_depth);
	this->parse_ordinal_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_step_value() {
    this->more_indent();
    this->go_into(STEP_VALUE);
	report_parse("PARSE_STEP_VALUE", this->parse_depth);
	if (this->try_match(MP_TO)) {
		this->match(MP_TO);
	} else if (this->try_match(MP_DOWNTO)) {
		this->match(MP_DOWNTO);
	} else {
		// report some syntax error
		report_msg("Weird...");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_final_value() {
    this->more_indent();
    this->go_into(FINAL_VALUE);
	report_parse("PARSE_FINAL_VALUE", this->parse_depth);
	this->parse_ordinal_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_statement() {
    this->more_indent();
    this->go_into(PROCEDURE_STATEMENT);
	report_parse("PARSE_PROCEDURE_STATEMENT", this->parse_depth);
	this->parse_procedure_identifier();
	this->parse_optional_actual_parameter_list();
    this->return_from();
    this->less_indent();
}

void Parser::parse_optional_actual_parameter_list() {
    this->more_indent();
    this->go_into(OPTIONAL_ACTUAL_PARAMETER_LIST);
	report_parse("PARSE_OPTIONAL_ACTUAL_PARAM_LIST", this->parse_depth);
	if (this->try_match(MP_LEFT_PAREN)) {
		// optional list used
		this->match(MP_LEFT_PAREN);
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_actual_parameter_tail() {
    this->more_indent();
    this->go_into(ACTUAL_PARAMETER_TAIL);
	report_parse("PARSE_ACTUAL_PARAM_TAIL", this->parse_depth);
	if (this->try_match(MP_COMMA)) {
		// param tail used
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon used
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_actual_parameter() {
    this->more_indent();
    this->go_into(ACTUAL_PARAMETER);
	report_parse("PARSE_ACTUAL_PARAM", this->parse_depth);
	this->parse_ordinal_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_expression() {
    this->more_indent();
    this->go_into(EXPRESSION);
	report_parse("PARSE_EXPRESSION", this->parse_depth);
	this->parse_simple_expression();
	this->parse_optional_relational_part();
    this->return_from();
    this->less_indent();
}

void Parser::parse_optional_relational_part() {
    this->more_indent();
    this->go_into(OPTIONAL_RELATIONAL_PART);
	report_parse("PARSE_OPTIONAL_RELATIONAL_PART", this->parse_depth);
	if (this->is_relational_operator()) {
		this->parse_relational_operator();
		this->parse_simple_expression();
	} else {
		// epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_relational_operator() {
    this->more_indent();
    this->go_into(RELATIONAL_OPERATOR);
	report_parse("PARSE_RELATIONAL_OPERATOR", this->parse_depth);
	if (this->try_match(MP_EQUALS))
		this->match(MP_EQUALS);
	else if (this->try_match(MP_LESSTHAN))
		this->match(MP_LESSTHAN);
	else if (this->try_match(MP_GREATERTHAN))
		this->match(MP_GREATERTHAN);
	else if (this->try_match(MP_GREATERTHAN_EQUALTO))
		this->match(MP_GREATERTHAN_EQUALTO);
	else if (this->try_match(MP_LESSTHAN_EQUALTO))
		this->match(MP_LESSTHAN_EQUALTO);
	else if (this->try_match(MP_NOT_EQUAL))
        this->match(MP_NOT_EQUAL);
    else {
        // no error should happen since we checked
        // for relational before running
        // shouldn't happen
        report_msg("Weird...");
    }
    this->return_from();
    this->less_indent();
}

void Parser::parse_simple_expression() {
    this->more_indent();
    this->go_into(SIMPLE_EXPRESSION);
	report_parse("PARSE_SIMPLE_EXPR", this->parse_depth);
	this->parse_optional_sign();
	this->parse_term();
	this->parse_term_tail();
    this->return_from();
    this->less_indent();
}

void Parser::parse_optional_sign() {
    this->more_indent();
    this->go_into(OPTIONAL_SIGN);
	report_parse("PARSE_OPT_SIGN", this->parse_depth);
	if (this->try_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->try_match(MP_MINUS))
		this->match(MP_MINUS);
	else {
		// no optional sign, epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_adding_operator() {
    this->more_indent();
    this->go_into(ADDING_OPERATOR);
	report_parse("PARSE_ADDING_OP", this->parse_depth);
	if (this->try_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->try_match(MP_MINUS))
		this->match(MP_MINUS);
	else if (this->try_match(MP_OR))
		this->match(MP_OR);
	else {
		// syntax error!!!!!
		report_error("Parse Error", "Yuck!!!! Operator not correct.");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_term_tail() {
    this->more_indent();
    this->go_into(TERM_TAIL);
	report_parse("PARSE_TERM_TAIL", this->parse_depth);
    if (this->is_adding_operator()) {
        this->parse_adding_operator();
        this->parse_term();
        this->parse_term_tail();
    } else {
		// epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
    }
    this->return_from();
    this->less_indent();
}

void Parser::parse_term() {
    this->more_indent();
    this->go_into(TERM);
	report_parse("PARSE_TERM", this->parse_depth);
	this->parse_factor();
	this->parse_factor_tail();
    this->return_from();
    this->less_indent();
}

void Parser::parse_factor_tail() {
    this->more_indent();
    this->go_into(FACTOR_TAIL);
	report_parse("PARSE_FACTOR_TAIL", this->parse_depth);
	if (this->is_multiplying_operator()) {
		this->parse_multiplying_operator();
		this->parse_factor();
		this->parse_factor_tail();
	} else {
		// epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_multiplying_operator() {
    this->more_indent();
    this->go_into(MULTIPLYING_OPERATOR);
	report_parse("PARSE_MULTIPLYING_OP", this->parse_depth);
	if (this->try_match(MP_MULT))
		this->match(MP_MULT);
	else if (this->try_match(MP_DIV))
		this->match(MP_DIV);
	else if (this->try_match(MP_AND))
		this->match(MP_AND);
	else if (this->try_match(MP_MOD_KW))
		this->match(MP_MOD_KW);
	else if (this->try_match(MP_DIV_KW))
		this->match(MP_DIV_KW);
	else {
		// syntax error!!!!!
		report_error("Parse Error", "Yuck, no operator reached...");
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_factor() {
    this->more_indent();
    this->go_into(FACTOR);
	report_parse("PARSE_FACTOR", this->parse_depth);
	if (this->try_match(MP_LEFT_PAREN)) {
		// assume an expression
		this->match(MP_LEFT_PAREN);
		this->parse_expression();
		this->match(MP_RIGHT_PAREN);
	} else if (this->try_match(MP_INT_LITERAL)) {
		this->match(MP_INT_LITERAL);
	} else if (this->try_match(MP_FLOAT_LITERAL)) {
		this->match(MP_FLOAT_LITERAL);
	} else if (this->try_match(MP_STRING_LITERAL)) {
		this->match(MP_STRING_LITERAL);
	} else if (this->try_match(MP_NOT)) {
		this->match(MP_NOT);
		this->parse_factor();
	} else {
		// would have expected a function identifier anyways
		this->parse_function_identifier();
		this->parse_optional_actual_parameter_list();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_program_identifier() {
    this->more_indent();
    this->go_into(PROGRAM_IDENTIFIER);
	report_parse("PARSE_PROGRAM_IDENTIFIER", this->parse_depth);
	// or this->match(MP_ID);
	this->parse_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_variable_identifier() {
    this->more_indent();
    this->go_into(VARIABLE_IDENTIFIER);
	report_parse("PARSE_VARIABLE_IDENTIFIER", this->parse_depth);
	this->parse_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_identifier() {
    this->more_indent();
    this->go_into(PROCEDURE_IDENTIFIER);
	report_parse("PARSE_PROCEDURE_IDENTIFIER", this->parse_depth);
	this->parse_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_function_identifier() {
    this->more_indent();
    this->go_into(FUNCTION_IDENTFIER);
	report_parse("PARSE_FUNCTION_IDENTIFIER", this->parse_depth);
	this->parse_identifier();
    this->return_from();
    this->less_indent();
}

void Parser::parse_boolean_expression() {
    this->more_indent();
    this->go_into(BOOLEAN_EXPRESSION);
	report_parse("PARSE_BOOLEAN_EXPRESSION", this->parse_depth);
	this->parse_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_ordinal_expression() {
    this->more_indent();
    this->go_into(ORDINAL_EXPRESSION);
	report_parse("PARSE_ORDINAL_EXPRESSION", this->parse_depth);
	this->parse_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_identifier_list() {
    this->more_indent();
    this->go_into(IDENTFIER_LIST);
	report_parse("PARSE_IDENTIFIER_LIST", this->parse_depth);
	this->parse_identifier();
	this->parse_identifier_tail();
    this->return_from();
    this->less_indent();
}

void Parser::parse_identifier_tail() {
    this->more_indent();
    this->go_into(IDENTFIER_TAIL);
	report_parse("PARSE_IDENTIFIER_TAIL", this->parse_depth);
	if (this->try_match(MP_COMMA)) {
        this->match(MP_COMMA);
		this->parse_identifier();
		this->parse_identifier_tail();
	} else {
		// epsilon
        report_parse("EPSILON_REACHED", this->parse_depth);
        this->go_into(EPSILON);
        this->return_from();
	}
    this->return_from();
    this->less_indent();
}

void Parser::parse_identifier() {
	report_parse("GIVE_IDENTIFIER", this->parse_depth);
	this->match(MP_ID);
}

bool Parser::is_relational_operator() {
	report_parse("IS_RELATIONAL_OPERATOR", this->parse_depth);
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_NOT_EQUAL
        && (int) lookahead_type >= MP_EQUALS) {
		return true;
	} else
		return false;
}

bool Parser::is_multiplying_operator() {
	report_parse("IS_MULTIPLYING_OPERATOR", this->parse_depth);
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_MOD_KW && (int) lookahead_type >= MP_MULT) {
		return true;
	} else if ((int) lookahead_type == MP_AND)
		return true;
	else
		return false;
}

bool Parser::is_adding_operator() {
    report_parse("IS_ADDING_OPERATOR", this->parse_depth);
    TokType lookahead_type = this->lookahead->get_token();
    if ((int) lookahead_type == MP_PLUS || (int) lookahead_type == MP_MINUS ||
        (int) lookahead_type == MP_OR)
		return true;
	else
		return false;
}

void Parser::more_indent() {
    this->parse_depth++;
}

void Parser::less_indent() {
    this->parse_depth--;
}

void Parser::next_token() {
	// consume the token and get the next
	if (this->fromList == false) {
		// get the next token from the dispatcher
		this->lookahead = this->scanner->scan_one();
	} else if (this->fromList == true) {
		// implement this later... with detaching token list
	}
}

void Parser::return_from() {
	this->program_syntax->goto_parent();
}

void Parser::go_into(ParseType parse_type) {
	this->program_syntax->add_move_child(shared_ptr<AbstractNode>(new AbstractNode(parse_type)));
}

void Parser::go_into_lit(shared_ptr<Token> token) {
    this->program_syntax->add_move_child(shared_ptr<AbstractNode>(new AbstractNode(token)));
}

void Parser::print_parse() {
    this->program_syntax->display_tree();
}

