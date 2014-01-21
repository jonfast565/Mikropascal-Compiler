//
//  Input.hpp
//  Compiler
//
//  Created by Jonathan Fast on 1/17/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#ifndef input
#define input

// use standard includes
#include "Standard.hpp"
#define DEFAULT_LINE_ALLOC 10000

// input structure
class Input {
private:
    // input in lines and string
	shared_ptr<vector<string>> lines;
	shared_ptr<string> resolved;
    Input(string filename, int line_alloc);
public:
    virtual ~Input();
    static Input* try_get_input(string filename);
    void print_input();
};

class Token {
private:
	int line_number;
	int column;
	string name;
	char value;
};

#endif
