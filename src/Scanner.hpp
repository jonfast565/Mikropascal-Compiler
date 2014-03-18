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
#include "Helper.hpp"

// makes this easier to read
using ScanBufPtr = shared_ptr<vector<char>>;
using ScanBuf = vector<char>;
using FSMachinePtr = shared_ptr<FiniteAutomataContainer>;
using FSMachineList = vector<shared_ptr<FiniteAutomataContainer>>;
using FSMachineListPtr = shared_ptr<vector<shared_ptr<FiniteAutomataContainer>>>;

using TokenPtr = shared_ptr<Token>;
using TokenList = vector<shared_ptr<Token>>;
using TokenListPtr = shared_ptr<vector<shared_ptr<Token>>>;

class Scanner {
private:
	// finite automata
	FSMachineListPtr keyword_machines;
	FSMachinePtr id_machine;
	FSMachinePtr intlit_machine;
	FSMachinePtr fltlit_machine;
	
    // tokens
	TokenListPtr consumed_tokens;
	
    // file pointer and input mgmt
	shared_ptr<Input> input_ptr;
	StringPtr file_buf_ptr;
	ScanBufPtr scan_buf;
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
	TokenPtr scan_keyword_or_id();
	TokenPtr scan_num();
	TokenPtr scan_line_comment();
	TokenPtr scan_bracket_comment();
	TokenPtr scan_string_literal();
	
    // line and col numbers setters
	void set_line_number(int new_line_number);
	void set_col_number(int new_col_number);
    
public:
    // constructors
	Scanner(shared_ptr<Input> input_ptr);
	virtual ~Scanner() = default;
	
    // reset the scanner
	void reset();
	
    // drop in replacements using DFAs
	bool isalnum(char next);
	bool isnum(char next);
	bool isalpha(char next);
	
    // scan pointer with appropriate naming conventions
	TokenPtr dispatcher() {return this->scan_one();};
    
    // scan pointer (file pointer) movement
    void scan_all();
	TokenPtr scan_one();
	int peek();
	char next();
    bool left();
	bool right();
    
    // useful scanning tools
    void skip_whitespace();
    
	// get line and col numbers
	unsigned int get_line_number();
	unsigned int get_col_number();
    
	// display tokens on the screen
	void display_tokens();
	void display_tokens_as_msg();
	void display_all_automata();
    
	// file ops
	void write_tokens_tof(string filename);
    void thin_comments();
	TokenListPtr detach_tokens();
};

#endif
