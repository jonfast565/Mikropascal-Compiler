/*
 * Scanner.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#ifndef scan
#define scan

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
		line(line), column(column), token(token), lexeme(lexeme){};
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
	string to_string() {
		stringstream ss;
		ss << this->get_token() << " " << this->get_line() << " " <<
				this->get_column() << " " << this->get_lexeme();
		return ss.str();
	}
};

class Scanner {
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> token_automata;
	shared_ptr<Input> input_ptr;
	shared_ptr<string> string_ptr;
	string::iterator file_ptr;
	string::iterator get_begin_fp();
	string::iterator get_end_fp();
public:
	Scanner(shared_ptr<Input> input_ptr);
	virtual ~Scanner();

};

#endif
