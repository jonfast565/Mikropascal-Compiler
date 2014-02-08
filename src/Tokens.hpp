/*
 * Tokens.hpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#ifndef TOKENS_HPP_
#define TOKENS_HPP_

#include "Standard.hpp"

// Mikropascal Token Types
enum TokType {

	// primitive tokens
	MP_SEMI_COLON,
	MP_COLON,
	MP_COMMA,
	MP_ASSIGNMENT,
	MP_LEFT_PAREN,
	MP_RIGHT_PAREN,
	MP_PERIOD,

	// program info tokens
	MP_PROGRAM,
	MP_PROCEDURE,
	MP_FUNCTION,
	MP_VAR,
	MP_BEGIN,
	MP_END,

	// io tokens
	MP_READ,
	MP_READLN,
	MP_WRITE,
	MP_WRITELN,

	// loop tokens
	MP_REPEAT,
	MP_UNTIL,
	MP_WHILE,
	MP_DO,
	MP_FOR,
	MP_TO,
	MP_DOWNTO,

	// control flow tokens
	MP_IF,
	MP_THEN,
	MP_ELSE,
	MP_EQUALS,

	// boolean comparison tokens
	MP_LESSTHAN,
	MP_GREATERTHAN,
	MP_GREATERTHAN_EQUALTO,
	MP_LESSTHAN_EQUALTO,
	MP_NOT_EQUAL,

	// boolean (logical) operator tokens
	MP_OR,
	MP_AND,
	MP_NOT,

	// boolean result tokens
	MP_TRUE,
	MP_FALSE,

	// operator tokens
	MP_PLUS,
	MP_MINUS,
	MP_MULT,
	MP_DIV,

	// multi character operator tokens
	MP_DIV_KW,
	MP_MOD_KW,

	// type declaration tokens
	MP_INTEGER,
	MP_FLOAT,
	MP_STRING,
	MP_BOOLEAN,

	// numeric literals
	MP_INT_LITERAL,
	MP_FLOAT_LITERAL,

	// literal for string
	MP_STRING_LITERAL,
	MP_RUN_STRING,

	// identifier
	MP_ID,

	// comment brackets
	MP_BRACKET_LEFT,
	MP_BRACKET_RIGHT,

	// end of file
	MP_NULLCHAR,
	MP_EOF,

	//ignorable
	MP_COMMENT,
	MP_RUN_COMMENT,
	MP_MALFORMED,
	MP_ERROR
};

// token mapper (except digits, decimal digits, or ids)
static pair<string, string> get_token_info(TokType token) {
	switch (token) {

	// primitive tokens
	case MP_SEMI_COLON:
		return pair<string, string>("MP_SEMI_COLON", ";");
	case MP_COLON:
		return pair<string, string>("MP_COLON", ":");
	case MP_COMMA:
		return pair<string, string>("MP_COMMA", ",");
	case MP_ASSIGNMENT:
		return pair<string, string>("MP_ASSIGNMENT", ":=");
	case MP_LEFT_PAREN:
		return pair<string, string>("MP_LEFT_PAREN", "(");
	case MP_RIGHT_PAREN:
		return pair<string, string>("MP_RIGHT_PAREN", ")");
	case MP_PERIOD:
		return pair<string, string>("MP_PERIOD", ".");

		// program info tokens
	case MP_PROGRAM:
		return pair<string, string>("MP_PROGRAM", "program");
	case MP_PROCEDURE:
		return pair<string, string>("MP_PROCEDURE", "procedure");
	case MP_FUNCTION:
		return pair<string, string>("MP_FUNCTION", "function");
	case MP_VAR:
		return pair<string, string>("MP_VAR", "var");
	case MP_BEGIN:
		return pair<string, string>("MP_BEGIN", "begin");
	case MP_END:
		return pair<string, string>("MP_END", "end");

		// io tokens
	case MP_READ:
		return pair<string, string>("MP_READ", "read");
	case MP_READLN:
		return pair<string, string>("MP_READLN", "readln");
	case MP_WRITE:
		return pair<string, string>("MP_WRITE", "write");
	case MP_WRITELN:
		return pair<string, string>("MP_WRITELN", "writeln");

		// loop tokens
	case MP_REPEAT:
		return pair<string, string>("MP_REPEAT", "repeat");
	case MP_UNTIL:
		return pair<string, string>("MP_UNTIL", "until");
	case MP_WHILE:
		return pair<string, string>("MP_WHILE", "while");
	case MP_DO:
		return pair<string, string>("MP_DO", "do");
	case MP_FOR:
		return pair<string, string>("MP_FOR", "for");
	case MP_TO:
		return pair<string, string>("MP_TO", "to");
	case MP_DOWNTO:
		return pair<string, string>("MP_DOWNTO", "downto");

		// control flow tokens
	case MP_IF:
		return pair<string, string>("MP_IF", "if");
	case MP_THEN:
		return pair<string, string>("MP_THEN", "then");
	case MP_ELSE:
		return pair<string, string>("MP_ELSE", "else");

		// boolean comparison tokens
	case MP_EQUALS:
		return pair<string, string>("MP_EQUALS", "=");
	case MP_LESSTHAN:
		return pair<string, string>("MP_LESSTHAN", "<");
	case MP_GREATERTHAN:
		return pair<string, string>("MP_GREATERTHAN", ">");
	case MP_GREATERTHAN_EQUALTO:
		return pair<string, string>("MP_GREATERTHAN_EQUALTO", ">=");
	case MP_LESSTHAN_EQUALTO:
		return pair<string, string>("MP_LESSTHAN_EQUALTO", "<=");
	case MP_NOT_EQUAL:
		return pair<string, string>("MP_NOT_EQUAL", "<>");

		// boolean (logical) operator tokens
	case MP_OR:
		return pair<string, string>("MP_OR", "or");
	case MP_AND:
		return pair<string, string>("MP_AND", "and");
	case MP_NOT:
		return pair<string, string>("MP_NOT", "not");

		// boolean result tokens
	case MP_TRUE:
		return pair<string, string>("MP_TRUE", "true");
	case MP_FALSE:
		return pair<string, string>("MP_FALSE", "false");

		// operator tokens
	case MP_PLUS:
		return pair<string, string>("MP_PLUS", "+");
	case MP_MINUS:
		return pair<string, string>("MP_MINUS", "-");
	case MP_MULT:
		return pair<string, string>("MP_MULT", "*");
	case MP_DIV:
		return pair<string, string>("MP_DIV", "/");

		// multi character operator tokens
	case MP_DIV_KW:
		return pair<string, string>("MP_DIV_KW", "div");
	case MP_MOD_KW:
		return pair<string, string>("MP_MOD_KW", "mod");

		// type declaration tokens
	case MP_INTEGER:
		return pair<string, string>("MP_INTEGER", "integer");
	case MP_FLOAT:
		return pair<string, string>("MP_FLOAT", "float");
	case MP_STRING:
		return pair<string, string>("MP_STRING", "string");
	case MP_BOOLEAN:
		return pair<string, string>("MP_BOOLEAN", "boolean");
	case MP_BRACKET_LEFT:
		return pair<string, string>("MP_BRACKET_LEFT", "{");
	case MP_BRACKET_RIGHT:
		return pair<string, string>("MP_BRACKET_RIGHT", "}");

		// others
	case MP_NULLCHAR:
		return pair<string, string>("MP_NULLCHAR", "\0");
	case MP_EOF:
		return pair<string, string>("MP_EOF", "EOF");
	case MP_COMMENT:
		return pair<string, string>("MP_COMMENT", "");
	case MP_ID:
		return pair<string, string>("MP_ID", "");
	case MP_INT_LITERAL:
		return pair<string, string>("MP_INT_LITERAL", "");
	case MP_FLOAT_LITERAL:
		return pair<string, string>("MP_FLT_LITERAL", "");
	case MP_STRING_LITERAL:
		return pair<string, string>("MP_STR_LITERAL", "'");
	case MP_RUN_STRING:
		return pair<string, string>("MP_RUN_STRING", "'");
	case MP_RUN_COMMENT:
		return pair<string, string>("MP_RUN_COMMENT", "'");
	case MP_MALFORMED:
		return pair<string, string>("MP_MALFORMED", "");
	case MP_ERROR:
		return pair<string, string>("MP_ERROR", "");
	default:
		return pair<string, string>("", "");
	}
}

// inverse token mapper, get token from a string
// it is used, don't let it fool you that it isn't!!!
static TokType get_token_by_name(string name) {
	for (auto i = (int)TokType::MP_SEMI_COLON; i <= (int)TokType::MP_BOOLEAN; i++) {
		if (get_token_info((TokType) i).first.compare(name) == 0) {
			return (TokType) i;
		}
	}
	// error
	return MP_MALFORMED;
}

// Token class
class Token {
private:
	unsigned int line;
	unsigned int column;
	TokType token;
	string lexeme;
public:
	Token(int line, int column, TokType token, string lexeme) :
			line(line), column(column), token(token), lexeme(lexeme) {
	}
	virtual ~Token() = default;
	void set_line(unsigned int line) {
		this->line = line;
	}
	void set_column(unsigned int column) {
		this->column = column;
	}
	void set_token(TokType token) {
		this->token = token;
	}
	void set_lexeme(string lexeme) {
		this->lexeme = lexeme;
	}
	unsigned int get_line() {
		return this->line;
	}
	unsigned int get_column() {
		return this->column;
	}
	TokType get_token() {
		return this->token;
	}
	string get_lexeme() {
		return this->lexeme;
	}
	shared_ptr<string> to_string() {
		stringstream ss;
		if (this->token == TokType::MP_MALFORMED
				|| this->token == TokType::MP_RUN_COMMENT
				|| this->token == TokType::MP_RUN_STRING
				|| this->token == TokType::MP_ERROR) {
			ss << std::left << setw(76) << string(this->get_lexeme());
		} else {
			ss << setw(25) << std::left
					<< get_token_info(this->get_token()).first << setw(10)
					<< std::left << this->get_line() << setw(10) << std::left
					<< (this->get_column() - this->get_lexeme().size())
					<< setw(30) << std::left
					<< string("'" + this->get_lexeme() + "'");
		}
		return shared_ptr<string>(new string(ss.str()));
	}
};

#endif /* TOKENS_HPP_ */
