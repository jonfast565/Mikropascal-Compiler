//
//  Input.cpp
//  Compiler
//
//  Created by Jonathan Fast on 1/17/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

// includes
#include "Input.hpp"

// input structures
Input::Input(string filename, int line_alloc) {
    
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
    this->resolved = shared_ptr<string>(new string(""));
    for (vector<string>::iterator i = lines->begin(); i != lines->end(); ++i) {
    	for (string::iterator j = i->begin(); j != i->end(); ++j) {
    		this->resolved->push_back(*j);
    	}
    }

    // no more file stuff
    fclose(f);
}

// factory pattern, try to get the input
Input* Input::try_get_input(string filename) {
    // do this the good ol' C way
    FILE* f = nullptr;
    try {
        f = fopen(filename.c_str(), "r");
        if (f == nullptr)
            throw string("INVALID FILE");
        else {
            fclose(f);
            return new Input(filename, DEFAULT_LINE_ALLOC);
        }
    } catch(std::string& exception) {
        fclose(f);
        return nullptr;
    }
}

// destructor
Input::~Input() {
    lines.reset();
    resolved.reset();
}

void Input::print_input() {
	cout << this->resolved << endl;
}
