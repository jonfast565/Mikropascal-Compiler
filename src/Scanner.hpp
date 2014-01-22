/*
 * Scanner.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#ifndef scanner_h
#define scanner_h

#include "Standard.hpp"
#include "Input.hpp"
#include "FiniteAutomata.hpp"

// Mikropascal Token Types
enum TokType {

	// primitive tokens
	MP_SEMI_COLON,
	MP_COLON,
	MP_COMMA,
	MP_ASSIGNMENT,
	MP_LEFT_PAREN,
	MP_RIGHT_PAREN,

	// program info tokens
	MP_PROGRAM,
	MP_PROCEDURE,
	MP_FUNCTION,
	MP_VAR,
	MP_BEGIN,
	MP_END,

	// io tokens
	MP_READ,
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
	MP_DIGITS,
	MP_DECIMAL_DIGITS,

	// identifier
	MP_ID
};

// token mapper (except digits, decimal digits, or ids)
static pair<string, string> get_names(TokType token) {
	switch (token) {
	// primitive tokens

	case MP_SEMI_COLON:
		return pair<string, string>("MP_SEMI_COLON", string(";"));
	case MP_COLON:
		return pair<string, string>("MP_COLON", string(":"));
	case MP_COMMA:
		return pair<string, string>("MP_COMMA", string(","));
	case MP_ASSIGNMENT:
		return pair<string, string>("MP_ASSIGNMENT", string(":="));
	case MP_LEFT_PAREN:
		return pair<string, string>("MP_LEFT_PAREN", string("("));
	case MP_RIGHT_PAREN:
		return pair<string, string>("MP_RIGHT_PAREN", string(")"));

		// program info tokens
	case MP_PROGRAM:
		return pair<string, string>("MP_PROGRAM", string("program"));
	case MP_PROCEDURE:
		return pair<string, string>("MP_PROCEDURE", string("procedure"));
	case MP_FUNCTION:
		return pair<string, string>("MP_FUNCTION", string("function"));
	case MP_VAR:
		return pair<string, string>("MP_VAR", string("var"));
	case MP_BEGIN:
		return pair<string, string>("MP_BEGIN", string("begin"));
	case MP_END:
		return pair<string, string>("MP_END", string("end"));

		// io tokens
	case MP_READ:
		return pair<string, string>("MP_READ", string("read"));
	case MP_WRITE:
		return pair<string, string>("MP_WRITE", string("write"));
	case MP_WRITELN:
		return pair<string, string>("MP_WRITELN", string("writeln"));

		// loop tokens
	case MP_REPEAT:
		return pair<string, string>("MP_REPEAT", string("repeat"));
	case MP_UNTIL:
		return pair<string, string>("MP_UNTIL", string("until"));
	case MP_WHILE:
		return pair<string, string>("MP_WHILE", string("while"));
	case MP_DO:
		return pair<string, string>("MP_DO", string("do"));
	case MP_FOR:
		return pair<string, string>("MP_FOR", string("for"));
	case MP_TO:
		return pair<string, string>("MP_TO", string("to"));
	case MP_DOWNTO:
		return pair<string, string>("MP_DOWNTO", string("downto"));

		// control flow tokens
	case MP_IF:
		return pair<string, string>("MP_IF", string("if"));
	case MP_THEN:
		return pair<string, string>("MP_THEN", string("then"));
	case MP_ELSE:
		return pair<string, string>("MP_ELSE", string("else"));
	case MP_EQUALS:
		return pair<string, string>("MP_EQUALS", string("="));

		// boolean comparison tokens
	case MP_LESSTHAN:
		return pair<string, string>("MP_LESSTHAN", string("<"));
	case MP_GREATERTHAN:
		return pair<string, string>("MP_GREATERTHAN", string(">"));
	case MP_GREATERTHAN_EQUALTO:
		return pair<string, string>("MP_GREATERTHAN_EQUALTO", string(">="));
	case MP_LESSTHAN_EQUALTO:
		return pair<string, string>("MP_LESSTHAN_EQUALTO", string("<="));
	case MP_NOT_EQUAL:
		return pair<string, string>("MP_NOT_EQUAL", string("<>"));

		// boolean (logical) operator tokens
	case MP_OR:
		return pair<string, string>("MP_OR", string("or"));
	case MP_AND:
		return pair<string, string>("MP_AND", string("and"));
	case MP_NOT:
		return pair<string, string>("MP_NOT", string("not"));

		// boolean result tokens
	case MP_TRUE:
		return pair<string, string>("MP_TRUE", string("true"));
	case MP_FALSE:
		return pair<string, string>("MP_FALSE", string("false"));

		// operator tokens
	case MP_PLUS:
		return pair<string, string>("MP_PLUS", string("+"));
	case MP_MINUS:
		return pair<string, string>("MP_MINUS", string("-"));
	case MP_MULT:
		return pair<string, string>("MP_MULT", string("*"));
	case MP_DIV:
		return pair<string, string>("MP_DIV", string("/"));

		// multi character operator tokens
	case MP_DIV_KW:
		return pair<string, string>("MP_DIV_KW", string("div"));
	case MP_MOD_KW:
		return pair<string, string>("MP_MOD_KW", string("mod"));

		// type declaration tokens
	case MP_INTEGER:
		return pair<string, string>("MP_INTEGER", string("integer"));
	case MP_FLOAT:
		return pair<string, string>("MP_FLOAT", string("float"));
	case MP_STRING:
		return pair<string, string>("MP_STRING", string("string"));
	case MP_BOOLEAN:
		return pair<string, string>("MP_BOOLEAN", string("boolean"));
	default:
		return pair<string, string>(string(), string());
	}
}

class Token {
private:
	int line;
	int column;
	TokType token;
	string lexeme;
public:
	Token(int line, int column, TokType token, string lexeme) :
			line(line), column(column), token(token), lexeme(lexeme) {
	}
	virtual ~Token() = default;
	void set_line(int line) {
		this->line = line;
	}
	void set_column(int column) {
		this->column = column;
	}
	void set_token(TokType token) {
		this->token = token;
	}
	void set_lexeme(string lexeme) {
		this->lexeme = lexeme;
	}
	int get_line() {
		return this->line;
	}
	int get_column() {
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
		ss << this->get_token() << " " << this->get_line() << " "
				<< this->get_column() << " " << this->get_lexeme();
		return shared_ptr<string>(new string(ss.str()));
	}
};

class Scanner {
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> token_automata;
	shared_ptr<FiniteAutomataContainer> id_automata;
	shared_ptr<vector<shared_ptr<Token>>> found_tokens;
	shared_ptr<Input> input_ptr;
	shared_ptr<string> string_ptr;
	string::iterator file_ptr;
	string::iterator get_begin_fp();
	string::iterator get_end_fp();
	void set_fp_begin();
	int line_number;
	int col_number;
public:
	Scanner(shared_ptr<Input> input_ptr);
	virtual ~Scanner();
	void reset();
	void scan();
	char peek();
	char next();
	bool right();
	bool left();
	int get_line_number();
	int get_col_number();
	void set_line_number(int new_line_number);
	void set_col_number(int new_col_number);
	void write_tof(string filename);
};

#endif
