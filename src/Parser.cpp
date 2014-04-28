/*
 * Parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#include "Parser.hpp"

Parser::Parser(ScannerPtr scanner, SemanticAnalyzerPtr analyzer) {
	this->scanner = scanner;
	this->parse_depth = 0;
    this->error_reported = false;
    this->analyzer = analyzer;
    this->symbols = TokenListPtr(new TokenList());
    this->sym_collect = false;
    this->gen_collect = shared_ptr<stack<int>>(new stack<int>);
}

bool Parser::try_match(TokType expected) {
	if (this->lookahead->get_token() != expected) {
		return false;
    } else {
		return true;
    }
}

void Parser::match(TokType expected) {
	if (this->lookahead->get_token() != expected) {
        // get the error information
        this->error_reported = true;
        string expect = get_token_info(expected).first;
        string received = get_token_info(this->lookahead->get_token()).first;
        unsigned long line = this->lookahead->get_line();
        unsigned long column = this->lookahead->get_column();
        // report the error
		report_error_lc("Parse Error", "Expected "
                        + expect + " but received '" + received
                        + "' instead. Fail!", line, column);
        // catastrophic mode
        if (this->lookahead->get_token() != TokType::MP_EOF) {
            // error checking assumes 'off-by-one'
            this->populate();
        }
        // stop
        exit(0);
	} else {
        // if we're collecting symbols
        if (this->sym_collect) {
            if (this->var_skip == true) {
                if ((this->lookahead->get_token() != MP_VAR)
                    & (this->lookahead->get_token() == MP_ID
                        || this->lookahead->get_token() == MP_INTEGER
                        || this->lookahead->get_token() == MP_FLOAT
                        || this->lookahead->get_token() == MP_STRING
                        || this->lookahead->get_token() == MP_BOOLEAN)) {
                    this->symbols->push_back(this->lookahead);
                }
            } else {
                if (this->lookahead->get_token() == MP_VAR
                       || this->lookahead->get_token() == MP_ID
                       || this->lookahead->get_token() == MP_INTEGER
                       || this->lookahead->get_token() == MP_FLOAT
                       || this->lookahead->get_token() == MP_STRING
                       || this->lookahead->get_token() == MP_BOOLEAN) {
                    this->symbols->push_back(this->lookahead);
                }
            }
        }
        // if we're collecting code generation data...
        if (!this->gen_collect->empty()) {
            // feed the token to the code block on top of the analyzer
            this->get_analyzer()->feed_token(this->lookahead);
        }
        // add token as a literal to the ast
        this->go_into_lit(this->lookahead);
        this->return_from();
        // report a match!
        report_parse("Match: " + get_token_info(expected).first
                     + ": " + this->lookahead->get_lexeme(), this->parse_depth);
        
        // get the next token from the dispatcher
        if (this->lookahead->get_token() != TokType::MP_EOF) {
            this->populate();
        }
	}
}

void Parser::print_sym_buffer() {
    // debug the symbol buffer
    for (auto i = this->symbols->begin(); i != this->symbols->end(); i++) {
        cout << get_token_info((*i)->get_token()).first << " ";
    }
    cout << endl;
}

void Parser::populate() {
    // scan and populate token buffer
    this->lookahead = this->scanner->scan_one();
    // look for irrelevant tokens
    if (this->lookahead->get_token() == MP_COMMENT
        || this->lookahead->get_token() == MP_RUN_COMMENT
        || this->lookahead->get_token() == MP_RUN_STRING) {
        while(this->lookahead->get_token() == MP_COMMENT
              || this->lookahead->get_token() == MP_RUN_COMMENT
              || this->lookahead->get_token() == MP_RUN_STRING) {
            this->lookahead = this->scanner->scan_one();
        }
    }
}

void Parser::parse() {
	// put the next token in the global buffer
	this->next_token();
	// parse the system goal!
	this->parse_system_goal();
}

void Parser::parse_system_goal() {
    this->more_indent();
    this->go_into(SYSTEM_GOAL);
	report_parse("PARSE_SYSTEM_GOAL", this->parse_depth);
	
    // parse system goal
    if (this->try_match(MP_PROGRAM)) {
        this->parse_program();
    }
    this->parse_eof();
    this->return_from();
    this->less_indent();
    
    // determine if there was an error...
    if (!this->error_reported) {
        // report success
        report_msg_type("Success", "Parse was successful! Awesome.");
    }
    else {
        // report failure
        report_msg_type("Failure", "Invalid parse. Yuck!");
        
        // terminate the program
        exit(0);
    }
}

void Parser::produce_code() {
    // generate code
    if (this->error_reported == false){
        this->analyzer->generate_all();
    } else {
        report_msg_type("Impossible", "Parse failed, code cannot be generated");
    }
}

void Parser::parse_eof() {
    this->more_indent();
    this->go_into(EOF_RULE);
	report_parse("PARSE_EOF", this->parse_depth);
    
    // ensure file ends with a newline
	if (!this->try_match(MP_EOF)) {
		report_error("Parse Error", "No end-of-file detected.\nMissing newline at end-of-file?");
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
    this->begin_generate_callable_part(true);
	this->parse_procedure_and_function_declaration_part();
    this->end_generate();
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
    if (this->try_match(MP_ID)) {
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
    this->begin_symbol(true);
	report_parse("PARSE_VARIABLE_DECL", this->parse_depth);
	this->parse_identifier_list();
	this->match(MP_COLON);
	this->parse_type();
    this->end_symbol(SYM_DATA, NO_CALL);
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
    this->begin_generate_callable_1(PROCEDURE, DEFINITION);
	this->parse_block();
    this->end_generate();
	this->match(MP_SEMI_COLON);
    this->analyzer->get_symtable()->return_from();
    this->return_from();
    this->less_indent();
}

void Parser::parse_function_declaration() {
    this->more_indent();
    this->go_into(FUNCTION_DECL);
	report_parse("PARSE_FUNCTION_DECL", this->parse_depth);
	this->parse_function_heading();
	this->match(MP_SEMI_COLON);
    this->begin_generate_callable_1(FUNCTION, DEFINITION);
	this->parse_block();
    this->end_generate();
	this->match(MP_SEMI_COLON);
    this->analyzer->get_symtable()->return_from();
    this->return_from();
    this->less_indent();
}

void Parser::parse_procedure_heading() {
    this->more_indent();
    this->go_into(PROCEDURE_HEAD);
    this->begin_symbol(false);
	report_parse("PARSE_PROCEDURE_HEADING", this->parse_depth);
	this->match(MP_PROCEDURE);
	this->parse_procedure_identifier();
	this->parse_optional_formal_parameter_list();
    this->end_symbol(SYM_CALLABLE, PROCEDURE);
    this->analyzer->get_symtable()->go_into();
    this->return_from();
    this->less_indent();
}

void Parser::parse_function_heading() {
    this->more_indent();
    this->go_into(FUNCTION_HEAD);
    this->begin_symbol(false);
	report_parse("PARSE_FUNCTION_HEADING", this->parse_depth);
	this->match(MP_FUNCTION);
	this->parse_function_identifier();
	this->parse_optional_formal_parameter_list();
    this->match(MP_COLON);
	this->parse_type();
    this->end_symbol(SYM_CALLABLE, FUNCTION);
    this->analyzer->get_symtable()->go_into();
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
    // try matching to all statement types
	if (this->try_match(MP_READ)) {
        this->begin_generate_io_action(IO_READ, false);
		this->parse_read_statement();
        this->end_generate();
    }
    else if (this->try_match(MP_READLN)) {
        this->begin_generate_io_action(IO_READ, true);
        this->parse_read_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_WRITE)) {
        this->begin_generate_io_action(IO_WRITE, false);
		this->parse_write_statement();
        this->end_generate();
    }
    else if (this->try_match(MP_WRITELN)) {
        this->begin_generate_io_action(IO_WRITE, true);
        this->parse_write_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_ID)) {
        this->begin_generate_assignment();
		this->parse_assignment_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_IF)) {
		this->parse_if_statement();
    }
	else if (this->try_match(MP_WHILE)) {
        this->begin_generate_loop(WHILELOOP);
		this->parse_while_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_REPEAT)) {
        this->begin_generate_loop(RPTUNTLLOOP);
		this->parse_repeat_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_FOR)) {
        this->begin_generate_loop(FORLOOP);
		this->parse_for_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_PROCEDURE)) {
        this->begin_generate_callable_2(PROCEDURE, CALL);
		this->parse_procedure_statement();
        this->end_generate();
    }
	else if (this->try_match(MP_BEGIN)) {
		this->parse_compound_statement();
    }
	else {
		this->parse_empty_statement();
    }
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
    if (this->try_match(MP_READ)) {
        this->match(MP_READ);
    }
    else if (this->try_match(MP_READLN)) {
        this->match(MP_READLN);
    }
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
    this->parse_variable_identifier();
    this->match(MP_ASSIGNMENT);
    this->parse_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_if_statement() {
    this->more_indent();
    this->go_into(IF_STATEMENT);
	report_parse("PARSE_IF_STATEMENT", this->parse_depth);
    ConditionalBlockPtr cond_if = this->begin_generate_if();
    this->match(MP_IF);
    this->parse_boolean_expression();
    this->match(MP_THEN);
    this->parse_statement();
    this->end_generate();
    if (this->try_match(MP_ELSE)) {
        ConditionalBlockPtr cond_else = this->begin_generate_opt_else();
        cond_else->set_connected(cond_if);
        cond_if->set_connected(cond_else);
        this->parse_optional_else_part();
        this->end_generate();
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
    this->match(MP_REPEAT);
    this->parse_statement_sequence();
    this->match(MP_UNTIL);
    this->parse_boolean_expression();
    this->return_from();
    this->less_indent();
}

void Parser::parse_while_statement() {
    this->more_indent();
    this->go_into(WHILE_STATEMENT);
	report_parse("PARSE_WHILE_STATEMENT", this->parse_depth);
    this->match(MP_WHILE);
    this->parse_boolean_expression();
    this->match(MP_DO);
    this->parse_statement();
    this->return_from();
    this->less_indent();
}

void Parser::parse_for_statement() {
    this->more_indent();
    this->go_into(FOR_STATEMENT);
	report_parse("PARSE_FOR_STATEMENT", this->parse_depth);
    this->match(MP_FOR);
    this->parse_control_variable();
    this->match(MP_ASSIGNMENT);
    this->parse_initial_value();
    this->parse_step_value();
    this->parse_final_value();
    this->match(MP_DO);
    this->parse_statement();
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
		report_error("Parse Error", "Invalid operator");
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
	} else {
		return false;
    }
}

bool Parser::is_multiplying_operator() {
	report_parse("IS_MULTIPLYING_OPERATOR", this->parse_depth);
	TokType lookahead_type = this->lookahead->get_token();
	if ((int) lookahead_type <= MP_MOD_KW
        && (int) lookahead_type >= MP_MULT) {
		return true;
	} else if ((int) lookahead_type == MP_AND) {
		return true;
    } else {
		return false;
    }
}

bool Parser::is_adding_operator() {
    report_parse("IS_ADDING_OPERATOR", this->parse_depth);
    TokType lookahead_type = this->lookahead->get_token();
    if ((int) lookahead_type == MP_PLUS
        || (int) lookahead_type == MP_MINUS
        || (int) lookahead_type == MP_OR) {
		return true;
    } else {
		return false;
    }
}

void Parser::more_indent() {
    this->parse_depth++;
}

void Parser::less_indent() {
    this->parse_depth--;
}

void Parser::next_token() {
    // get the next token from the dispatcher
    this->lookahead = this->scanner->scan_one();
}

void Parser::return_from() {
	this->analyzer->get_ast()->goto_parent();
}

void Parser::go_into(ParseType parse_type) {
	this->analyzer->get_ast()->add_move_child(AbstractNodePtr(new AbstractNode(parse_type)));
}

void Parser::go_into_lit(TokenPtr token) {
    this->analyzer->get_ast()->add_move_child(AbstractNodePtr(new AbstractNode(token)));
}

SemanticAnalyzerPtr Parser::get_analyzer() {
    return this->analyzer;
}

TokenPtr Parser::get_token() {
    return this->lookahead;
}

void Parser::begin_symbol(bool var_skip) {
    this->sym_collect = true;
    this->var_skip = var_skip;
}

VarType Parser::to_var(TokType token_type) {
    switch(token_type) {
        case MP_INTEGER:
            return INTEGER;
        case MP_STRING:
            return STRING;
        case MP_FLOAT:
            return FLOATING;
        case MP_BOOLEAN:
            return BOOLEAN;
        default:
            return VOID;
    }
}

void Parser::begin_generate_assignment() {
    AssignmentBlockPtr assign_block = AssignmentBlockPtr(new AssignmentBlock(false));
    this->get_analyzer()->append_block(assign_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Assignment Block");
}

void Parser::begin_generate_io_action(IOAction action, bool newline) {
    IOBlockPtr io_block = IOBlockPtr(new IOBlock(action, newline));
    this->get_analyzer()->append_block(io_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In IO Block");
}

void Parser::begin_generate_loop(LoopType loop) {
    LoopBlockPtr loop_block = LoopBlockPtr(new LoopBlock(loop));
    this->get_analyzer()->append_block(loop_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Loop Block");
}

ConditionalBlockPtr Parser::begin_generate_if() {
    ConditionalBlockPtr cond_block = ConditionalBlockPtr(new ConditionalBlock(COND_IF));
    this->get_analyzer()->append_block(cond_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In If Block");
    return cond_block;
}

ConditionalBlockPtr Parser::begin_generate_opt_else() {
    ConditionalBlockPtr cond_block = ConditionalBlockPtr(new ConditionalBlock(COND_ELSE));
    this->get_analyzer()->append_block(cond_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Else Block");
    return cond_block;
}

void Parser::begin_generate_callable_part(bool jump_around) {
    JumpBlockPtr jump = JumpBlockPtr(new JumpBlock(jump_around));
    this->get_analyzer()->append_block(jump);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Jump Block");
}

void Parser::begin_generate_callable_1(ActivationType activation, ActivityType activity) {
    SymCallablePtr last_callable = this->get_analyzer()->get_symtable()->get_last_callable();
    ActivationBlockPtr act_block = ActivationBlockPtr(new ActivationBlock(activation, activity, last_callable));
    last_callable->set_callable_definition(act_block);
    this->get_analyzer()->append_block(act_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Activation Block");
}

void Parser::begin_generate_callable_2(ActivationType activation, ActivityType activity) {
    ActivationBlockPtr act_block = ActivationBlockPtr(new ActivationBlock(activation, activity, nullptr));
    this->get_analyzer()->append_block(act_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Activation Call Block");
}

void Parser::begin_generate_callable(ActivationType activation, ActivityType activity, SymCallablePtr strecord) {
    ActivationBlockPtr act_block = ActivationBlockPtr(new ActivationBlock(activation, activity, strecord));
    this->get_analyzer()->append_block(act_block);
    this->begin_generate();
    if (DEBUG_OUTPUT)
    report_msg("In Activation Block");
}

void Parser::begin_generate() {
    this->gen_collect->push(0);
}

void Parser::end_generate() {
    this->gen_collect->pop();
    this->get_analyzer()->rappel_block();
    if (DEBUG_OUTPUT)
    report_msg("Out of Block");
}

void Parser::end_symbol(SymType symbol_type, ActivationType call_type) {
    this->sym_collect = false;
    // parse symbols into table
    
    // get the symbol table
    SymTablePtr table = this->get_analyzer()->get_symtable();
    
    // temporary symbol names
    vector<string> symbol_names = vector<string>();
    
    // symbol iteration
    TokenList::iterator symbol_iter = this->symbols->begin();
    TokenList::iterator symbol_end = this->symbols->end();
    
    // check to see if the record type is a callable type
    if (symbol_type == SYM_CALLABLE) {
        
        // get the callable info and return type
        string callable_name = (*this->symbols->begin())->get_lexeme();
        unsigned long row = (*this->symbols->begin())->get_line();
        unsigned long col = (*this->symbols->begin())->get_column();
        VarType return_type = VOID;
        
        // argument list pointer for addition to callable
        ArgumentListPtr argument_list = ArgumentListPtr(new ArgumentList());
        
        // return type is at end of function symbols
        if (call_type == FUNCTION) {
            // end is return type
            symbol_end = this->symbols->end() - 2;
            return_type = this->to_var((*(this->symbols->end() - 1))->get_token());
            if (this->symbols->size() == 2) {
                table->create_callable(callable_name, return_type, argument_list, row, col);
                symbol_names.clear();
                this->symbols->clear();
                return;
            }
        } else {
            // end is just another argument
            symbol_end = this->symbols->end();
            if (this->symbols->size() == 1) {
                // create function or procedure call
                table->create_callable(callable_name, return_type, argument_list, row, col);
                // reset
                symbol_names.clear();
                this->symbols->clear();
                return;
            }
        }
        // iterate through all of the symbols
        for (symbol_iter = this->symbols->begin() + 1;
             symbol_iter != symbol_end + 1; symbol_iter++) {
            // if we're looking at an ID, get its name
            if ((*symbol_iter)->get_token() == MP_ID) {
                // add the symbol lexeme to the list of arguments of the same type
                symbol_names.push_back((*symbol_iter)->get_lexeme());
            } else if ((*symbol_iter)->get_token() == MP_VAR) {
                // do nothing (no functions are pbr at the moment)
            } else {
                // assume type
                VarType var_type = this->to_var((*symbol_iter)->get_token());
                // turn all symbol names into arguments
                for (auto it = symbol_names.begin(); it != symbol_names.end(); it++) {
                    argument_list->push_back(table->create_argument(*it, var_type, VALUE));
                }
                // create function or procedure call
                table->create_callable(callable_name, return_type, argument_list, row, col);
                // reset
                symbol_names.clear();
            }
        }
        // clear and return
        this->symbols->clear();
        return;
    } else {
        for (symbol_iter = this->symbols->begin();
             symbol_iter != this->symbols->end(); symbol_iter++) {
            if ((*symbol_iter)->get_token() == MP_ID) {
                // add the symbol lexeme to the list of arguments of the same type
                symbol_names.push_back((*symbol_iter)->get_lexeme());
            } else if ((*symbol_iter)->get_token() == MP_VAR) {
                // do nothing (no functions are pbr at the moment)
            } else {
                // assume type
                VarType var_type = this->to_var((*symbol_iter)->get_token());
                // turn all symbol names into arguments
                for (auto it = symbol_names.begin(); it != symbol_names.end(); it++) {
                    unsigned long row = 0, col = 0;
                    for (auto it2 = this->symbols->begin(); it2 != this->symbols->end(); it2++) {
                        if ((*it2)->get_lexeme() == (*it)) {
                            row = (*it2)->get_line();
                            col = (*it2)->get_column();
                        }
                    }
                    // push back data
                    table->create_data(*it, var_type, row, col);
                }
            }
        }
        symbol_names.clear();
    }
    // clear the symbol buffer
    this->symbols->clear();
    return;
}
