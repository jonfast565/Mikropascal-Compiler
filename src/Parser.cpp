/*
 * Parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#include "Parser.hpp"

Parser::Parser(shared_ptr<vector<shared_ptr<Token>>> token_list) {
	this->token_list = token_list;
	this->fromList = true;
}

Parser::Parser(shared_ptr<Scanner> scanner) {
	this->scanner = scanner;
	this->fromList = false;
}

void Parser::parseMe() {
	if (this->fromList == false) {
		// parse by calling the scanner's dispatcher
	}
	else if (this->fromList == true) {
		// parse by pulling off a token from the beginning
		// of the list and attempting to parse it
	}
}

void Parser::match(TokType expected) {
	if (this->lookahead != expected) {
		cout << "Syntax error, expected " << get_token_info(expected).first <<
				" but got " << get_token_info(this->lookahead->get_token()).first << " instead. Fail!" << endl;
	} else {
		// consume the token and get the next
		if (this->fromList == false) {
			// get the next token from the dispatcher
			this->lookahead = this->scanner->scan_one();
		}
		else if (this->fromList == true) {
			// implement this later... with detaching token list
		}
	}
}

