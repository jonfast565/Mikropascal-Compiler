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
#include "Scanner.hpp"
#include "Helper.hpp"
#include "Rules.hpp"

// AST Stuff
class AbstractNode {
private:
    bool is_root;
    bool is_rule;
    shared_ptr<AbstractNode> parent_node;
    shared_ptr<vector<shared_ptr<AbstractNode>>> child_nodes;
    ParseType parse_type;
    shared_ptr<Token> token;
public:
    AbstractNode();
    AbstractNode(shared_ptr<AbstractNode> parent_node, ParseType parse_type);
    AbstractNode(ParseType parse_type);
    AbstractNode(shared_ptr<Token> token);
    virtual ~AbstractNode(){};
    void add_child_node(shared_ptr<AbstractNode> child_node);
    void set_is_root(bool is_root);
    bool get_is_root();
    bool get_is_rule();
    bool get_is_epsilon();
    void set_parent(shared_ptr<AbstractNode> parent_node);
    shared_ptr<AbstractNode> get_parent();
    vector<shared_ptr<AbstractNode>>::iterator get_child_begin();
    vector<shared_ptr<AbstractNode>>::iterator get_child_end();
};

class AbstractTree {
private:
	shared_ptr<AbstractNode> root_node;
	shared_ptr<AbstractNode> iterable;
public:
	AbstractTree();
	AbstractTree(shared_ptr<AbstractNode> root);
	void add_move_child(shared_ptr<AbstractNode> child_node);
	void goto_parent();
	shared_ptr<AbstractNode> get_current_parent();
	void display_tree_at(shared_ptr<AbstractNode> iterable);
    void display_tree();
	virtual ~AbstractTree(){};
};

// Parser Stuff
class Parser {
private:
	shared_ptr<vector<shared_ptr<Token>>> token_list;
	shared_ptr<Scanner> scanner;
	shared_ptr<Token> lookahead;
    shared_ptr<AbstractTree> program_syntax;
	bool fromList;
    bool error_reported;
    unsigned int parse_depth;
public:
	Parser(shared_ptr<vector<shared_ptr<Token>>> token_list);
	Parser(shared_ptr<Scanner> scanner);
	void parse_me();
	void match(TokType expected);
	bool try_match(TokType expected);
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
    bool is_adding_operator();
    // grab the next token from the input stream
	void next_token();
    // indent/dedent the output by one level (debug)
    void more_indent();
    void less_indent();
    // ast helper methods
    void return_from();
    void go_into(ParseType parse_type);
    void go_into_lit(shared_ptr<Token> token);
};


#endif /* PARSER_HPP_ */
