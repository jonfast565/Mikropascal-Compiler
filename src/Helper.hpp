//
//  Helper.h
//  CompilerTest
//
//  Created by Jonathan Fast on 2/9/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#ifndef helper_h
#define helper_h

#include "Standard.hpp"

static string format_error_lc(const string& type, const string& msg,
		const unsigned int line, const unsigned int column) {
	return string("[ " + type + ": " + msg + " @ " +
			to_string(line) + ":" + to_string(column) + " ]" + '\n');
}

static void report_error_lc(const string& type, const string& msg,
		const unsigned int line, const unsigned int column) {
    cerr << "[ " << type << ": " << msg << " @ " << line << ":" << column << " ]" << endl;
}

static string format_error(const string& type, const string& msg) {
	return string("[ " + type + ": " + msg + " ]" + '\n');
}

static void report_error(const string& type, const string& msg) {
    cerr << "[ " << type << ": " << msg << " ]" << endl;
}

static void report_msg(const string& msg) {
    cout << "[ " << msg << " ]" << endl;
}

static void report_msg_type(const string& type, const string& msg) {
    cout << "[ " << type << ": " << msg << " ]" << endl;
}

static void report_parse(const string& msg, const unsigned int depth) {
    if (DEBUG_OUTPUT) {
    	cout << "[ ";
        for (unsigned int i = 0; i < depth; i++) {
            cout << " ";
        }
        cout << msg << " ]" << endl;
    }
}

#endif