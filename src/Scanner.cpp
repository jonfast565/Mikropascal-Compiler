/*
 * Scanner.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#include "Scanner.hpp"

Scanner::Scanner(shared_ptr<Input> input_ptr) {
    // scanner constructor initializes all FAs and column, line numbers and file pointer
    // initialize input
	this->input_ptr = input_ptr;
	this->file_buf_ptr = input_ptr->detach_input();
    
    // initialize token list
	this->consumed_tokens = TokenListPtr(new vector<TokenPtr>);
    
    // create a scan buffer
	this->scan_buf = ScanBufPtr(new ScanBuf);
	this->load_keyword_machines();
	this->load_id_machine();
	this->load_num_automata();
	this->col_number = 1;
	this->line_number = 1;
	this->file_ptr = this->get_begin_fp();
}

string::iterator Scanner::get_begin_fp() {
	// get a pointer to the start of the file
	return this->file_buf_ptr->begin();
}

string::iterator Scanner::get_end_fp() {
	// get a pointer to the end of the file
	return this->file_buf_ptr->end();
}

void Scanner::set_fp_begin() {
	// push the file pointer back to the input's beginning
	this->file_ptr = this->file_buf_ptr->begin();
}

void Scanner::reset() {
	// reset all FSAs, delete all tokens, move file pointer to beginning
	this->reset_all_kword();
	this->keyword_machines->clear();
	this->set_fp_begin();
}

void Scanner::reset_all_kword() {
	// reset all keyword automata to their initial state
	for (vector<FSMachinePtr>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end())
			(*i)->reset();
	}
}

void Scanner::step_all_kword(char next) {
    // step one character over all automata currently initialized
	// run character 'next' on all FSAs
	for (vector<FSMachinePtr>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end())
			(*i)->step(next);
	}
}

bool Scanner::check_all_kword_accept() {
	// check if any keyword FSA has accepted
	for (vector<FSMachinePtr>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end()) {
			if ((*i)->accepted())
				return true;
		}
	}
	return false;
}

TokType Scanner::get_first_kword_accept() {
	// get the first accepted token if available from an FSA
	for (vector<FSMachinePtr>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if ((*i)->accepted()) {
			// names of the tokens are aligned with their enum names
			// which are the names of each created FSA
			return get_token_by_name((*i)->get_name());
		}
	}
	// this is weird, so return the weird one
	return TokType::MP_MALFORMED;
}

void Scanner::scan_all() {
    // scan over all token until the file pointer reaches the end of file
	while (!(this->dispatcher()->get_token() == TokType::MP_EOF)){};
}

void Scanner::skip_whitespace() {
    // check for space, tab, or newline, or etc...
	if (this->peek() == ' ' ||
        this->peek() == '\n' ||
        this->peek() == '\t' ||
        this->peek() == '\r' ||
        this->peek() == '\v' ||
        this->peek() == '\f') {
		while(this->peek() == ' ' ||
              this->peek() == '\n' ||
              this->peek() == '\t' ||
              this->peek() == '\r' ||
              this->peek() == '\v' ||
              this->peek() == '\f')
			this->right();
	}
}

TokenPtr Scanner::scan_one() {
	// dispatch one token
    // scan for whitespace
    this->skip_whitespace();
    
    // check for the end of file, and return the EOF token.
	if (this->peek() == '\0') {
		TokenPtr eof = TokenPtr(
                                new Token(this->get_line_number(), this->get_col_number(),
                                          TokType::MP_EOF, "EOF"));
		this->consumed_tokens->push_back(eof);
		return *(this->consumed_tokens->end() - 1);
	}
    
	// remove double dash comments
	if (this->peek() == get_token_info(TokType::MP_DIV).second[0]
        && (this->next() == get_token_info(TokType::MP_DIV).second[0])) {
		return this->scan_line_comment();
	}
    
	// remove bracket comments
	else if (this->peek()
             == get_token_info(TokType::MP_BRACKET_LEFT).second[0]) {
		return this->scan_bracket_comment();
	}
    
	// scan string literals
	else if (this->peek()
             == get_token_info(TokType::MP_STRING_LITERAL).second[0]) {
		return this->scan_string_literal();
	}
    
	// now look for other tokens that are actually important
	// find numeric tokens, literal, etc.
	else if (this->isnum(this->peek())) {
		return this->scan_num();
	}
    
	// scan a keyword, id, or malformed item?
	else {
		return this->scan_keyword_or_id();
	}
}

TokenPtr Scanner::scan_keyword_or_id() {
	// reset the id automata
	this->reset_id();
	this->reset_all_kword();
	if (this->isalnum(this->peek()) || this->peek() == '_') {
		// continuously parse
		while (this->isalnum(this->peek()) || this->peek() == '_') {
			// parse id
			// initialize the identifier automata
			this->step_id(this->peek());
			this->step_all_kword(this->peek());
			// push back the first character
			scan_buf->push_back(this->peek());
			// move right
			this->right();
		}
		TokType kword;
		if (this->check_all_kword_accept() == true
            && this->check_id_accepted() == true) {
			// get the matched token
			kword = this->get_first_kword_accept();
		} else if (this->check_all_kword_accept() == false
                   && this->check_id_accepted() == true) {
			kword = TokType::MP_ID;
		} else {
			// chances are this is bad
			kword = TokType::MP_MALFORMED;
		}
		// convert input stream to a string
		string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
		// generate token
		TokenPtr new_kword;
		if (kword != TokType::MP_MALFORMED) {
			new_kword = TokenPtr(
                                 new Token(this->get_line_number(), this->get_col_number(),
                                           kword, buf_lexeme));
		} else {
			new_kword =
            TokenPtr(
                     new Token(this->get_line_number(),
                               this->get_col_number(), kword,
                               format_error_lc("Scan Error", "Malformed ID", this->get_line_number(),
                                               this->get_col_number())));
		}
		// save token
		this->consumed_tokens->push_back(new_kword);
		// clear the scan buffer
		this->scan_buf->clear();
		// we're done here, so we can return the new token
		return *(this->consumed_tokens->end() - 1);
	}
	// look for symbols
	else {
		// parse short keyword
		// look at the char in the input
		// initialize the identifier automata
		this->step_all_kword(this->peek());
		// push back the first character
		scan_buf->push_back(this->peek());
		// get the first accepted symbol
		TokType symbol = this->get_first_kword_accept();
		TokenPtr new_symbol;
		// see if the symbol exists
		if (symbol == TokType::MP_MALFORMED) {
			// generate token
			new_symbol = TokenPtr(
                                  new Token(this->get_line_number(), this->get_col_number(),
                                            TokType::MP_ERROR,
                                            format_error_lc("Scan Error", "Unrecognized symbol '"
                                                            + string(scan_buf->begin(),
                                                                     scan_buf->end())
                                                            + "' (MP_ERROR)", this->get_line_number(),
                                                            this->get_col_number())));
		} else {
			// go one to the right, and get second accepted if possible
			this->step_all_kword(this->next());
			if (this->check_all_kword_accept() == true) {
				this->right();
				scan_buf->push_back(this->peek());
				symbol = this->get_first_kword_accept();
			}
			// convert input stream to a string
			string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
			// generate token
			new_symbol = TokenPtr(
                                  new Token(this->get_line_number(), this->get_col_number(),
                                            symbol, buf_lexeme));
		}
        
		// save token
		this->consumed_tokens->push_back(new_symbol);
		// clear the scan buffer
		this->scan_buf->clear();
		// move the file pointer ahead by 1 (as per contract)
		this->right();
		// we're done here, so we can return the new token
		return *(this->consumed_tokens->end() - 1);
	}
}

TokenPtr Scanner::scan_num() {
	// reset automata
	this->reset_flt();
	this->reset_int();
    
	// keep scanning until there is some nasty fail
	while (this->isnum(this->peek()) || this->peek() == '.'
           || this->peek() == 'E' || this->peek() == 'e' || this->peek() == '+'
           || this->peek() == '-') {
		// push back the first character
		scan_buf->push_back(this->peek());
		// step each automata
		this->step_int(this->peek());
		this->step_flt(this->peek());
		// move right
		this->right();
	}
	// once done, make the decision as to what type of number this is
	TokType numeric_type;
	if (this->check_flt_accepted()) {
		numeric_type = TokType::MP_FLOAT_LITERAL;
	} else if (this->check_int_accepted()) {
		numeric_type = TokType::MP_INT_LITERAL;
	} else {
		numeric_type = TokType::MP_MALFORMED;
	}
	// convert input stream to a string
	string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
	// generate token
	TokenPtr numeric_literal;
	// determine if a scan error exists
	if (numeric_type != TokType::MP_MALFORMED) {
		numeric_literal = TokenPtr(
                                   new Token(this->get_line_number(), this->get_col_number(),
                                             numeric_type, buf_lexeme));
	} else {
		numeric_literal =
        TokenPtr(
                 new Token(this->get_line_number(),
                           this->get_col_number(), TokType::MP_ERROR,
                           format_error_lc("Scan Error", "Malformed numeric literal (MP_ERROR)",
                                           this->get_line_number(), this->get_col_number())));
	}
	// save token
	this->consumed_tokens->push_back(numeric_literal);
	// clear the scan buffer
	this->scan_buf->clear();
	// we're done here, so we can return the new token
	return *(this->consumed_tokens->end() - 1);
}

TokenPtr Scanner::scan_line_comment() {
	// new line comment, so ends at the next line
	while (this->peek() != '\n') {
		// push the current char on to the scan buffer
		scan_buf->push_back(this->peek());
		// move the file pointer to the right a tad
		this->right();
	}
	// convert input stream to a string
	string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
	// create the token
	TokenPtr comment_tok = TokenPtr(
                                    new Token(this->get_line_number(), this->get_col_number(),
                                              TokType::MP_COMMENT, buf_lexeme));
	// push token onto the list of found tokens
	consumed_tokens->push_back(comment_tok);
	// clear the scan buffer
	this->scan_buf->clear();
	// move the file pointer ahead 1 (as per contract)
	this->right();
	// return the comment
	return *(this->consumed_tokens->end() - 1);
}

TokenPtr Scanner::scan_bracket_comment() {
	// push back the left bracket (lexeme) and
	// wait for the second bracket to appear under the file pointer
	stack<char> comment_stack;
	// consume until all comments are consumed
	TokType comment_valid = TokType::MP_COMMENT;
	TokenPtr comment_tok;
	do {
		// check for new opening and closing braces, or an invalid condition
		// (ie. reached the end of file)
		if (this->file_ptr == this->file_buf_ptr->end()) {
			// create the token
			comment_tok =
            TokenPtr(
                     new Token(this->get_line_number(),
                               this->get_col_number(),
                               TokType::MP_RUN_COMMENT,
                               format_error_lc("Scan Error", "Run on comment (MP_RUN_COMMENT)",
                                               this->get_line_number(), this->get_col_number())));
			// save the token
			this->consumed_tokens->push_back(comment_tok);
			// clear the scan buffer
			scan_buf->clear();
			// return the last token
			return *(this->consumed_tokens->end() - 1);
		} else if (this->peek()
                   == get_token_info(TokType::MP_BRACKET_LEFT).second[0])
			comment_stack.push(
                               get_token_info(TokType::MP_BRACKET_LEFT).second[0]);
		else if (this->peek()
                 == get_token_info(TokType::MP_BRACKET_RIGHT).second[0])
			comment_stack.pop();
		// pop the last seen character onto the buffer
		scan_buf->push_back(this->peek());
		// move the file pointer ahead 1 (as per contract)
		this->right();
	} while (!comment_stack.empty());
	// convert input stream to a string
	string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
	// create the token
	comment_tok = TokenPtr(
                           new Token(this->get_line_number(), this->get_col_number(),
                                     comment_valid, buf_lexeme));
	// save the token
	this->consumed_tokens->push_back(comment_tok);
	// clear the scan buffer
	scan_buf->clear();
	// return the last token
	return *(this->consumed_tokens->end() - 1);
}

TokenPtr Scanner::scan_string_literal() {
	// move right to prevent malformed condition
	this->right();
	// determine if the string is not run-on while scanning
	TokType string_valid = TokType::MP_STRING_LITERAL;
	while (this->peek() != '\'') {
		if (this->peek() == '\n') {
			string_valid = TokType::MP_MALFORMED;
			break;
		} else {
			scan_buf->push_back(this->peek());
			this->right();
		}
	}
	// convert input stream to a string
	string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
	// create the token
	TokenPtr string_tok;
	if (string_valid == TokType::MP_STRING_LITERAL) {
		string_tok = TokenPtr(
                              new Token(this->get_line_number(), this->get_col_number(),
                                        string_valid, buf_lexeme));
	} else {
		string_tok =
        TokenPtr(
                 new Token(this->get_line_number(),
                           this->get_col_number(), TokType::MP_RUN_STRING,
                           format_error_lc("Scan Error", "Run on string (MP_RUN_STRING)",
                                           this->get_line_number(), this->get_col_number())));
	}
	// save the token
	this->consumed_tokens->push_back(string_tok);
	// clear the scan buffer
	scan_buf->clear();
	// move the file pointer ahead 1 (as per contract)
	this->right();
	// return the last token
	return *(this->consumed_tokens->end() - 1);
}

int Scanner::peek() {
	// look at the current char in the input
	if (this->file_ptr != this->file_buf_ptr->end())
		// if not end, character is valid
		return *this->file_ptr;
	else
		return '\0';
}

char Scanner::next() {
	// look at the next char in the input, without consuming
	if (this->right()) {
		// get the next character
		char lookahead = this->peek();
		// go back
		this->left();
		// return this character
		return lookahead;
	} else
		return '\0';
    
}

bool Scanner::right() {
	// move the file pointer to the right by one
	if (this->file_ptr != this->file_buf_ptr->end()) {
		// deal with line and column numbers
		if (*this->file_ptr == '\n') {
			this->line_number++;
			this->col_number = 1;
		} else {
			this->col_number = this->col_number + 1;
		}
		this->file_ptr++;
		return true;
	} else
		return false;
}

bool Scanner::left() {
	// move the file pointer to the left by one
	if (this->file_ptr != this->file_buf_ptr->begin()) {
		// deal with line and column numbers
		// (unimplemented)
		// successful
		this->file_ptr = this->file_ptr - 1;
		return true;
	} else
		// unsuccessful
		return false;
}

void Scanner::set_line_number(int new_line_number) {
	// get the line number
	this->line_number = new_line_number;
}

void Scanner::set_col_number(int new_col_number) {
	// get the column number
	this->col_number = new_col_number;
}

void Scanner::load_keyword_machines() {
    // initialize all FSA for MP keywords
	this->keyword_machines = shared_ptr<
    vector<FSMachinePtr>>(new vector<FSMachinePtr>);
    
	// get the token type for all keyworded or single char types
	for (auto i = (int) TokType::MP_SEMI_COLON; i <= (int) TokType::MP_BOOLEAN; i++) {
		
        // create new automata for all types
		FSMachinePtr new_fa = shared_ptr<
        FiniteAutomataContainer>(
        new FiniteAutomataContainer(get_token_info((TokType)i).first, true));
		
        // build the keyword into the automata
		new_fa->build_keyword(get_token_info((TokType)i).second);
		
        // add to the vector of automata
		this->keyword_machines->push_back(new_fa);
	}
}

// load automata for scanning valid identifiers
void Scanner::load_id_machine() {
	// create the id scan automata
	this->id_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_ID).first,
    true));
    
	// detects ids without two underscores
	this->id_machine->add_state("0", true, false);
	this->id_machine->add_state("1", false, true);
	this->id_machine->add_state("2", false, true);
	this->id_machine->add_state("3", false, false);
	this->id_machine->add_state("4", false, false);
    
	// add transitions
	this->id_machine->add_alphabet("0", "1");
	this->id_machine->add_alphabet("1", "1");
	this->id_machine->add_digits("1", "1");
	this->id_machine->add_transition("1", '_', "4");
	this->id_machine->add_alphabet("4", "1");
	this->id_machine->add_digits("4", "1");
    
	// other part
	this->id_machine->add_transition("0", '_', "3");
	this->id_machine->add_alphabet("3", "2");
	this->id_machine->add_digits("3", "2");
	this->id_machine->add_alphabet("2", "2");
	this->id_machine->add_digits("2", "2");
	this->id_machine->add_transition("2", '_', "3");
}

bool Scanner::check_id_accepted() {
    // check if the identifier is accepted
	return this->id_machine->accepted();
}

void Scanner::step_id(char next) {
    // step through the identifier FA on char
	this->id_machine->step(next);
}

void Scanner::reset_id() {
    // reset identifier FA
	this->id_machine->reset();
}

void Scanner::load_num_automata() {
	// create floating point automata for scanning numeric literals
	this->fltlit_machine = FSMachinePtr(
    new FiniteAutomataContainer("FLT_LIT", true));
    
    // add states to this DFA, first and final
	this->fltlit_machine->add_state("1", true, false);
	this->fltlit_machine->add_state("2", false, true);
	this->fltlit_machine->add_state("3", false, false);
	this->fltlit_machine->add_state("4", false, false);
	this->fltlit_machine->add_state("5", false, true);
	this->fltlit_machine->add_digits("1", "1");
	this->fltlit_machine->add_transition("1", '.', "2");
	this->fltlit_machine->add_digits("2", "2");
	this->fltlit_machine->add_transition("2", 'e', "3");
	this->fltlit_machine->add_transition("2", 'E', "3");
	this->fltlit_machine->add_transition("3", '+', "4");
	this->fltlit_machine->add_transition("3", '-', "4");
	this->fltlit_machine->add_digits("3", "5");
	this->fltlit_machine->add_digits("4", "5");
    
	// create integer automata
	this->intlit_machine = FSMachinePtr(
    new FiniteAutomataContainer("INT_LIT", true));
    
    // add states to this DFA
	this->intlit_machine->add_state("0", true, false);
	this->intlit_machine->add_state("1", false, true);
	this->intlit_machine->add_digits("0", "1");
	this->intlit_machine->add_digits("1", "1");
    
	// reset both
	this->fltlit_machine->reset();
	this->intlit_machine->reset();
}

bool Scanner::check_int_accepted() {
    // check if int machine accepted
	return this->intlit_machine->accepted();
}

void Scanner::step_int(char next) {
    // step int machine
	this->intlit_machine->step(next);
}

void Scanner::reset_int() {
    // reset int machine
	this->intlit_machine->reset();
}

bool Scanner::check_flt_accepted() {
    // check if float machine accepted
	return this->fltlit_machine->accepted();
}

void Scanner::step_flt(char next) {
    // step float machine
	this->fltlit_machine->step(next);
}

void Scanner::reset_flt() {
    // reset float machine
	this->fltlit_machine->reset();
}

bool Scanner::isalnum(char next) {
    // load automata for determining if an input symbol is a letter or digit
	if (isalpha(next) || isdigit(next))
		return true;
	else
		return false;
}

bool Scanner::isnum(char next) {
	// step over a digit
	FSMachinePtr is_digit = shared_ptr<
    FiniteAutomataContainer>(
    new FiniteAutomataContainer("IS_DIGIT", true));
    
    // add states
	is_digit->add_state("0", true, false);
	is_digit->add_state("1", false, true);
	is_digit->add_digits("0", "1");
    
	// reset to start
	is_digit->reset();
    
	// step
	is_digit->step(next);
    
	// determine acceptance
	if (is_digit->accepted() == true) {
		return true;
	} else {
		return false;
	}
}

bool Scanner::isalpha(char next) {
    // step over a letter
	FSMachinePtr is_letter = shared_ptr<
    FiniteAutomataContainer>(
    new FiniteAutomataContainer("IS_LETTER", true));
    
    // add states
	is_letter->add_state("0", true, false);
	is_letter->add_state("1", false, true);
	is_letter->add_alphabet("0", "1");
    
	// reset
	is_letter->reset();
    
	// step on
	is_letter->step(next);
    
	// determine acceptance
	if (is_letter->accepted() == true) {
		return true;
	} else {
		return false;
	}
}

unsigned int Scanner::get_line_number() {
	// get the scanning line number
	return this->line_number;
}

unsigned int Scanner::get_col_number() {
	// get the scanning column number
	return this->col_number;
}

// remove comments from the list of consumed tokens
void Scanner::thin_comments() {
    if (this->consumed_tokens->size() > 0) {
        for (vector<TokenPtr>::iterator i =
        this->consumed_tokens->begin();
        i != consumed_tokens->end(); i++)
        {
            if ((*i)->get_token() == MP_COMMENT) {
                this->consumed_tokens->erase(i);
            }
        }
    }
}

// write tokens to a file
void Scanner::write_tokens_tof(string filename) {
	if (this->consumed_tokens->size() > 0) {
		ofstream file;
		file.open(filename);
		for (vector<TokenPtr>::iterator i =
             this->consumed_tokens->begin();
             i != this->consumed_tokens->end(); i++) {
			file << (*(*i)->to_string()) << '\n';
		}
		file.close();
	}
}

void Scanner::display_tokens() {
	for (vector<TokenPtr>::iterator i = this->consumed_tokens->begin();
         i != consumed_tokens->end(); i++) {
		report_error_lc("Token found", "'" + (*i)->get_lexeme() + "'", (*i)->get_line(), (*i)->get_column());
	}
}

TokenListPtr Scanner::detach_tokens() {
	return this->consumed_tokens;
}

