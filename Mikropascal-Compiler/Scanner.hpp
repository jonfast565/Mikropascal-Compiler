#ifndef scanner_h
#define scanner_h

#include "Standard.hpp"
#include "Input.hpp"
#include "FiniteAutomata.hpp"
#include "Tokens.hpp"
#include "Helper.hpp"

// makes this easier to read
class Scanner;
using ScannerPtr = shared_ptr<Scanner>;
using ScanBufPtr = shared_ptr<vector<char>>;
using ScanBuf = vector<char>;
using TokenPtr = shared_ptr<Token>;
using TokenList = vector<shared_ptr<Token>>;
using TokenListPtr = shared_ptr<vector<shared_ptr<Token>>>;

class Scanner {
private:
	// finite automata
	FSMachineListPtr fsmachines;
    
    // accept detectors
    bool some_accept();
    bool none_accept();
    
    // reset and run commands
    void reset_all_auto();
    void run_all_auto(string input);
    void run_chain();
    
    // accepting machine checks
    FSMachineListPtr not_dead();
    FSMachineListPtr accepting();
    FSMachineListPtr run_buffer();
    
    // loading finite automata
	void load_keyword_machines();
	void load_id_machine();
	void load_num_machines();
    void load_strand_machines(unsigned int nesting);
	
    // tokens
	TokenListPtr consumed;
    void consume(TokenPtr token);
    TokenPtr last_token();
    TokenPtr create_token(TokType token, string lexeme,
    unsigned int line, unsigned int column);
	
    // buffers
	shared_ptr<Input> input_ptr;
	StringPtr file_buf_ptr;
	ScanBufPtr scan_buf;
    string contents();
    void clear_buffer();
    
    // shave commands
    void shave_all();
    void shave_buffer();
    void shave_file_ptr();
    void shave_chain();
    
    // pointers
	string::iterator file_ptr;
	void set_fp_begin();
    string::iterator get_begin_fp();
	string::iterator get_end_fp();
	
    // line and column numbers
	unsigned long line_number;
	unsigned long col_number;
    
    // scanner internal operations
    TokenPtr scan_infinite();
    TokenPtr scan_finite();
    
    // debug set input string
    void debug_set_input_string(StringPtr input);
    
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
	int get_char();
    bool forward();
	bool rewind();
    void cache();
    void goto_next(char c);
    
    // useful scanning tools
    void skip_whitespace();
    
	// get line and col numbers
	unsigned long get_line_number();
	unsigned long get_col_number();
    unsigned long last_newline();
    
	// display tokens on the screen
	void display_tokens();
	void display_all_auto();
    
	// file ops
	void write_tokens_tof(string filename);
    void thin_comments();
	TokenListPtr detach_tokens();
};

#endif
