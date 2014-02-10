/*
 * Scanner.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#ifndef scanner_h
#define scanner_h

#include "Standard.hpp"
#include "Input.hpp"
#include "FiniteAutomata.hpp"
#include "Tokens.hpp"

class Scanner {
private:
	// finite automata
	shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>> keyword_machines;
	shared_ptr<FiniteAutomataContainer> id_machine;
	shared_ptr<FiniteAutomataContainer> intlit_machine;
	shared_ptr<FiniteAutomataContainer> fltlit_machine;
	// tokens
	shared_ptr<vector<shared_ptr<Token>>> consumed_tokens;
	// file mgmt
	shared_ptr<Input> input_ptr;
	shared_ptr<string> file_buf_ptr;
	shared_ptr<vector<char>> scan_buf;
	string::iterator file_ptr;
	string::iterator get_begin_fp();
	string::iterator get_end_fp();
	void set_fp_begin();
	// file info (line & col)
	unsigned int line_number;
	unsigned int col_number;
	// kword ops
	void load_keyword_machines();
	void reset_all_kword();
	void step_all_kword(char next);
	bool check_all_kword_accept();
	TokType get_first_kword_accept();
	// id ops
	void load_id_machine();
	bool check_id_accepted();
	void step_id(char next);
	void reset_id();
	// numerical automata ops
	void load_num_automata();
	bool check_int_accepted();
	bool check_flt_accepted();
	void step_int(char next);
	void reset_int();
	void step_flt(char next);
	void reset_flt();
	// scanner internal ops
	shared_ptr<Token> scan_keyword_or_id();
	shared_ptr<Token> scan_num();
	shared_ptr<Token> scan_line_comment();
	shared_ptr<Token> scan_bracket_comment();
	shared_ptr<Token> scan_string_literal();
	// line and col numbers setters
	void set_line_number(int new_line_number);
	void set_col_number(int new_col_number);
public:
	Scanner(shared_ptr<Input> input_ptr);
	virtual ~Scanner();
	// reset the scanner
	void reset();
	// ld ops
	bool isalnum(char next);
	bool isnum(char next);
	bool isalpha(char next);
	// scan pointer with appropriate naming conventions
	shared_ptr<Token> dispatcher() {return this->scan_one();};
    // scan pointer (file pointer) movement
	shared_ptr<Token> scan_one();
	void scan_all();
	int peek();
	char next();
	bool right();
	bool left();
	// line and col numbers
	unsigned int get_line_number();
	unsigned int get_col_number();
	// display tokens on the screen
	void display_tokens();
	void display_all_automata();
	// file ops
	void write_tokens_tof(string filename);
    void thin_comments();
	shared_ptr<vector<shared_ptr<Token>>> detach_tokens();
};

#endif
