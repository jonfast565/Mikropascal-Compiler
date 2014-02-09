//
//  Input.hpp
//  Compiler
//
//  Created by Jonathan Fast on 1/17/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#ifndef input_h
#define input_h

// use standard includes
#include "Standard.hpp"

// input structure
class Input {
private:
	// input in lines and string
	shared_ptr<vector<string>> lines;
	shared_ptr<string> entire_input;
	Input(string filename);
public:
	virtual ~Input();
	static shared_ptr<Input> open_file(string filename);
	shared_ptr<string> detach_input();
	void print_input();
};

#endif
