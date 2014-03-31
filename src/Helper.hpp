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
                              const unsigned long line, const unsigned long column) {
	return string("[ " + type + ": " + msg + " @ " +
                  to_string(line) + ":" + to_string(column) + " ]" + '\n');
}

static void report_error_lc(const string& type, const string& msg,
                            const unsigned long line, const unsigned long column) {
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

// returns all variants of a character (upper and lowercase)
static pair<char, char> all_char_variants(char c) {
    pair<char, char> char_pair;
    if (c == toupper(c)) {
        char_pair.first = c;
    } else {
        char_pair.first = toupper(c);
    }
    if (c == tolower(c)) {
        char_pair.second = c;
    } else {
        char_pair.second = tolower(c);
    }
    return char_pair;
}

static void write_raw(string s) {
    cout << s << endl;
}

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

template <class T>
static inline string conv_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

#endif
