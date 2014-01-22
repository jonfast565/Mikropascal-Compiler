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
	shared_ptr<string> entire_input;
    Input(string filename, int line_alloc);
public:
    virtual ~Input();
    static shared_ptr<Input> try_get_input(string filename);
    shared_ptr<string> detach_input();
    void print_input();
};

#endif
