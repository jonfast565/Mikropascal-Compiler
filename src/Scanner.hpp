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

	// identifier
	MP_ID,

	// comment brackets
	MP_BRACKET_LEFT,
	MP_BRACKET_RIGHT,

	// end of file
	MP_NULLCHAR,

	//ignorable
	MP_COMMENT,
	MP_MALFORMED
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
	case MP_EQUALS:
		return pair<string, string>("MP_EQUALS", "=");

		// boolean comparison tokens
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
    case MP_COMMENT:
        return pair<string, string>("MP_COMMENT", "");
    case MP_ID:
        return pair<string, string>("MP_ID", "");
    case MP_INT_LITERAL:
        return pair<string, string>("MP_INT_LITERAL", "");
    case MP_FLOAT_LITERAL:
        return pair<string, string>("MP_FLT_LITERAL", "");
    case MP_MALFORMED:
        return pair<string, string>("MP_MALFORMED", "");
	default:
		return pair<string, string>("", "");
	}
}

// inverse token mapper, get token from a string
static TokType get_token_by_name(string name) {
	for (auto i = TokType::MP_SEMI_COLON; i <= TokType::MP_BOOLEAN; i++) {
		if (get_token_info(i).first.compare(name) == 0) {
			return (TokType) i;
		}
	}
	// error
	return (TokType) 0;
}

// Token class
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
		ss << setw(25) << left << get_token_info(this->get_token()).first
        << setw(10) << left << this->get_line()
		<< setw(10) << left << this->get_column()
        << setw(30) << left << string("'" + this->get_lexeme() + "'");
		return shared_ptr<string>(new string(ss.str()));
	}
};

class Scanner {
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> keyword_automata;
	shared_ptr<FiniteAutomataContainer> id_automata;
	shared_ptr<FiniteAutomataContainer> int_literal_automata;
	shared_ptr<FiniteAutomataContainer> float_literal_automata;
	shared_ptr<vector<shared_ptr<Token>>> found_tokens;
	shared_ptr<Input> input_ptr;
	shared_ptr<string> string_ptr;
	shared_ptr<vector<char>> scan_buf;
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
	// kword ops
	void load_keyword_automata();
	void reset_all_kword_automata();
	void step_all_kword(char next);
	bool check_any_kword_accepted();
	TokType get_first_accepted_kword();
	pair<TokType, vector<char>> quick_lookahead_kword(TokType old, vector<string> looking_for);
	// ld ops
	bool isalnum(char next);
	bool isnum(char next);
	bool isalpha(char next);
	// id ops
	void load_id_automata();
	bool check_id_accepted();
	void step_id(char next);
	void reset_id();
	// numerical automata ops
	void load_numerical_automata();
	bool check_int_accepted();
    bool check_flt_accepted();
	void step_int(char next);
	void reset_int();
    void step_flt(char next);
    void reset_flt();
	// scanner ops
	shared_ptr<Token> scan_one();
	shared_ptr<Token> scan_keyword_or_id();
	shared_ptr<Token> scan_num();
	shared_ptr<Token> scan_line_comment();
	shared_ptr<Token> scan_bracket_comment();
	shared_ptr<Token> scan_string_literal();
	void scan_all();
	char peek();
	char next();
	bool right();
	bool right_two();
	bool left();
	// line and col numbers
	int get_line_number();
	int get_col_number();
	void set_line_number(int new_line_number);
	void set_col_number(int new_col_number);
	// display tokens on the screen
	void display_tokens();
	void display_all_automata();
	// file ops
	void write_tokens_tof(string filename);
};

#endif
