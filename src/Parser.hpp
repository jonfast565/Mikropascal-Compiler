/*
 * Parser.hpp
 *
 *  Created on: Feb 7, 2014
 *      Author: jonfast
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "Standard.hpp"
#include "Tokens.hpp"

class Parser {
private:
	shared_ptr<vector<shared_ptr<Token>>> token_list;
	shared_ptr<Scanner> scanner;
	shared_ptr<Token> lookahead;
	bool fromList;
public:
	Parser(shared_ptr<vector<shared_ptr<Token>>> token_list);
	Parser(shared_ptr<Scanner> scanner);
	void parseMe();
	void match(TokType token_type);
	virtual ~Parser() {
		if (scanner != nullptr)
			scanner.reset();
		if (token_list != nullptr)
			token_list.reset();
	}
};


#endif /* PARSER_HPP_ */
