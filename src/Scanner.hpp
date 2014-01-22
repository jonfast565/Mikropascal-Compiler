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

class Token {
private:
	int line;
	int column;
	string token;
	string lexeme;
public:
	Token(int line, int column, string token, string lexeme) :
		line(line), column(column), token(token), lexeme(lexeme){};
	~Token() = default;
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
		return string(this->get_token() + " " + this->get_line() + " " +
				this->get_column() + " " + this->get_lexeme());
	}
};

class Scanner {
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> token_automata;

};

#endif
