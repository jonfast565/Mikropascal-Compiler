/*
 * Parser.hpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "Standard.hpp"
#include "Tokens.hpp"

class Parser {
private:
	shared_ptr<vector<shared_ptr<Token>>> token_list;
	shared_ptr<Scanner> scanner;
	shared_ptr<Token> lookahead;
	bool fromList;
public:
	Parser(shared_ptr<vector<shared_ptr<Token>>> token_list);
	Parser(shared_ptr<Scanner> scanner);
	void parseMe();
	void match(TokType expected);
	bool just_match(TokType expected)
	virtual ~Parser() {
		if (scanner != nullptr)
			scanner.reset();
		if (token_list != nullptr)
			token_list.reset();
	}
	// parse for all Mikropascal non-terminals
	void parse_system_goal();
	void parse_program();
	void parse_program_heading();
	// the program basically
	void parse_block();
	// variable declaration stuff
	void parse_variable_declaration_part();
	void parse_variable_declaration_tail();
	void parse_variable_declaration();
	void parse_type();
	// procedure and function declaration stuff
	void parse_procedure_and_function_declaration_part();
	void parse_procedure_declaration();
	void parse_function_declaration();
	void parse_procedure_heading();
	void parse_function_heading();
	void parse_optional_formal_parameter_list();
	void parse_formal_parameter_section_tail();
	void parse_formal_parameter_section();
	void parse_value_parameter_section();
	void parse_variable_parameter_section();
	// parse statements
	void parse_statement_part();
	void parse_compound_statement();
	void parse_statement_sequence();
	void parse_statement_tail();
	void parse_statement();
	void parse_empty_statement();
	void parse_read_statement();
	void parse_read_parameter_tail();
	void parse_read_parameter();
	void parse_write_statement();
	void parse_write_parameter_tail();
	void parse_write_parameter();
	void parse_assignment_statement();
	// parse control flow statements
	void parse_if_statement();
	void parse_optional_else_part();
	// parse looping statements
	void parse_repeat_statement();
	void parse_while_statement();
	void parse_for_statement();
	void parse_control_variable();
	void parse_initial_value();
	void parse_step_value();
	void parse_final_value();
	// parse procedure statements
	void parse_procedure_statement();
	void parse_optional_actual_parameter_list();
	void parse_actual_parameter_tail();
	void parse_actual_parameter();
	// parse expressions
	void parse_expression();
	void parse_optional_relational_part();
	void parse_relational_operator();
	void parse_simple_expression();
	void parse_term_tail();
	void parse_optional_sign();
	void parse_adding_operator();
	void parse_term();
	void parse_factor_tail();
	void parse_multiplying_operator();
	void parse_factor();
	// parse identifiers
	void parse_program_identifier();
	void parse_variable_identifier();
	void parse_procedure_identifier();
	void parse_function_identifier();
	void parse_boolean_expression();
	void parse_ordinal_expression();
	void parse_identifier_list();
	void parse_identifier_tail();
	void parse_identifier();
	// parse end of file
	void parse_eof();

	// helper functions
	bool is_relational_operator();
	bool is_multiplying_operator();

	// error reporting
	void report_error(string error);
	void report_match(string match);
	void next_token();
};


#endif /* PARSER_HPP_ */
