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
	this->consumed = TokenListPtr(new vector<TokenPtr>);
    
    // initialize all FSAs for MP keywords
	this->fsmachines = unique_ptr<
    vector<FSMachinePtr>>(new vector<FSMachinePtr>);
    
    // create a scan buffer
	this->scan_buf = ScanBufPtr(new ScanBuf);
    
    // load all machines
	this->load_keyword_machines();
	this->load_id_machine();
	this->load_num_machines();
    this->load_strand_machines(4);
    
    // set the line and column numbers to default
	this->col_number = 1L;
	this->line_number = 1L;
    
    // put the file pointer on the first character in the buffer
	this->file_ptr = this->get_begin_fp();
}

void Scanner::debug_set_input_string(StringPtr input) {
    // for testing purposes only
    // breaks encapsulation
    this->file_buf_ptr = input;
    this->file_ptr = input->begin();
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
	this->reset_all_auto();
    this->consumed->clear();
	this->set_fp_begin();
}

void Scanner::reset_all_auto() {
    // reset all FSMachines to their default states
	for (auto i = this->fsmachines->begin();
         i != fsmachines->end(); i++) {
        (*i)->reset();
    }
}

void Scanner::run_all_auto(string input) {
    // reset all FSMachines to their default states
	for (auto i = this->fsmachines->begin();
         i != fsmachines->end(); i++) {
        (*i)->run(input);
    }
}

void Scanner::scan_all() {
    // scan over all token until the file pointer reaches the end of file
	while (!(this->dispatcher()->get_token() == TokType::MP_EOF)){};
}

void Scanner::skip_whitespace() {
    // check for space, tab, or newline, or etc...
	if (this->get_char() == ' ' ||
        this->get_char() == '\n' ||
        this->get_char() == '\t' ||
        this->get_char() == '\r' ||
        this->get_char() == '\v' ||
        this->get_char() == '\f') {
		while(this->get_char() == ' ' ||
              this->get_char() == '\n' ||
              this->get_char() == '\t' ||
              this->get_char() == '\r' ||
              this->get_char() == '\v' ||
              this->get_char() == '\f')
			this->forward();
	}
}

void Scanner::consume(TokenPtr token) {
    // add a token to the buffer
    this->consumed->push_back(token);
}

TokenPtr Scanner::last_token() {
    // get the last token added to the buffer
    return *(this->consumed->end() - 1);
}

string Scanner::contents() {
    // dump the buffer contents
    return string(this->scan_buf->begin(), this->scan_buf->end());
}

bool Scanner::some_accept() {
    // determine if some machines accept
    for (auto i = this->fsmachines->begin(); i !=
         this->fsmachines->end(); i++) {
        if ((*i)->accepting()) {
            return true;
        }
    }
    return false;
}

bool Scanner::none_accept() {
    // determine if no machines accept
    for (auto i = this->fsmachines->begin(); i !=
         this->fsmachines->end(); i++) {
        if ((*i)->accepting()) {
            return false;
        }
    }
    return true;
}

FSMachineListPtr Scanner::not_dead() {
    // return machines not in dead states
    FSMachineListPtr alive_machines =
    FSMachineListPtr(new FSMachineList());
    for (auto i = this->fsmachines->begin(); i !=
         this->fsmachines->end(); i++) {
        if ((*i)->not_dead() == true) {
            alive_machines->push_back(*i);
        }
    }
    return alive_machines;
}

FSMachineListPtr Scanner::run_buffer() {
    // run every machine against the buffer contents
    this->run_all_auto(this->contents());
    // return machines not in dead states
    FSMachineListPtr alive_machines = this->not_dead();
    return alive_machines;
}

FSMachineListPtr Scanner::accepting() {
    // return a list of all accepting machines
    // ordered by priority
    FSMachineListPtr accepting_machines =
    FSMachineListPtr(new FSMachineList());
    for (auto i = this->fsmachines->begin(); i !=
         this->fsmachines->end(); i++) {
        if ((*i)->accepting() == true) {
            accepting_machines->push_back(*i);
        }
    }
    sort(accepting_machines->begin(),
                accepting_machines->end());
    return accepting_machines;
}

TokenPtr Scanner::scan_one() {
    // scan for whitespace
    this->skip_whitespace();
    
    // return EOF on failed scan of
    // end of file
    if (this->get_char() == '\0') {
        TokenPtr new_token = TokenPtr(new Token());
        new_token->set_token(MP_EOF);
        new_token->set_lexeme("EOF");
        new_token->set_line(this->line_number);
        new_token->set_column(this->col_number);
        return new_token;
    }
    
    // throw a character into the buffer
    this->cache();
    
    // move the input pointer forward
    this->forward();
    
    // run buffer, then check to see what scanning
    // mode we should be in
    this->run_buffer();
    
    // if none accept, scan to the end of the input
    // to try to accept
    if (this->none_accept()) {
        // scan until we find something, or reach EOF
        this->reset_all_auto();
        return this->scan_infinite();
    } else {
        // scan until nothing accepts any longer
        this->reset_all_auto();
        return this->scan_finite();
    }
}

void Scanner::shave_all() {
    // shave both the scan buffer
    // by a character and rewind the file
    // pointer by one
    this->shave_buffer();
    this->shave_file_ptr();
}

void Scanner::shave_buffer() {
    // pop the scan buffer
    this->scan_buf->pop_back();
}

void Scanner::shave_file_ptr() {
    // rewind the file pointer by one
    this->rewind();
}

void Scanner::shave_chain() {
    // shave all
    this->shave_all();
    // reset all machines
    this->reset_all_auto();
    // run buffer
    this->run_buffer();
}

void Scanner::clear_buffer() {
    // clear the scan buffer
    this->scan_buf->clear();
}

void Scanner::run_chain() {
    // standard run chain for all buffer moves
    // reset all machines to their start states
    this->reset_all_auto();
    // cache the current token under the fp
    this->cache();
    // move the file pointer forward 1
    this->forward();
    // run the entire buffer against the automata
    // individually
    this->run_buffer();
    // use with some, no, and not_dead()
}

TokenPtr Scanner::scan_infinite() {
    // a token pointer to see what we can get
    TokenPtr new_token = TokenPtr(new Token());
    unsigned long start_line = this->line_number;
    unsigned long start_column = this->col_number;
    new_token->set_line(start_line);
    new_token->set_column(--start_column);
    // moves to the end of the buffer
    unsigned int moves = 0;
    // scan until the null character
    while (this->get_char() != '\0') {
        // run the buffer
        this->run_buffer();
        // check if a machine accepted
        if (some_accept()) {
            // if some machine(s) is(are) accepting
            while(!none_accept() || this->not_dead()->size() > 0) {
                // scan until none accept
                this->run_chain();
            }
            // shave chain
            this->shave_chain();
            // error condition checked
            if (this->none_accept()) {
                break;
            }
            // some will accept, since
            // the condition above satisfies it
            // get first high priority accepting machines
            FSMachinePtr accepting = move(*this->accepting()->begin());
            string token_name = accepting->get_name();
            // create a token
            TokType this_tok = get_token_by_name(token_name);
            new_token->set_token(this_tok);
            string contents = this->contents();
            contents = to_lower(contents);
            new_token->set_lexeme(contents);
            // clear buffer
            this->clear_buffer();
            // return new token
            this->consume(new_token);
            return new_token;
        } else {
            this->reset_all_auto();
            // cache the current token under the fp
            this->cache();
            // move the file pointer forward 1
            this->forward();
            // run the entire buffer against the automata
            // individually
        }
        moves++;
    }
    // error condition
    // unwind the file pointer back to the first item
    for (unsigned int i = moves; i > 0; i--) {
        this->shave_all();
    }
    if (*this->scan_buf->begin() == '\'') {
        new_token->set_token(MP_RUN_STRING);
        new_token->set_error("There is a run on string here");
        this->goto_next('\n');
    } else if (*this->scan_buf->begin() == '{') {
        new_token->set_token(MP_RUN_COMMENT);
        new_token->set_error("There is a run on comment here");
        this->goto_next('\n');
    } else {
        new_token->set_token(MP_ERROR);
        new_token->set_error("There is an unreconized token here");
    }
    // create the error token
    string contents = this->contents();
    contents = to_lower(contents);
    new_token->set_lexeme(contents);
    // hidden error message here? (might be good)
    report_error_lc("Scan Error", new_token->get_error() + contents,
                    start_line, start_column);
    // clear buffer
    this->clear_buffer();
    // return error token
    this->consume(new_token);
    return new_token;
}

TokenPtr Scanner::scan_finite() {
    // token to build
    TokenPtr new_token = TokenPtr(new Token());
    new_token->set_line(this->line_number);
    unsigned long cur_col = this->col_number;
    new_token->set_column(--cur_col);
    // wait until no state machine accepts
    while (!this->none_accept() || this->not_dead()->size() > 0) {
        // check for the null pointed item
        if (this->get_char() == '\0') {
            break;
        }
        // store and forward
        this->run_chain();
    }
    // shave off the end of the buffer
    this->shave_chain();
    // get first high priority accepting machine
    FSMachineListPtr accepting_list = this->accepting();
    FSMachinePtr accepting = move(*accepting_list->begin());
    string token_name = accepting->get_name();
    // create a token
    TokType this_tok = get_token_by_name(token_name);
    new_token->set_token(this_tok);
    string contents = this->contents();
    contents = to_lower(contents);
    new_token->set_lexeme(contents);
    // clear buffer
    this->clear_buffer();
    // return new token
    this->consume(new_token);
    return new_token;
}

TokenPtr create_token(TokType token, string lexeme,
                      unsigned int line, unsigned int column) {
    // create a cool token
    return TokenPtr(new Token(token, lexeme, line, column));
}

int Scanner::get_char() {
	// look at the current char in the input
	if (this->file_ptr != this->file_buf_ptr->end())
		// if not end, character is valid
		return *(this->file_ptr);
	else
        // null character is buffer end
		return '\0';
}

void Scanner::cache() {
    // get char and push onto the buffer
    char c = get_char();
    this->scan_buf->push_back(c);
}

bool Scanner::forward() {
	// move the file pointer to the right by one
	if (this->file_ptr != this->file_buf_ptr->end()) {
		// deal with line and column numbers
		if (this->get_char() == '\n') {
			this->line_number++;
			this->col_number = 1;
		} else {
			this->col_number++;
		}
        // move forward
        this->file_ptr++;
		return true;
	} else {
		return false;
    }
}

bool Scanner::rewind() {
	// move the file pointer to the left by one
	if (this->file_ptr != this->file_buf_ptr->begin()) {
        // decreased the file ptr
        this->file_ptr--;
        if (this->get_char() == '\n') {
            // deal with the remaining column size
            this->line_number--;
            // save the file pointer
            string::iterator saved = this->file_ptr;
            unsigned int col_size = 0;
            char current = '\0';
            // rewind until we reach the beginning
            // of file or beginning of line
            do {
                this->file_ptr--;
                col_size++;
                current = this->get_char();
            } while (current != '\0' && current != '\n');
            // if we're at the beginning of the file
            // then we increment the line number by 1
            if (current == '\0') {
                this->line_number++;
            }
            // restore the saved file pointer
            this->file_ptr = saved;
            // restore the saved line number
            this->col_number = col_size;
        } else {
            this->col_number--;
        }
		return true;
	} else {
		return false;
    }
}

void Scanner::goto_next(char c) {
    // go to the next character specified
    while(this->get_char() != c) {
        // go forward
        this->forward();
    }
}

void Scanner::load_keyword_machines() {
	// get the token type for all keyworded or single char types
	for (auto i = (int) TokType::MP_SEMI_COLON;
         i <= (int) TokType::MP_BOOLEAN; i++) {
		
        // create new automata for all types
		FSMachinePtr new_fa = unique_ptr<
        FiniteAutomataContainer>(
        new FiniteAutomataContainer(get_token_info((TokType)i).first, true));
		
        // build the keyword into the automata
		new_fa->build_keyword(get_token_info((TokType)i).second);
        
        // set priority: keywords are always more important than ids
        new_fa->set_priority(1);
		
        // add to the vector of automata
		this->fsmachines->push_back(new_fa);
	}
}

// load automata for scanning valid identifiers
void Scanner::load_id_machine() {
	// create the id scan automata
	auto id_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_ID).first,
    true));
    
	// detects ids without two underscores
	id_machine->add_state("0", true, false);
	id_machine->add_state("1", false, true);
	id_machine->add_state("2", false, true);
	id_machine->add_state("3", false, false);
	id_machine->add_state("4", false, false);
    
	// add transitions
	id_machine->add_alphabet("0", "1");
	id_machine->add_alphabet("1", "1");
	id_machine->add_digits("1", "1");
	id_machine->add_transition("1", '_', "4");
	id_machine->add_alphabet("4", "1");
	id_machine->add_digits("4", "1");
    
	// other part
	id_machine->add_transition("0", '_', "3");
	id_machine->add_alphabet("3", "2");
	id_machine->add_digits("3", "2");
	id_machine->add_alphabet("2", "2");
	id_machine->add_digits("2", "2");
	id_machine->add_transition("2", '_', "3");
    
    // set priority
    id_machine->set_priority(2);
    
    // add to finite machines
    this->fsmachines->push_back(id_machine);
}

void Scanner::load_num_machines() {
	// create floating point automata for scanning numeric literals
	auto float_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_FLOAT_LITERAL).first, true));
    
    // add states to this DFA, first and final
    float_machine->add_state("0", true, false);
	float_machine->add_state("1", false, false);
	float_machine->add_state("2", false, true);
	float_machine->add_state("3", false, false);
	float_machine->add_state("4", false, false);
	float_machine->add_state("5", false, true);
    float_machine->add_digits("0", "1");
    float_machine->add_transition("0", '-', "1");
	float_machine->add_digits("1", "1");
	float_machine->add_transition("1", '.', "2");
	float_machine->add_digits("2", "2");
	float_machine->add_transition("2", 'e', "3");
	float_machine->add_transition("2", 'E', "3");
	float_machine->add_transition("3", '+', "4");
	float_machine->add_transition("3", '-', "4");
	float_machine->add_digits("3", "5");
	float_machine->add_digits("4", "5");
    
	// create integer automata
	auto integer_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_INT_LITERAL).first, true));
    
    // add states to this DFA
	integer_machine->add_state("0", true, false);
	integer_machine->add_state("1", false, true);
    integer_machine->add_transition("0", '-', "1");
	integer_machine->add_digits("0", "1");
	integer_machine->add_digits("1", "1");
    
	// reset both
	float_machine->reset();
	integer_machine->reset();
    
    // set machine priorities
    float_machine->set_priority(1);
    integer_machine->set_priority(1);
    
    // add both to the finite machines list
    this->fsmachines->push_back(float_machine);
    this->fsmachines->push_back(integer_machine);
}

void Scanner::load_strand_machines(unsigned int within) {
    // comments and strings, take that!
    // add nested strings and comments feature using within variable
    auto comment_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_COMMENT).first, true));
    
    // add states
    comment_machine->add_state("0", true, false);
    comment_machine->add_state("1", false, false);
    comment_machine->add_state("2", false, true);
    
    // for state 1
    comment_machine->add_alphabet("1", "1");
    comment_machine->add_symbols("1", "1");
    comment_machine->add_digits("1", "1");
    comment_machine->add_transition("1", ' ', "1");
    comment_machine->add_transition("1", '\n', "1");
    comment_machine->remove_transition("1", '{');
    comment_machine->remove_transition("1", '}');
    comment_machine->add_transition("0", '{', "1");
    comment_machine->add_transition("1", '}', "2");
    
    // new machine
    auto string_machine = FSMachinePtr(
    new FiniteAutomataContainer(get_token_info(TokType::MP_STRING_LITERAL).first, true));
    
    // add states
    string_machine->add_state("0", true, false);
    string_machine->add_state("1", false, false);
    string_machine->add_state("2", false, true);
    
    // add transitions
    string_machine->add_transition("0", '\'', "1");
    string_machine->add_alphabet("1", "1");
    string_machine->add_symbols("1", "1");
    string_machine->add_digits("1", "1");
    string_machine->add_transition("1", ' ', "1");
    string_machine->remove_transition("1", '\'');
    string_machine->remove_transition("1", '\'');
    string_machine->add_transition("1", '\'', "2");
    
    // set machine priority
    comment_machine->set_priority(1);
    string_machine->set_priority(1);
    
    // add the machines to our FSMachines list
    this->fsmachines->push_back(comment_machine);
    this->fsmachines->push_back(string_machine);
}

bool Scanner::isalnum(char next) {
    // load automata for determining if
    // an input symbol is a letter or digit
	if (isalpha(next) || isdigit(next))
		return true;
	else
		return false;
}

bool Scanner::isnum(char next) {
	// step over a digit
	FSMachinePtr is_digit = unique_ptr<
    FiniteAutomataContainer>(
    new FiniteAutomataContainer("DIGIT", true));
    
    // add states
	is_digit->add_state("0", true, false);
	is_digit->add_state("1", false, true);
	is_digit->add_digits("0", "1");
    
	// reset to start
	is_digit->reset();
    
	// step
	is_digit->step(next);
    
	// determine acceptance
	if (is_digit->accepting() == true) {
		return true;
	} else {
		return false;
	}
}

bool Scanner::isalpha(char next) {
    // step over a letter
	FSMachinePtr is_letter = unique_ptr<
    FiniteAutomataContainer>(
    new FiniteAutomataContainer("LETTER", true));
    
    // add states
	is_letter->add_state("0", true, false);
	is_letter->add_state("1", false, true);
	is_letter->add_alphabet("0", "1");
    
	// reset
	is_letter->reset();
    
	// step on
	is_letter->step(next);
    
	// determine acceptance
	if (is_letter->accepting() == true) {
		return true;
	} else {
		return false;
	}
}

unsigned long Scanner::get_line_number() {
	// get the scanning line number
	return this->line_number;
}

unsigned long Scanner::get_col_number() {
	// get the scanning column number
	return this->col_number;
}

// remove comments from the list of consumed tokens
void Scanner::thin_comments() {
    if (this->consumed->size() > 0) {
        for (vector<TokenPtr>::iterator i =
        this->consumed->begin();
        i != consumed->end(); i++)
        {
            if ((*i)->get_token() == MP_COMMENT) {
                this->consumed->erase(i);
            }
        }
    }
}

// write tokens to a file
void Scanner::write_tokens_tof(string filename) {
	if (this->consumed->size() > 0) {
		ofstream file;
		file.open(filename);
		for (vector<TokenPtr>::iterator i =
             this->consumed->begin();
             i != this->consumed->end(); i++) {
			file << (*(*i)->to_string()) << '\n';
		}
		file.close();
	}
}

void Scanner::display_tokens() {
	for (vector<TokenPtr>::iterator i = this->consumed->begin();
         i != consumed->end(); i++) {
		report_error_lc("Found", "'" + (*i)->get_lexeme()
                        + "' : " + get_token_info((*i)->get_token()).first, (*i)->get_line(), (*i)->get_column());
	}
}

TokenListPtr Scanner::detach_tokens() {
	return this->consumed;
}

