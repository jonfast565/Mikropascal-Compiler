/*
 * Parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#include "Parser.hpp"

#define DEBUG_OUTPUT 1

Parser::Parser(shared_ptr<vector<shared_ptr<Token>>> token_list) {
	this->token_list = token_list;
	this->fromList = true;
}

// USE ONLY THIS CONSTRUCTOR FOR NOW!!!!
Parser::Parser(shared_ptr<Scanner> scanner) {
	this->scanner = scanner;
	this->fromList = false;
}

bool Parser::just_match(TokType expected) {
	if (this->lookahead->get_token() != expected)
		return false;
	else
		return true;
}

void Parser::match(TokType expected) {
	if (this->lookahead->get_token() != expected) {
		report_error(
				string(
						"Syntax error, expected "
								+ get_token_info(expected).first + " but got "
								+ get_token_info(this->lookahead->get_token()).first
								+ " instead. Fail!"));
	} else {
		// consume the token and get the next
		if (this->fromList == false) {
			// get the next token from the dispatcher
			this->lookahead = this->scanner->scan_one();
		} else if (this->fromList == true) {
			// implement this later... with detaching token list
		}
	}
}

void Parser::parseMe() {
	if (DEBUG_OUTPUT)
		cout << "In parser..." << endl;
	// put the next token in the global buffer
	// parse the system goal!
	this->next_token();
	this->parse_system_goal();
}

void Parser::parse_system_goal() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_SYSTEM_GOAL" << endl;
	// parse system goal
	this->parse_program();
	this->parse_eof();
}

void Parser::parse_eof() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_EOF" << endl;
	bool is_eof = this->just_match(MP_EOF);
	if (!is_eof) {
		report_error(string("No end-of-file detected."));
	}
}

void Parser::parse_program() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROGRAM" << endl;
	this->parse_program_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_PERIOD);
}

void Parser::parse_block() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_BLOCK" << endl;
	this->parse_variable_declaration_part();
	this->parse_procedure_and_function_declaration_part();
	this->parse_statement_part();
}

void Parser::parse_program_heading() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROGRAM_HEADING" << endl;
	this->match(MP_PROGRAM);
	this->parse_program_identifier();
}

void Parser::parse_variable_declaration_part() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VARIABLE_DECLARATION" << endl;
	// is an identifier clause
	if (this->just_match(MP_VAR)) {
		this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		this->parse_variable_declaration_tail();
	} else {
		// or matches epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_variable_declaration_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VARIABLE_DECL_TAIL" << endl;
	if (this->just_match(MP_VAR)) {
		this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		// recursive so watch out!
		this->parse_variable_declaration_tail();
	} else {
		// or matches epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_variable_declaration() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VARIABLE_DECL" << endl;
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_type() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_TYPE" << endl;
	bool integer_match = just_match(MP_INTEGER);
	bool float_match = just_match(MP_FLOAT);
	bool string_match = just_match(MP_STRING);
	bool boolean_match = just_match(MP_BOOLEAN);
	if (integer_match) {
		this->report_match("Matched integer decl.");
	} else if (float_match) {
		this->report_match("Matched float decl.");
	} else if (string_match) {
		this->report_match("Matched string decl.");
	} else if (boolean_match) {
		this->report_match("Matched boolean decl.");
	} else {
		this->report_error(
				string("Syntax is incorrect when matching identifier."));
	}
}

void Parser::parse_procedure_and_function_declaration_part() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROCEDURE_AND_FUNCTION_DECL_PART" << endl;
	if (this->just_match(MP_PROCEDURE)) {
		this->parse_procedure_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else if (this->just_match(MP_FUNCTION)) {
		this->parse_function_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else {
		// or epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_procedure_declaration() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROCEDURE_DECL" << endl;
	this->parse_procedure_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
}

void Parser::parse_function_declaration() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FUNCTION_DECL" << endl;
	this->parse_function_declaration();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
}

void Parser::parse_procedure_heading() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROCEDURE_HEADING" << endl;
	this->match(MP_PROCEDURE);
	this->parse_procedure_identifier();
	this->parse_optional_formal_parameter_list();
	this->parse_type();
}

void Parser::parse_function_heading() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FUNCTION_HEADING" << endl;
	this->match(MP_FUNCTION);
	this->parse_function_identifier();
	this->parse_optional_formal_parameter_list();
	this->parse_type();
}

void Parser::parse_optional_formal_parameter_list() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_OPT_FORMAL_PARAM_LIST" << endl;
	if (this->just_match(MP_LEFT_PAREN)) {
		this->match(MP_LEFT_PAREN);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
		this->match(MP_RIGHT_PAREN);
	} else {
		// or match epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_formal_parameter_section_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FORMAL_PARAM_SECTION_TAIL" << endl;
	if (this->just_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
	} else {
		// or match epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_formal_parameter_section() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FORMAL_PARAM_SECTION" << endl;
	if (this->just_match(MP_ID)) {
		this->parse_value_parameter_section();
	} else if (this->just_match(MP_VAR)) {
		this->parse_variable_parameter_section();
	}
}

void Parser::parse_value_parameter_section() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VAL_PARAM_SECTION" << endl;
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_variable_parameter_section() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VAL_PARAM_SECTION" << endl;
	this->match(MP_VAR);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_statement_part() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_STATEMENT_PART" << endl;
	this->parse_compound_statement();
}

void Parser::parse_compound_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_COMPOUND_STATEMENT" << endl;
	this->match(MP_BEGIN);
	this->parse_statement_sequence();
	this->match(MP_END);
}

void Parser::parse_statement_sequence() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_STATEMENT_SEQUENCE" << endl;
	this->parse_statement();
	this->parse_statement_tail();
}

void Parser::parse_statement_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_STATEMENT_TAIL" << endl;
	if (this->just_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_statement();
		this->parse_statement_tail();
	} else {
		// or match epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_STATEMENT" << endl;
	// parsing these will be trial and
	// error, so will have to add things for it?
	if (this->just_match(MP_READ) || this->just_match(MP_READLN))
		this->parse_read_statement();
	else if (this->just_match(MP_WRITE) || this->just_match(MP_WRITELN))
		this->parse_write_statement();
	else if (this->just_match(MP_ID))
		this->parse_assignment_statement();
	else if (this->just_match(MP_IF))
		this->parse_if_statement();
	else if (this->just_match(MP_WHILE))
		this->parse_while_statement();
	else if (this->just_match(MP_REPEAT))
		this->parse_repeat_statement();
	else if (this->just_match(MP_FOR))
		this->parse_for_statement();
	else if (this->just_match(MP_PROCEDURE))
		this->parse_procedure_statement();
	else if (this->just_match(MP_BEGIN))
		this->parse_compound_statement();
	else
		this->parse_empty_statement();
}

void Parser::parse_empty_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_EMPTY_STATEMENT" << endl;
	// basically one hell of an epsilon rule
	if (DEBUG_OUTPUT)
		cout << "EPSILON_MATCHED" << endl;
}

void Parser::parse_read_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_READ_STATEMENT" << endl;
	this->match(MP_READ);
	this->match(MP_LEFT_PAREN);
	this->parse_read_parameter();
	this->parse_read_parameter_tail();
	this->match(MP_RIGHT_PAREN);
}

void Parser::parse_read_parameter_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_READ_PARAM_TAIL" << endl;
	if (this->just_match(MP_COMMA)) {
		this->parse_read_parameter();
		this->parse_read_parameter_tail();
	} else {
		// matched epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_MATCHED" << endl;
	}
}

void Parser::parse_read_parameter() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_READ_PARAM" << endl;
	this->parse_variable_identifier();
}

void Parser::parse_write_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_WRITE_STATEMENT" << endl;
	if (this->just_match(MP_WRITELN)) {
		this->match(MP_WRITELN);
	} else if (this->just_match(MP_WRITE)) {
		this->match(MP_WRITE);
	}
	this->match(MP_LEFT_PAREN);
	this->parse_write_parameter();
	this->parse_write_parameter_tail();
	this->match(MP_RIGHT_PAREN);
}

void Parser::parse_write_parameter_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_WRITE_PARAM_TAIL" << endl;
	if (this->just_match(MP_COMMA)) {
		this->match(MP_COMMA);
		this->parse_write_parameter();
		this->parse_write_parameter_tail();
	} else {
		// matched epsilon
	}
}

void Parser::parse_write_parameter() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_WRITE_PARAM" << endl;
	this->parse_ordinal_expression();
}

void Parser::parse_assignment_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_ASSIGN_STATEMENT" << endl;
	if (this->just_match(MP_ID)) {
		this->parse_variable_identifier();
		this->match(MP_ASSIGNMENT);
		this->parse_expression();
	}
}

void Parser::parse_if_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_IF_STATEMENT" << endl;
	if (this->just_match(MP_IF)) {
		this->match(MP_IF);
		this->parse_boolean_expression();
		this->match(MP_THEN);
		this->parse_statement();
		this->parse_optional_else_part();
	} else {
		this->report_error(string("Weird..."));
	}
}

void Parser::parse_optional_else_part() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_OPTIONAL_ELSE_PART" << endl;
	if (this->just_match(MP_ELSE)) {
		// we have an else part
		this->match(MP_ELSE);
		this->parse_statement();
	} else {
		// or epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_repeat_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_REPEAT_STATEMENT" << endl;
	if (this->just_match(MP_REPEAT)) {
		this->match(MP_REPEAT);
		this->parse_statement_sequence();
		this->match(MP_UNTIL);
		this->parse_boolean_expression();
	} else {
		// report not the statement we
		// were looking for
		this->report_error(string("Weird..."));
	}
}

void Parser::parse_while_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_WHILE_STATEMENT" << endl;
	if (this->just_match(MP_WHILE)) {
		this->match(MP_WHILE);
		this->parse_boolean_expression();
		this->match(MP_DO);
		this->parse_statement();
	} else {
		// report not the statement we
		// were looking for
		this->report_error(string("Weird..."));
	}
}

void Parser::parse_for_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FOR_STATEMENT" << endl;
	if (this->just_match(MP_FOR)) {
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
		this->report_error(string("Weird..."));
	}
}

void Parser::parse_control_variable() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_CONTROL_VARIABLE" << endl;
	this->parse_variable_identifier();
}

void Parser::parse_initial_value() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_INITIAL_VALUE" << endl;
	this->parse_ordinal_expression();
}

void Parser::parse_step_value() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_STEP_VALUE" << endl;
	if (this->just_match(MP_TO)) {
		this->match(MP_TO);
	} else if (this->just_match(MP_DOWNTO)) {
		this->match(MP_DOWNTO);
	} else {
		// report some syntax error
		this->report_error(string("Yuck!!!!..."));
	}
}

void Parser::parse_final_value() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FINAL_VALUE" << endl;
	this->parse_ordinal_expression();
}

void Parser::parse_procedure_statement() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROCEDURE_STATEMENT" << endl;
	this->parse_procedure_identifier();
	this->parse_optional_actual_parameter_list();
}

void Parser::parse_optional_actual_parameter_list() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_OPTIONAL_ACTUAL_PARAM_LIST" << endl;
	if (this->just_match(MP_LEFT_PAREN)) {
		// optional list used
		this->match(MP_LEFT_PAREN);
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_actual_parameter_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_ACTUAL_PARAM_TAIL" << endl;
	if (this->just_match(MP_COMMA)) {
		// param tail used
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon used
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_actual_parameter() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_ACTUAL_PARAM" << endl;
	this->parse_ordinal_expression();
}

void Parser::parse_expression() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_EXPRESSION" << endl;
	this->parse_simple_expression();
	this->parse_optional_relational_part();
}

void Parser::parse_optional_relational_part() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_OPTIONAL_RELATIONAL_PART" << endl;
	if (this->is_relational_operator()) {
		this->parse_relational_operator();
		this->parse_simple_expression();
	} else {
		// epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_relational_operator() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_RELATIONAL_OPERATOR" << endl;
	if (this->just_match(MP_EQUALS))
		this->match(MP_EQUALS);
	else if (this->just_match(MP_LESSTHAN))
		this->match(MP_LESSTHAN);
	else if (this->just_match(MP_GREATERTHAN))
		this->match(MP_GREATERTHAN);
	else if (this->just_match(MP_GREATERTHAN_EQUALTO))
		this->match(MP_GREATERTHAN_EQUALTO);
	else if (this->just_match(MP_LESSTHAN_EQUALTO))
		this->match(MP_LESSTHAN_EQUALTO);
	else
		this->match(MP_NOT_EQUAL);
	// no error should happen since we checked
	// for relational before running
}

void Parser::parse_simple_expression() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_SIMPLE_EXPR" << endl;
	this->parse_optional_sign();
	this->parse_term();
	this->parse_term_tail();
}

void Parser::parse_optional_sign() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_OPT_SIGN" << endl;
	if (this->just_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->just_match(MP_MINUS))
		this->match(MP_MINUS);
	else {
		// no optional sign, epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_adding_operator() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_ADDING_OP" << endl;
	if (this->just_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->just_match(MP_MINUS))
		this->match(MP_MINUS);
	else if (this->just_match(MP_OR))
		this->match(MP_OR);
	else {
		// syntax error!!!!!
		this->report_error(string("Yuck!!!! Operator not correct."));
	}
}

void Parser::parse_term_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_TERM_TAIL" << endl;
	this->parse_adding_operator();
	this->parse_term();
	this->parse_term_tail();
}

void Parser::parse_term() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_TERM" << endl;
	this->parse_factor();
	this->parse_factor_tail();
}

void Parser::parse_factor_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FACTOR_TAIL" << endl;
	if (this->is_multiplying_operator()) {
		this->parse_multiplying_operator();
		this->parse_factor();
		this->parse_factor_tail();
	} else {
		// epsilon
		if (DEBUG_OUTPUT)
			cout << "EPSILON_REACHED" << endl;
	}
}

void Parser::parse_multiplying_operator() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_MULTIPLYING_OP" << endl;
	if (this->just_match(MP_MULT))
		this->match(MP_MULT);
	else if (this->just_match(MP_DIV))
		this->match(MP_DIV);
	else if (this->just_match(MP_AND))
		this->match(MP_AND);
	else if (this->just_match(MP_MOD_KW))
		this->match(MP_MOD_KW);
	else if (this->just_match(MP_DIV_KW))
		this->match(MP_DIV_KW);
	else {
		// syntax error!!!!!
		this->report_error(string("Yuck, no operator reached..."));
	}
}

void Parser::parse_factor() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FACTOR" << endl;
	if (this->just_match(MP_LEFT_PAREN)) {
		// assume an expression
		this->match(MP_LEFT_PAREN);
		this->parse_expression();
		this->match(MP_RIGHT_PAREN);
	} else if (this->just_match(MP_INT_LITERAL)) {
		this->match(MP_INT_LITERAL);
	} else if (this->just_match(MP_FLOAT_LITERAL)) {
		this->match(MP_FLOAT_LITERAL);
	} else if (this->just_match(MP_STRING_LITERAL)) {
		this->match(MP_STRING_LITERAL);
	} else if (this->just_match(MP_NOT)) {
		this->match(MP_NOT);
		this->parse_factor();
	} else {
		// would have expected a function identifier anyways
		this->parse_function_identifier();
		this->parse_optional_actual_parameter_list();
	}
}

void Parser::parse_program_identifier() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROGRAM_IDENTIFIER" << endl;
	// or this->match(MP_ID);
	this->parse_identifier();
}

void Parser::parse_variable_identifier() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_VARIABLE_IDENTIFIER" << endl;
	this->parse_identifier();
}

void Parser::parse_procedure_identifier() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_PROCEDURE_IDENTIFIER" << endl;
	this->parse_identifier();
}

void Parser::parse_function_identifier() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_FUNCTION_IDENTIFIER" << endl;
	this->parse_identifier();
}

void Parser::parse_boolean_expression() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_BOOLEAN_EXPRESSION" << endl;
	this->parse_expression();
}

void Parser::parse_ordinal_expression() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_ORDINAL_EXPRESSION" << endl;
	this->parse_expression();
}

void Parser::parse_identifier_list() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_IDENTIFIER_LIST" << endl;
	this->parse_identifier();
	this->parse_identifier_tail();
}

void Parser::parse_identifier_tail() {
	if (DEBUG_OUTPUT)
		cout << "PARSE_IDENTIFIER_TAIL" << endl;
	if (this->just_match(MP_COMMA)) {
		this->parse_identifier();
		this->parse_identifier_tail();
	} else {
		// epsilon
	}
}

void Parser::parse_identifier() {
	if (DEBUG_OUTPUT)
		cout << "GIVE_IDENTIFIER" << endl;
	this->match(MP_ID);
}

bool Parser::is_relational_operator() {
	if (DEBUG_OUTPUT)
		cout << "IS_RELATIONAL_OPERATOR" << endl;
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_NOT_EQUAL
			&& (int) lookahead_type >= MP_EQUALS) {
		return true;
	} else
		return false;
}

bool Parser::is_multiplying_operator() {
	if (DEBUG_OUTPUT)
		cout << "IS_MULTIPLYING_OPERATOR" << endl;
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_MOD_KW && (int) lookahead_type >= MP_MULT) {
		return true;
	} else if ((int) lookahead_type == MP_AND)
		return true;
	else
		return false;
}

void Parser::report_error(string error) {
	cout << "Parse Error: " << error << endl;
}

void Parser::report_match(string match) {
	cout << match << endl;
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

