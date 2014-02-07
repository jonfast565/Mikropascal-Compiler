/*
 * Parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#include "Parser.hpp"

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
	if (this->lookahead != expected)
		return false;
	else
		return true;
}

void Parser::match(TokType expected) {
	if (this->lookahead != expected) {
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
	// put the next token in the global buffer
	// parse the system goal!
	this->next_token();
	this->parse_system_goal();
}

void Parser::parse_system_goal() {
	// parse system goal
	this->parse_program();
	this->parse_eof();
}

void Parser::parse_eof() {
	bool is_eof = this->just_match(MP_EOF);
	if (!is_eof) {
		report_error(string("No end-of-file detected."));
	}
}

void Parser::parse_program() {
	this->parse_program_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_PERIOD);
}

void Parser::parse_variable_declaration_part() {
	// is an identifier clause
	if (this->just_match(MP_VAR)) {
		this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		this->parse_variable_declaration_tail();
	} else {
		// or matches epsilon
	}
}

void Parser::parse_variable_declaration_tail() {
	if (this->just_match(MP_VAR)) {
		this->parse_variable_declaration();
		this->match(MP_SEMI_COLON);
		// recursive so watch out!
		this->parse_variable_declaration_tail();
	} else {
		// or matches epsilon
	}
}

void Parser::parse_variable_declaration() {
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_type() {
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
	if (this->just_match(MP_PROCEDURE)) {
		this->parse_procedure_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else if (this->just_match(MP_FUNCTION)) {
		this->parse_function_declaration();
		this->parse_procedure_and_function_declaration_part();
	} else {
		// or epsilon
	}
}

void Parser::parse_procedure_declaration() {
	this->parse_procedure_heading();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
}

void Parser::parse_function_declaration() {
	this->parse_function_declaration();
	this->match(MP_SEMI_COLON);
	this->parse_block();
	this->match(MP_SEMI_COLON);
}

void Parser::parse_procedure_heading() {
	this->match(MP_PROCEDURE);
	this->parse_procedure_identifier();
	this->parse_optional_formal_parameter_list();
	this->parse_type();
}

void Parser::parse_function_heading() {
	this->match(MP_FUNCTION);
	this->parse_function_identifier();
	this->parse_optional_formal_parameter_list();
	this->parse_type();
}

void Parser::parse_optional_formal_parameter_list() {
	if (this->just_match(MP_LEFT_PAREN)) {
		this->match(MP_LEFT_PAREN);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
		this->match(MP_RIGHT_PAREN);
	} else {
		// or match epsilon
	}
}

void Parser::parse_formal_parameter_section_tail() {
	if (this->just_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_formal_parameter_section();
		this->parse_formal_parameter_section_tail();
	} else {
		// or match epsilon
	}
}

void Parser::parse_formal_parameter_section() {
	if (this->just_match(MP_ID)) {
		this->parse_value_parameter_section();
	} else if (this->just_match(MP_VAR)) {
		this->parse_variable_parameter_section();
	}
}

void Parser::parse_value_parameter_section() {
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_variable_parameter_section() {
	this->match(MP_VAR);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
}

void Parser::parse_statement_part() {
	this->parse_compound_statement();
}

void Parser::parse_compound_statement() {
	this->match(MP_BEGIN);
	this->parse_statement_sequence();
	this->match(MP_END);
}

void Parser::parse_statement_sequence() {
	this->parse_statement();
	this->parse_statement_tail();
}

void Parser::parse_statement_tail() {
	if (this->just_match(MP_SEMI_COLON)) {
		this->match(MP_SEMI_COLON);
		this->parse_statement();
		this->parse_statement_tail();
	} else {
		// or match epsilon
	}
}

void Parser::parse_statement() {
	this->parse_compound_statement();
	this->parse_read_statement();
	this->parse_write_statement();
	this->parse_assignment_statement();
	this->parse_if_statement();
	this->parse_while_statement();
	this->parse_repeat_statement();
	this->parse_for_statement();
	this->parse_procedure_statement();
}

void Parser::parse_empty_statement() {
	// ? epsilon here
}

void Parser::parse_read_statement() {
	this->match(MP_READ);
	this->match(MP_LEFT_PAREN);
	this->parse_read_parameter();
	this->parse_read_parameter_tail();
	this->match(MP_RIGHT_PAREN);
}

void Parser::parse_read_parameter_tail() {
	if (this->just_match(MP_COMMA)) {
		this->parse_read_parameter();
		this->parse_read_parameter_tail();
	} else {
		// matched epsilon
	}
}

void Parser::parse_read_parameter() {
	this->parse_variable_identifier();
}

void Parser::parse_write_statement() {
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
	if (this->just_match(MP_COMMA)) {
		this->match(MP_COMMA);
		this->parse_write_parameter();
		this->parse_write_parameter_tail();
	} else {
		// matched epsilon
	}
}

void Parser::parse_write_parameter() {
	this->parse_ordinal_expression();
}

void Parser::parse_assignment_statement() {
	// do something clever here...
	// it won't work otherwise
	this->parse_variable_identifier();
	this->match(MP_ASSIGNMENT);
	this->parse_expression();

	// or...
	this->parse_variable_identifier();
	this->match(MP_ASSIGNMENT);
	this->parse_expression();
}

void Parser::parse_if_statement() {
	if (this->just_match(MP_IF)) {
		this->match(MP_IF);
		this->parse_boolean_expression();
		this->match(MP_THEN);
		this->parse_statement();
		this->parse_optional_else_part();
	} else {
		// report not the statement
		// we were looking for
	}
}

void Parser::parse_optional_else_part() {
	if (this->just_match(MP_ELSE)) {
		// we have an else part
		this->match(MP_ELSE);
		this->parse_statement();
	} else {
		// or epsilon
	}
}

void Parser::parse_repeat_statement() {
	if (this->just_match(MP_REPEAT)) {
		this->match(MP_REPEAT);
		this->parse_statement_sequence();
		this->match(MP_UNTIL);
		this->parse_boolean_expression();
	} else {
		// report not the statement we
		// were looking for
	}
}

void Parser::parse_while_statement() {
	if (this->just_match(MP_WHILE)) {
		this->match(MP_WHILE);
		this->parse_boolean_expression();
		this->match(MP_DO);
		this->parse_statement();
	} else {
		// report not the statement we
		// were looking for
	}
}

void Parser::parse_for_statement() {
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
	}
}

void Parser::parse_control_variable() {
	this->parse_variable_identifier();
}

void Parser::parse_initial_value() {
	this->parse_ordinal_expression();
}

void Parser::parse_step_value() {
	if (this->just_match(MP_TO)) {
		this->match(MP_TO);
	} else if (this->just_match(MP_DOWNTO)) {
		this->match(MP_DOWNTO);
	} else {
		// report some syntax error
	}
}

void Parser::parse_final_value() {
	this->parse_ordinal_expression();
}

void Parser::parse_procedure_statement() {
	this->parse_procedure_identifier();
	this->parse_optional_actual_parameter_list();
}

void Parser::parse_optional_actual_parameter_list() {
	if (this->just_match(MP_LEFT_PAREN)) {
		// optional list used
		this->match(MP_LEFT_PAREN);
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon
	}
}

void Parser::parse_actual_parameter_tail() {
	if (this->just_match(MP_COMMA)) {
		// param tail used
		this->parse_actual_parameter();
		this->parse_actual_parameter_tail();
	} else {
		// epsilon used
	}
}

void Parser::parse_actual_parameter() {
	this->parse_ordinal_expression();
}

void Parser::parse_expression() {
	this->parse_simple_expression();
	this->parse_optional_relational_part();
}

void Parser::parse_optional_relational_part() {
	if (this->is_relational_operator()) {
		this->parse_relational_operator();
		this->parse_simple_expression();
	} else {
		// epsilon
	}
}

void Parser::parse_relational_operator() {
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
	this->parse_optional_sign();
	this->parse_term();
	this->parse_term_tail();
}

void Parser::parse_optional_sign() {
	if (this->just_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->just_match(MP_MINUS))
		this->match(MP_MINUS);
	else {
		// no optional sign, epsilon
	}
}

void Parser::parse_adding_operator() {
	if (this->just_match(MP_PLUS))
		this->match(MP_PLUS);
	else if (this->just_match(MP_MINUS))
		this->match(MP_MINUS);
	else if (this->just_match(MP_OR))
		this->match(MP_OR);
	else {
		// syntax error!!!!!
	}
}

void Parser::parse_term_tail() {
	this->parse_adding_operator();
	this->parse_term();
	this->parse_term_tail();
}

void Parser::parse_term() {
	this->parse_factor();
	this->parse_factor_tail();
}

void Parser::parse_factor_tail() {
	if (this->is_multiplying_operator()) {
		this->parse_multiplying_operator();
		this->parse_factor();
		this->parse_factor_tail();
	} else {
		// epsilon
	}
}

void Parser::parse_multiplying_operator() {
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
	}
}

void Parser::parse_factor() {
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
	// or this->match(MP_ID);
	this->parse_identifier();
}

void Parser::parse_variable_identifier() {
	this->parse_identifier();
}

void Parser::parse_procedure_identifier() {
	this->parse_identifier();
}

void Parser::parse_function_identifier() {
	this->parse_identifier();
}

void Parser::parse_boolean_expression() {
	this->parse_expression();
}

void Parser::parse_ordinal_expression() {
	this->parse_expression();
}

void Parser::parse_identifier_list() {
	this->parse_identifier();
	this->parse_identifier_tail();
}

void Parser::parse_identifier_tail() {
	if (this->just_match(MP_COMMA)) {
		this->parse_identifier();
		this->parse_identifier_tail();
	} else {
		// epsilon
	}
}

bool Parser::is_relational_operator() {
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_NOT_EQUAL
			&& (int) lookahead_type >= MP_EQUALS) {
		return true;
	} else
		return false;
}

bool Parser::is_multiplying_operator() {
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

