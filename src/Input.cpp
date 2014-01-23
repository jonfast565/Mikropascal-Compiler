//
//  Input.cpp
//  Compiler
//
//  Created by Jonathan Fast on 1/17/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

// includes
#include "Input.hpp"

Input::Input(string filename, int line_alloc) {
	// input constructor
    // reopen file
    FILE* f = fopen(filename.c_str(), "r");
    // set up line capture (for getting file lines)
    this->lines = unique_ptr<vector<string>>(new vector<string>());
    char input_str[line_alloc];
    memset(&input_str, '\0', line_alloc * sizeof(char));
    // line capture
    while(fgets(input_str, line_alloc, f) != nullptr) {
    	this->lines->push_back(string(input_str));
    	memset(&input_str, '\0', line_alloc * sizeof(char));
    }
    // line concat
    this->entire_input = shared_ptr<string>(new string(""));
    for (vector<string>::iterator i = lines->begin(); i != lines->end(); ++i) {
    	for (string::iterator j = i->begin(); j != i->end(); ++j) {
    		this->entire_input->push_back(*j);
    	}
    }
    // no more file stuff
    fclose(f);
}

Input::~Input() {
	// destructor
    lines.reset();
    entire_input.reset();
}

shared_ptr<Input> Input::try_get_input(string filename) {
	// factory pattern, try to get the input
    // do this the good ol' C way
    FILE* f = nullptr;
    try {
        f = fopen(filename.c_str(), "r");
        if (f == nullptr)
            throw string("INVALID FILE");
        else {
            fclose(f);
            return shared_ptr<Input>(new Input(filename, DEFAULT_LINE_ALLOC));
        }
    } catch(std::string& exception) {
        fclose(f);
        return nullptr;
    }
}

shared_ptr<string> Input::detach_input() {
	// copy and return detached input
	return shared_ptr<string>(new string(*this->entire_input));
}

void Input::print_input() {
	// test print input
	cout << (*this->entire_input) << endl;
}
