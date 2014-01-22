/*
 * Scanner.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#include "Scanner.hpp"

Scanner::Scanner(shared_ptr<Input> input_ptr) {
	this->input_ptr = input_ptr;
	this->string_ptr = input_ptr->detach_input();
	this->token_automata = shared_ptr<
			vector<shared_ptr<FiniteAutomataContainer>>>(new vector<shared_ptr<FiniteAutomataContainer>>);
	this->col_number = 0;
	this->line_number = 0;
}

Scanner::~Scanner() {
	this->input_ptr.reset();
	this->string_ptr.reset();
}

string::iterator Scanner::get_begin_fp() {
	return this->string_ptr->begin();
}

string::iterator Scanner::get_end_fp() {
	return this->string_ptr->end();
}

void Scanner::set_fp_begin() {
	this->file_ptr = this->string_ptr->begin();
}

void Scanner::reset() {
	// reset all FSAs, delete all tokens, move file pointer to beginning
}

void Scanner::scan() {
	// tokenize the input until done
	// initialize a vector to hold all token characters
	vector<char> scan_buf;

	// begin the scanning loop
	while(true) {

		// get the next input char
		char curchar = this->peek();
		char nexchar = this->next();

		// basic cases for comments, whitespace, and newlines
		// remove whitespace
		if (curchar == ' ') {
			// ignore
			this->right();
		}
		// check for a new line
		else if (curchar == '\n') {
			// ignore, managed in right() and left()
			this->right();
		}
		// remove double dash comments
		else if (curchar == '/' && (nexchar == '/' && nexchar != '\0')) {
			while(this->next() != '\n')
				this->right();
		}
		// remove bracket comments
		else if (curchar == '{') {
			while(this->next() != '}')
				this->right();
		}
		// now look for tokens
		else {

		}
	}

	// string conv_tok = string(scan_buf.begin(),scan_buf.end());
}

char Scanner::peek() {
	// look at the current char in the input
	if (this->file_ptr != this->string_ptr->end())
		// if not end, character is valid
		return *this->file_ptr;
	else
		return '\0';
}
char Scanner::next() {
	// look at the next char in the input, without consuming
	if (this->right()) {
		// get the next character
		char lookahead = this->peek();
		// go back
		this->left();
		// return this character
		return lookahead;
	} else return '\0';

}
bool Scanner::right() {
	// move the file pointer to the right by one
	if (this->file_ptr != this->string_ptr->end()) {
		// deal with line and column numbers
		if (*this->file_ptr == '\n') {
			this->line_number++;
			this->col_number = 1;
		} else {
			this->col_number++;
		}
		// successful
		this->file_ptr++;
		return true;
	} else
		// unsuccessful
		return false;
}

bool Scanner::left() {
	// move the file pointer to the left by one
	if (this->file_ptr != this->string_ptr->begin()) {
		// deal with line and column numbers
		if (*this->file_ptr == '\n') {
			this->line_number--;
			// determine the length of the last line
			// unimplemented, expect shady results
			// from the line below...
			this->col_number = 1;
		} else {
			this->col_number--;
		}
		// successful
		this->file_ptr--;
		return true;
	} else
		// unsuccessful
		return false;
}

void Scanner::set_line_number(int new_line_number) {
	this->line_number = new_line_number;
}

void Scanner::set_col_number(int new_col_number) {
	this->col_number = new_col_number;
}

void Scanner::write_tof(string filename) {

}

