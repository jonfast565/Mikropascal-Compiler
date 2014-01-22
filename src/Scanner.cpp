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

