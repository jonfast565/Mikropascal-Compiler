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
#include "Helper.hpp"

class Input;
using InputPtr = shared_ptr<Input>;
using StringList = vector<string>;
using StringListPtr = shared_ptr<StringList>;

// input structure
class Input {
private:
	// input in lines and string
	StringListPtr lines;
	StringPtr entire_input;
	Input(string filename);
public:
	virtual ~Input() = default;
	static InputPtr open_file(string filename);
	StringPtr detach_input();
	void print_input();
};

#endif
