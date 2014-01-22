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

class Token {
private:
	int line;
	int column;
	string token;
	string lexeme;
public:
	Token(int line, int column, string token, string lexeme) :
			line(line), column(column), token(token), lexeme(lexeme) {
	}
	;
	virtual ~Token() = default;
	void set_line(int line) {
		this->line = line;
	}
	void set_column(int column) {
		this->column = column;
	}
	void set_token(string token) {
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
	string get_token() {
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
	enum TokType {

	};
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> token_automata;
	shared_ptr<FiniteAutomataContainer> id_automata;
	shared_ptr<vector<shared_ptr<Token>>> found_tokens;
	shared_ptr<Input> input_ptr;
	shared_ptr<string> string_ptr;
	string::iterator file_ptr;
	string::iterator get_begin_fp();
	string::iterator get_end_fp();
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
