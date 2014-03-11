#ifndef rules_h
#define rules_h

enum ParseType {
	// parse for all Mikropascal non-terminals
	SYSTEM_GOAL,
	PROGRAM,
	PROGRAM_HEADING,
	// the program basically
	BLOCK,
	// variable declaration stuff
	VARIABLE_DECL_PART,
	VARIABLE_DECL_TAIL,
	VARIABLE_DECL,
	TYPE,
	// procedure and function declaration stuff
	PROCEDURE_AND_FUNCTION_DECL_PART,
	PROCEDURE_DECL,
	FUNCTION_DECL,
	PROCEDURE_HEAD,
	FUNCTION_HEAD,
	OPT_FORMAL_PARAM_LIST,
	FORMAL_PARAM_SECTION_TAIL,
	FORMAL_PARAM,
	VALUE_PARAM_SECTION,
	VARIABLE_PARAM_SECTION,
	// parse statements
	STATEMENT_PART,
	COMPOUND_STATEMENT,
	STATEMENT_SEQUENCE,
	STATEMENT_TAIL,
	STATEMENT,
	EMPTY_STATEMENT,
	READ_STATEMENT,
	READ_PARAMETER_TAIL,
	READ_PARAMETER,
	WRITE_STATEMENT,
	WRITE_PARAMETER_TAIL,
	WRITE_PARAMETER,
	ASSIGNMENT_STATEMENT,
	// parse control flow statements
	IF_STATEMENT,
	OPTIONAL_ELSE_PART,
	// parse looping statements
	REPEAT_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT,
	CONTROL_VARIABLE,
	INITIAL_VALUE,
	STEP_VALUE,
	FINAL_VALUE,
	// parse procedure statements
	PROCEDURE_STATEMENT,
	OPTIONAL_ACTUAL_PARAMETER_LIST,
	ACTUAL_PARAMETER_TAIL,
	ACTUAL_PARAMETER,
	// parse expressions
	EXPRESSION,
	OPTIONAL_RELATIONAL_PART,
	RELATIONAL_OPERATOR,
	SIMPLE_EXPRESSION,
	TERM_TAIL,
	OPTIONAL_SIGN,
	ADDING_OPERATOR,
	TERM,
	FACTOR_TAIL,
	MULTIPLYING_OPERATOR,
	FACTOR,
	// parse identifiers
	PROGRAM_IDENTIFIER,
	VARIABLE_IDENTIFIER,
	PROCEDURE_IDENTIFIER,
	FUNCTION_IDENTFIER,
	BOOLEAN_EXPRESSION,
	ORDINAL_EXPRESSION,
	IDENTFIER_LIST,
	IDENTFIER_TAIL,
	IDENTIFIER,
	// parse end of file
	EOF_RULE,
	NO_RULE,
	EPSILON,
	LITERAL
};

// token mapper (except digits, decimal digits, or ids)
static string get_rule_info(ParseType rule) {
	switch (rule) {
	// primitive tokens
	// parse for all Mikropascal non-terminals
	case SYSTEM_GOAL:
		return "SYSTEM_GOAL";
	case PROGRAM:
		return "PROGRAM";
	case PROGRAM_HEADING:
		return "PROGRAM_HEADING";
		// the program basically
	case BLOCK:
		return "BLOCK";
		// variable declaration stuff
	case VARIABLE_DECL_PART:
		return "VARIABLE_DECL_PART";
	case VARIABLE_DECL_TAIL:
		return "VARIABLE_DECL_TAIL";
	case VARIABLE_DECL:
		return "VARIABLE_DECL";
	case TYPE:
		return "TYPE";
		// procedure and function declaration stuff
	case PROCEDURE_AND_FUNCTION_DECL_PART:
		return "PROCEDURE_AND_FUNCTION_DECL_PART";
	case PROCEDURE_DECL:
		return "PROCEDURE_DECL";
	case FUNCTION_DECL:
		return "FUNCTION_DECL";
	case PROCEDURE_HEAD:
		return "PROCEDURE_HEAD";
	case FUNCTION_HEAD:
		return "FUNCTION_HEAD";
	case OPT_FORMAL_PARAM_LIST:
		return "OPT_FORMAL_PARAM_LIST";
	case FORMAL_PARAM_SECTION_TAIL:
		return "FORMAL_PARAM_SECTION_TAIL";
	case FORMAL_PARAM:
		return "FORMAL_PARAM";
	case VALUE_PARAM_SECTION:
		return "VALUE_PARAM_SECTION";
	case VARIABLE_PARAM_SECTION:
		return "VARIABLE_PARAM_SECTION";
		// parse statements
	case STATEMENT_PART:
		return "STATEMENT_PART";
	case COMPOUND_STATEMENT:
		return "COMPOUND_STATEMENT";
	case STATEMENT_SEQUENCE:
		return "STATEMENT_SEQUENCE";
	case STATEMENT_TAIL:
		return "STATEMENT_TAIL";
	case STATEMENT:
		return "STATEMENT";
	case EMPTY_STATEMENT:
		return "EMPTY_STATEMENT";
	case READ_STATEMENT:
		return "READ_STATEMENT";
	case READ_PARAMETER_TAIL:
		return "READ_PARAMETER_TAIL";
	case READ_PARAMETER:
		return "READ_PARAMETER";
	case WRITE_STATEMENT:
		return "WRITE_STATEMENT";
	case WRITE_PARAMETER_TAIL:
		return "WRITE_PARAMETER_TAIL";
	case WRITE_PARAMETER:
		return "WRITE_PARAMETER";
	case ASSIGNMENT_STATEMENT:
		return "ASSIGNMENT_STATEMENT";
		// parse control flow statements
	case IF_STATEMENT:
		return "IF_STATEMENT";
	case OPTIONAL_ELSE_PART:
		return "OPTIONAL_ELSE_PART";
		// parse looping statements
	case REPEAT_STATEMENT:
		return "REPEAT_STATEMENT";
	case WHILE_STATEMENT:
		return "WHILE_STATEMENT";
	case FOR_STATEMENT:
		return "FOR_STATEMENT";
	case CONTROL_VARIABLE:
		return "CONTROL_VARIABLE";
	case INITIAL_VALUE:
		return "INITIAL_VALUE";
	case STEP_VALUE:
		return "STEP_VALUE";
	case FINAL_VALUE:
		return "FINAL_VALUE";
		// parse procedure statements
	case PROCEDURE_STATEMENT:
		return "PROCEDURE_STATEMENT";
	case OPTIONAL_ACTUAL_PARAMETER_LIST:
		return "OPTIONAL_ACTUAL_PARAMETER_LIST";
	case ACTUAL_PARAMETER_TAIL:
		return "ACTUAL_PARAMETER_TAIL";
	case ACTUAL_PARAMETER:
		return "ACTUAL_PARAMETER";
		// parse expressions
	case EXPRESSION:
		return "EXPRESSION";
	case OPTIONAL_RELATIONAL_PART:
		return "OPTIONAL_RELATIONAL_PART";
	case RELATIONAL_OPERATOR:
		return "RELATIONAL_OPERATOR";
	case SIMPLE_EXPRESSION:
		return "SIMPLE_EXPRESSION";
	case TERM_TAIL:
		return "TERM_TAIL";
	case OPTIONAL_SIGN:
		return "OPTIONAL_SIGN";
	case ADDING_OPERATOR:
		return "ADDING_OPERATOR";
	case TERM:
		return "TERM";
	case FACTOR_TAIL:
		return "FACTOR_TAIL";
	case MULTIPLYING_OPERATOR:
		return "MULTIPLYING_OPERATOR";
	case FACTOR:
		return "FACTOR";
		// parse identifiers
	case PROGRAM_IDENTIFIER:
		return "PROGRAM_IDENTIFIER";
	case VARIABLE_IDENTIFIER:
		return "VARIABLE_IDENTIFIER";
	case PROCEDURE_IDENTIFIER:
		return "PROCEDURE_IDENTIFIER";
	case FUNCTION_IDENTFIER:
		return "FUNCTION_IDENTFIER";
	case BOOLEAN_EXPRESSION:
		return "BOOLEAN_EXPRESSION";
	case ORDINAL_EXPRESSION:
		return "ORDINAL_EXPRESSION";
	case IDENTFIER_LIST:
		return "IDENTFIER_LIST";
	case IDENTFIER_TAIL:
		return "IDENTFIER_TAIL";
	case IDENTIFIER:
		return "IDENTIFIER";
		// parse end of file
	case EOF_RULE:
		return "EOF_RULE";
	case NO_RULE:
		return "NO_RULE";
	case EPSILON:
		return "EPSILON";
	case LITERAL:
		return "LITERAL";
	default:
		return "NO_PARSE";
	}
}

#endif
