/*
 * Scanner.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#include "Scanner.hpp"
#define PRINT_DEBUG 0

// scanner constructor initializes all FAs and column, line numbers and file pointer
Scanner::Scanner(shared_ptr<Input> input_ptr) {
	// ctor
	this->input_ptr = input_ptr;
	this->file_buf_ptr = input_ptr->detach_input();
	this->consumed_tokens = shared_ptr<vector<shared_ptr<Token>>>(new vector<shared_ptr<Token>>);
	this->keyword_machines = shared_ptr<
    vector<shared_ptr<FiniteAutomataContainer>>>(new vector<shared_ptr<FiniteAutomataContainer>>);
	this->scan_buf = shared_ptr<vector<char>>(new vector<char>);
	this->load_keyword_machines();
	this->load_id_machine();
	this->load_num_automata();
	this->col_number = 0;
	this->line_number = 0;
	this->file_ptr = this->get_begin_fp();
}

// destructor, releases all objects
Scanner::~Scanner() {
	// dtor
	this->input_ptr.reset();
	this->file_buf_ptr.reset();
	this->consumed_tokens.reset();
	this->keyword_machines.reset();
	this->id_machine.reset();
}

// get an iterator that returns the beginning of the input stream
string::iterator Scanner::get_begin_fp() {
	// get a pointer to the start of the file
	return this->file_buf_ptr->begin();
}

// get an iterator that goes to the end of the input stream
string::iterator Scanner::get_end_fp() {
	// get a pointer to the end of the file
	return this->file_buf_ptr->end();
}

// set the file pointer to the beginning of the input stream
void Scanner::set_fp_begin() {
	// push the file pointer back to the input's beginning
	this->file_ptr = this->file_buf_ptr->begin();
}

// reset
void Scanner::reset() {
	// reset all FSAs, delete all tokens, move file pointer to beginning
	this->reset_all_kword();
	this->keyword_machines->clear();
	this->set_fp_begin();
}

// re-initialize all automata dealing with keyword matching
void Scanner::reset_all_kword() {
	// reset all automata to their initial state
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end())
			(*i)->reset();
	}
}

// step one character over all automata currently initialized
void Scanner::step_all_kword(char next) {
	// run character 'next' on all FSAs
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end())
			(*i)->step(next);
	}
}

// find any keyword that has accepted
bool Scanner::check_all_kword_accept() {
	// check if any keyword FSA has accepted
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
         this->keyword_machines->begin(); i != this->keyword_machines->end();
         i++) {
		if (i != this->keyword_machines->end()) {
			if ((*i)->accepted())
				return true;
		}
	}
	return false;
}

// get the first accepted token
TokType Scanner::get_first_kword_accept() {
	// get the first accepted token if available from an FSA
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
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

// scan over all token until the file pointer reaches the end of file
void Scanner::scan_all() {
	while (true)
		if (this->dispatcher()->get_token() == TokType::MP_EOF)
            break;
}

// scan over one token (this is the dispatcher method!!!... crickey critical mate)
shared_ptr<Token> Scanner::scan_one() {
	
    // VIRTUAL CONTRACT CLAUSE ADHERENCE: Always stop at the beginning of the next token.
	// token, move there at the end of every case
	// using this->right();
    
	// tokenize the input until done
	// debug printer for the scanner
	if (PRINT_DEBUG) {
		static int iteration = 0;
		cout << "Iteration: " << iteration << endl;
		cout << "-" << endl;
		this->display_tokens();
		cout << "-" << endl;
		iteration++;
	}
    
	// check for the end of file, and return the EOF token.
	if (this->file_ptr == this->file_buf_ptr->end()) {
		shared_ptr<Token> eof = shared_ptr<Token>(new Token(this->get_line_number(), this->get_col_number(),
                                                            TokType::MP_EOF, "EOF"));
		this->consumed_tokens->push_back(eof);
		return *(this->consumed_tokens->end() - 1);
	}
	// basic cases for comments, whitespace, and newlines
	// remove whitespace
	if (this->peek() == get_token_info(TokType::MP_NULLCHAR).second[0]) {
		// create token
		shared_ptr<Token> null_tok = shared_ptr<Token>(
                                                       new Token(this->get_line_number(), this->get_col_number(),
                                                                 TokType::MP_NULLCHAR, "\0"));
		// push back and return
		this->consumed_tokens->push_back(null_tok);
		return *(this->consumed_tokens->end() - 1);
	}
	// check for space or newline
	if (this->peek() == ' ' || this->peek() == '\n') {
		// ignore whitespace
		// ignore newline, (line number managed in right() and left())
		// move the file pointer ahead 1 (as per contract)
		this->right();
	}
	// remove double dash comments
	else if (this->peek() == get_token_info(TokType::MP_DIV).second[0]
             && (this->next() == get_token_info(TokType::MP_DIV).second[0])) {
		return this->scan_line_comment();
	}
	// remove bracket comments
	else if (this->peek()
             == get_token_info(TokType::MP_BRACKET_LEFT).second[0]) {
		return this->scan_bracket_comment();
	}
	// scan string literals
	else if (this->peek() == get_token_info(TokType::MP_STRING_LITERAL).second[0]) {
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
	// should not happen
	return shared_ptr<Token>(
                             new Token(this->get_line_number(), this->get_col_number(),
                                       TokType::MP_MALFORMED, "INVALID"));
}

shared_ptr<Token> Scanner::scan_keyword_or_id() {
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
        shared_ptr<Token> new_kword;
        if (kword != TokType::MP_MALFORMED) {
            new_kword = shared_ptr<Token>(
                                          new Token(this->get_line_number(), this->get_col_number(),
                                                    kword, buf_lexeme));
        } else {
            new_kword = shared_ptr<Token>(
                                          new Token(this->get_line_number(), this->get_col_number(),
                                                    kword, string("<< Scan Error: Malformed ID detected at line " + to_string(this->get_line_number()) + " col " + to_string(this->get_col_number() - this->scan_buf->size()) + " >>")));
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
        shared_ptr<Token> new_symbol;
        // see if the symbol exists
        if (symbol == TokType::MP_MALFORMED) {
            // generate token
            new_symbol = shared_ptr<Token>(
                                           new Token(this->get_line_number(), this->get_col_number(),
                                                     TokType::MP_ERROR, string("<< Scan Error: Unrecognized symbol '" + string(scan_buf->begin(), scan_buf->end()) + "' (MP_ERROR) at line " + to_string(this->get_line_number()) + " col " + to_string(this->get_col_number()) + " >>")));
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
            new_symbol = shared_ptr<Token>(
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
shared_ptr<Token> Scanner::scan_num() {
	// reset automata
	this->reset_flt();
	this->reset_int();
    
	// keep scanning until there is some nasty fail
	while (this->isnum(this->peek()) || this->peek() == '.' ||
           this->peek() == 'E' || this->peek() == 'e' ||
           this->peek() == '+' || this->peek() == '-') {
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
    shared_ptr<Token> numeric_literal;
    // determine if a scan error exists
    if (numeric_type != TokType::MP_MALFORMED) {
        numeric_literal = shared_ptr<Token>(
                                            new Token(this->get_line_number(), this->get_col_number(), numeric_type,
                                                      buf_lexeme));
    } else {
        numeric_literal = shared_ptr<Token>(
                                            new Token(this->get_line_number(), this->get_col_number(), TokType::MP_ERROR,
                                                      string("<< Scan Error: Malformed numeric literal detected (MP_ERROR) at line " + to_string(this->get_line_number()) + " col " + to_string(this->get_col_number()) + " >>")));
    }
	// save token
	this->consumed_tokens->push_back(numeric_literal);
	// clear the scan buffer
	this->scan_buf->clear();
	// we're done here, so we can return the new token
	return *(this->consumed_tokens->end() - 1);
}
shared_ptr<Token> Scanner::scan_line_comment() {
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
	shared_ptr<Token> comment_tok = shared_ptr<Token>(
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
shared_ptr<Token> Scanner::scan_bracket_comment() {
	// push back the left bracket (lexeme) and
	// wait for the second bracket to appear under the file pointer
	stack<char> comment_stack;
	// consume until all comments are consumed
    TokType comment_valid = TokType::MP_COMMENT;
    shared_ptr<Token> comment_tok;
	do {
		// check for new opening and closing braces, or an invalid condition
        // (ie. reached the end of file)
        if (this->file_ptr == this->file_buf_ptr->end()) {
            // create the token
            comment_tok = shared_ptr<Token>(
                                            new Token(this->get_line_number(), this->get_col_number(),
                                                      TokType::MP_RUN_COMMENT, string("<< Scan Error: Comment reaching EOF detected (MP_RUN_COMMENT) at line " + to_string(this->get_line_number()) + " col " + to_string(this->get_col_number()) + " >>")));
            // save the token
            this->consumed_tokens->push_back(comment_tok);
            // clear the scan buffer
            scan_buf->clear();
            // return the last token
            return *(this->consumed_tokens->end() - 1);
        }
		else if (this->peek() == get_token_info(TokType::MP_BRACKET_LEFT).second[0])
			comment_stack.push(get_token_info(TokType::MP_BRACKET_LEFT).second[0]);
		else if (this->peek() == get_token_info(TokType::MP_BRACKET_RIGHT).second[0])
			comment_stack.pop();
		// pop the last seen character onto the buffer
		scan_buf->push_back(this->peek());
		// move the file pointer ahead 1 (as per contract)
		this->right();
	} while (!comment_stack.empty());
	// convert input stream to a string
	string buf_lexeme = string(scan_buf->begin(), scan_buf->end());
	// create the token
	comment_tok = shared_ptr<Token>(
                                                      new Token(this->get_line_number(), this->get_col_number(),
                                                                comment_valid, buf_lexeme));
	// save the token
	this->consumed_tokens->push_back(comment_tok);
	// clear the scan buffer
	scan_buf->clear();
	// return the last token
	return *(this->consumed_tokens->end() - 1);
}

shared_ptr<Token> Scanner::scan_string_literal() {
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
	shared_ptr<Token> string_tok;
    if (string_valid == TokType::MP_STRING_LITERAL) {
        string_tok = shared_ptr<Token>(
                                       new Token(this->get_line_number(), this->get_col_number(),
                                                 string_valid, buf_lexeme));
    } else {
        string_tok = shared_ptr<Token>(
                                       new Token(this->get_line_number(), this->get_col_number(),
                                                 TokType::MP_RUN_STRING, string("<< Scan Error: Run-on string detected (MP_RUN_STRING) at line " + to_string(this->get_line_number()) + " col " + to_string(this->get_col_number()) + " >>")));
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

// look at the current character under the file pointer
char Scanner::peek() {
	// look at the current char in the input
	if (this->file_ptr != this->file_buf_ptr->end())
		// if not end, character is valid
		return *this->file_ptr;
	else
		return '\0';
}

// look at the next character under the file pointer
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

// move the file pointer to the right
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
		// successful
		this->file_ptr++;
		return true;
	} else
		// unsuccessful
		return false;
}

// move the file pointer left
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

// set the line number
void Scanner::set_line_number(int new_line_number) {
	// get the line number
	this->line_number = new_line_number;
}

// set the column number
void Scanner::set_col_number(int new_col_number) {
	// get the column number
	this->col_number = new_col_number;
}

// create automata for dealing with keywords
void Scanner::load_keyword_machines() {
	// get the token type for all keyworded or single char types
	for (TokType i = TokType::MP_SEMI_COLON; i <= TokType::MP_BOOLEAN; i++) {
		// create new automata for all types
		shared_ptr<FiniteAutomataContainer> new_fa = shared_ptr<
        FiniteAutomataContainer>(
                                 new FiniteAutomataContainer(get_token_info(i).first, true));
		// build the keyword into the automata
		new_fa->build_keyword(get_token_info(i).second);
		// add to the vector of automata
		this->keyword_machines->push_back(new_fa);
	}
}

// load automata for scanning valid identifiers
void Scanner::load_id_machine() {
	// create the id scan automata
	this->id_machine = shared_ptr<FiniteAutomataContainer>(
                                                           new FiniteAutomataContainer(get_token_info(TokType::MP_ID).first,
                                                                                       true));
	// build custom id automata (based on homework)
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

// check if the identifier is accepted
bool Scanner::check_id_accepted() {
	return this->id_machine->accepted();
}

// step through the identifier FA on char
void Scanner::step_id(char next) {
	this->id_machine->step(next);
}

// reset identifier FA
void Scanner::reset_id() {
	this->id_machine->reset();
}

// load automata for scanning numeric literals
void Scanner::load_num_automata() {
	// create floating point automata
	this->fltlit_machine = shared_ptr<FiniteAutomataContainer>(
                                                               new FiniteAutomataContainer("FLT_LIT", true));
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
	this->intlit_machine = shared_ptr<FiniteAutomataContainer>(
                                                               new FiniteAutomataContainer("INT_LIT", true));
	this->intlit_machine->add_state("0", true, false);
	this->intlit_machine->add_state("1", false, true);
	this->intlit_machine->add_digits("0", "1");
	this->intlit_machine->add_digits("1", "1");
    
	// reset both
	this->fltlit_machine->reset();
	this->intlit_machine->reset();
}

bool Scanner::check_int_accepted() {
	return this->intlit_machine->accepted();
}

void Scanner::step_int(char next) {
	this->intlit_machine->step(next);
}

void Scanner::reset_int() {
	this->intlit_machine->reset();
}

bool Scanner::check_flt_accepted() {
	return this->fltlit_machine->accepted();
}

void Scanner::step_flt(char next) {
	this->fltlit_machine->step(next);
}

void Scanner::reset_flt() {
	this->fltlit_machine->reset();
}

// load automata for determining if an input symbol is a letter or digit
bool Scanner::isalnum(char next) {
	if (isalpha(next) || isdigit(next))
		return true;
	else
		return false;
}

// step over a digit
bool Scanner::isnum(char next) {
	// load automata that decides whether or not a character is a digit
	// operation is performed only once before dead state
	// so automata must be reset on every run
	shared_ptr<FiniteAutomataContainer> is_digit = shared_ptr<
    FiniteAutomataContainer>(
                             new FiniteAutomataContainer("IS_DIGIT", true));
	is_digit->add_state("0", true, false);
	is_digit->add_state("1", false, true);
	is_digit->add_digits("0", "1");
	// reset to start
	is_digit->reset();
	// step
	is_digit->step(next);
	// determine acceptance
	// then free memory and return result
	if (is_digit->accepted() == true) {
		is_digit.reset();
		return true;
	} else {
		is_digit.reset();
		return false;
	}
}

// step over a letter
bool Scanner::isalpha(char next) {
	// load automata that decides whether or not a character is a letter
	// operation is performed only once before dead state
	// so automata must be reset on every run
	shared_ptr<FiniteAutomataContainer> is_letter = shared_ptr<
    FiniteAutomataContainer>(
                             new FiniteAutomataContainer("IS_LETTER", true));
	is_letter->add_state("0", true, false);
	is_letter->add_state("1", false, true);
	is_letter->add_alphabet("0", "1");
	// reset both
	is_letter->reset();
	// step on both
	is_letter->step(next);
	// determine acceptance
	// then free memory and return result
	if (is_letter->accepted() == true) {
		is_letter.reset();
		return true;
	} else {
		is_letter.reset();
		return false;
	}
}

// get the line number
unsigned int Scanner::get_line_number() {
	// get the scanning line number
	return this->line_number;
}

// get the column number
unsigned int Scanner::get_col_number() {
	// get the scanning column number
	return this->col_number;
}

// write tokens to a file
void Scanner::write_tokens_tof(string filename) {
	if (this->consumed_tokens->size() > 0) {
		ofstream file;
		file.open(filename);
		for (vector<shared_ptr<Token>>::iterator i =
             this->consumed_tokens->begin(); i != this->consumed_tokens->end();
             i++) {
			file << (*(*i)->to_string()) << '\n';
		}
		file.close();
	}
}

// display tokens on the screen
void Scanner::display_tokens() {
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++)
		cout << "-";
	cout << endl;
	stringstream ss;
	ss << setw(1) << std::left << "| ";
	ss << setw(25) << std::left << "Token:";
	ss << setw(10) << std::left << "Line:";
	ss << setw(10) << std::left << "Column:";
	ss << setw(30) << std::left << "Lexeme:";
	cout << ss.str() << endl;
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++)
		cout << "-";
	cout << endl;
	for (vector<shared_ptr<Token>>::iterator i = this->consumed_tokens->begin();
         i != consumed_tokens->end(); i++) {
		shared_ptr<string> string_rep = (*i)->to_string();
		cout << "| " << *string_rep << endl;
	}
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++)
		cout << "-";
	cout << endl;
}

