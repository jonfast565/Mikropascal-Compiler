//
//  Input.cpp
//  Compiler
//
//  Created by Jonathan Fast on 1/17/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

// includes
#include "Input.hpp"

Input::Input(string filename) {
	// input constructor
	// set up line capture (for getting file lines)
    string line;
	this->lines = StringListPtr(new StringList());
    // open file
	ifstream file_to_compile(filename, ios::binary);
    if (file_to_compile.is_open()) {
        while (std::getline(file_to_compile, line)) {
            for (string::iterator i = line.begin(); i != line.end(); i++)
                if (*i == '\0')
                    line.erase(i, i);
            this->lines->push_back(line + '\n');
        }
        file_to_compile.close();
    }
    this->entire_input = StringPtr(new string(""));
    for (StringList::iterator i = this->lines->begin();
         i != this->lines->end(); i++) {
        *this->entire_input += (*i);
    }
}

InputPtr Input::open_file(string filename) {
	ifstream test_file(filename, ios::in|ios::binary|ios::ate);
    if (test_file.is_open() && test_file.good()) {
        test_file.close();
        return InputPtr(new Input(filename));
    }
    else {
        report_msg_type("Invalid File Path", "Check to ensure path is correct");
        return nullptr;
    }
}

StringPtr Input::detach_input() {
	// copy and return detached input
    return StringPtr(new string(*this->entire_input));
}

void Input::print_input() {
	// test print input
	report_msg_type("Input", *this->entire_input);
}
