/*
 * Scanner.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#include "Scanner.hpp"
#define PRINT_DEBUG 0

Scanner::Scanner(shared_ptr<Input> input_ptr) {
    // ctor
	this->input_ptr = input_ptr;
	this->string_ptr = input_ptr->detach_input();
    this->found_tokens = shared_ptr<vector<shared_ptr<Token>>>(new vector<shared_ptr<Token>>);
	this->keyword_automata = shared_ptr<
			vector<shared_ptr<FiniteAutomataContainer>>>(new vector<shared_ptr<FiniteAutomataContainer>>);
	this->load_keyword_automata();
    this->load_ld_automata();
	this->col_number = 0;
	this->line_number = 0;
    this->file_ptr = this->get_begin_fp();
}

Scanner::~Scanner() {
    // dtor
	this->input_ptr.reset();
	this->string_ptr.reset();
    this->found_tokens.reset();
    this->keyword_automata.reset();
    this->is_digit.reset();
    this->is_letter.reset();
}

string::iterator Scanner::get_begin_fp() {
    // get a pointer to the start of the file
	return this->string_ptr->begin();
}

string::iterator Scanner::get_end_fp() {
    // get a pointer to the end of the file
	return this->string_ptr->end();
}

void Scanner::set_fp_begin() {
    // push the file pointer back to the input's beginning
	this->file_ptr = this->string_ptr->begin();
}

void Scanner::reset() {
	// reset all FSAs, delete all tokens, move file pointer to beginning
	this->reset_all_kword_automata();
	this->keyword_automata->clear();
	this->set_fp_begin();
}

void Scanner::reset_all_kword_automata() {
    // reset all automata to their initial state
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
			this->keyword_automata->begin(); i != this->keyword_automata->end();
			i++) {
		if (i != this->keyword_automata->end())
			(*i)->reset();
	}
}

void Scanner::step_all_kword(char next) {
	// run character 'next' on all FSAs
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
			this->keyword_automata->begin(); i != this->keyword_automata->end();
			i++) {
		if (i != this->keyword_automata->end())
			(*i)->step(next);
	}
}

bool Scanner::check_any_kword_accepted() {
	// check if any keyword FSA has accepted
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
			this->keyword_automata->begin(); i != this->keyword_automata->end();
			i++) {
		if (i != this->keyword_automata->end()) {
			if ((*i)->accepted())
				return true;
		}
	}
	return false;
}

TokType Scanner::get_first_accepted_token() {
	// get the first accepted token if available from an FSA
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
				this->keyword_automata->begin(); i != this->keyword_automata->end();
				i++) {
		if ((*i)->accepted()) {
			// names of the tokens are aligned with their enum names
			// which are the names of each created FSA
			return get_token_by_name((*i)->get_name());
		}
	}
	return (TokType) 0;
}

void Scanner::scan_all() {
    while(this->file_ptr != this->string_ptr->end()) {
        scan_one();
    }
}

shared_ptr<Token> Scanner::scan_one() {
	// tokenize the input until done
	// initialize a vector to hold all token characters
	vector<char> scan_buf;
    
    // debug printer for the scanner
    if (PRINT_DEBUG) {
        static int iteration = 0;
        cout << "Iteration: " << iteration << endl;
        cout << "-" << endl;
        this->display_tokens();
        cout << "-" << endl;
        iteration++;
    }
    
	// VIRTUAL CONTRACT CLAUSE ADHERENCE: Always stop at the beginning of the next token.
	// token, move there at the end of every case
	// using this->right();

	// basic cases for comments, whitespace, and newlines
	// remove whitespace
	if (this->peek() == get_token_info(TokType::MP_NULLCHAR).second[0]) {
		// create token
		shared_ptr<Token> null_tok = shared_ptr<Token>(
				new Token(this->get_line_number(), this->get_col_number(),
						TokType::MP_NULLCHAR, "\0"));
		// push back and return
		this->found_tokens->push_back(null_tok);
		return *(this->found_tokens->end() - 1);
	}
	if (this->peek() == ' ') {
		// ignore whitespace
        // move the file pointer ahead 1 (as per contract)
		this->right();
	}
	// check for a new line
	else if (this->peek() == '\n') {
		// ignore, (line number managed in right() and left())
        // move the file pointer ahead 1 (as per contract)
		this->right();
	}
	// remove double dash comments
	else if (this->peek() == get_token_info(TokType::MP_DIV).second[0]
			&& (this->next() == get_token_info(TokType::MP_DIV).second[0]
					&& this->next() != '\0')) {
                // new line comment, so ends at the next line
            while (this->peek() != '\n') {
                // push the current char on to the scan buffer
                scan_buf.push_back(this->peek());
                // move the file pointer to the right a tad
                this->right();
                }
                // convert input stream to a string
                string buf_lexeme = string(scan_buf.begin(), scan_buf.end());
                // create the token
                shared_ptr<Token> comment_tok = shared_ptr<Token>(
                                                                  new Token(this->get_line_number(), this->get_col_number(),
                                                                            TokType::MP_COMMENT, buf_lexeme));
                // push token onto the list of found tokens
                found_tokens->push_back(comment_tok);
                // move the file pointer ahead 1 (as per contract)
                this->right();
                // return the comment
                return comment_tok;
	}
	// remove bracket comments
	else if (this->peek() == get_token_info(TokType::MP_BRACKET_LEFT).second[0]) {
		// push back the left bracket (lexeme) and
		// wait for the second bracket to appear under the file pointer
		while (this->peek()
				!= get_token_info(TokType::MP_BRACKET_RIGHT).second[0]) {
			// push back the current bracket
			scan_buf.push_back(this->peek());
			// move the file pointer ahead 1 (as per contract)
			this->right();
		}
		// once done push back a right bracket (lexeme)
		scan_buf.push_back(get_token_info(TokType::MP_BRACKET_RIGHT).second[0]);
		// convert input stream to a string
		string buf_lexeme = string(scan_buf.begin(), scan_buf.end());
		// create the token
		shared_ptr<Token> comment_tok = shared_ptr<Token>(
				new Token(this->get_line_number(), this->get_col_number(),
						TokType::MP_COMMENT, buf_lexeme));
		// save the token
		this->found_tokens->push_back(comment_tok);
		// clear the scan buffer
		scan_buf.clear();
		// move the file pointer ahead 1 (as per contract)
		this->right();
		// return the last token
		return *(this->found_tokens->end() - 1);
	}

	// now look for other tokens
	else {
		// keyword tokens
		this->reset_all_kword_automata();
		// until the end of the token has been reached
		while (true) {
			// get the first character
			char look = this->peek();
			// step all FSAs by this character
			this->step_all_kword(look);
			// push the character onto the temp token buffer
			scan_buf.push_back(look);
			// check to see if any automata has accepted
			// if this is the case, break
			if (this->check_any_kword_accepted() == true) {
				// get accepted keyword
				TokType kword = this->get_first_accepted_token();
				// convert input stream to a string
				string buf_lexeme = string(scan_buf.begin(), scan_buf.end());
				// generate token
				shared_ptr<Token> new_kword = shared_ptr<Token>(new Token(this->get_line_number(), this->get_col_number(), kword, buf_lexeme));
				// save token
				this->found_tokens->push_back(new_kword);
				// move the file pointer ahead by 1 (as per contract)
				this->right();
				// we're done here
				break;
			}
			// move the file pointer ahead 1 (as per contract)
			this->right();
		}
		// clear the scan buffer, since we're done with it
		scan_buf.clear();
	}
	// error case token (shouldn't happen, yikes!)
	return shared_ptr<Token>(
			new Token(this->get_line_number(), this->get_col_number(),
					TokType::MP_NULLCHAR, "INVALID"));
}

char Scanner::peek() {
// look at the current char in the input
	if (this->file_ptr != this->string_ptr->end())
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
	if (this->file_ptr != this->string_ptr->end()) {
		// deal with line and column numbers
		if (*this->file_ptr == '\n') {
			this->line_number++;
			this->col_number = 1;
		} else {
			this->col_number++;
		}
		// successful
		this->file_ptr++;
		return true;
	} else
		// unsuccessful
		return false;
}

bool Scanner::right_two() {
	// go to the right two, if you went back
	// starts cleanly at the next token
	bool success = true;
	for (auto i = 0; i < 2; i++)
		success &= this->right();
	return success;
}

bool Scanner::left() {
// move the file pointer to the left by one
	if (this->file_ptr != this->string_ptr->begin()) {
		// deal with line and column numbers
		if (*this->file_ptr == '\n') {
			this->line_number--;
			// determine the length of the last line
			// unimplemented, expect shady results
			// this->col_number = 1;
		} else {
			this->col_number--;
		}
		// successful
		this->file_ptr--;
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

void Scanner::load_keyword_automata() {
	// get the token type for all keyworded or single char types
	for (TokType i = TokType::MP_SEMI_COLON; i <= TokType::MP_BOOLEAN; i++) {
		// create new automata for all types
		shared_ptr<FiniteAutomataContainer> new_fa = shared_ptr<
				FiniteAutomataContainer>(
				new FiniteAutomataContainer(get_token_info(i).first, true));
		// build the keyword into the automata
		new_fa->build_keyword(get_token_info(i).second);
		// add to the vector of automata
		this->keyword_automata->push_back(new_fa);
	}
}

void Scanner::load_id_automata() {
	// create the id scan automata
	// NOT FINISHED.
	this->id_automata = shared_ptr<FiniteAutomataContainer>(
			new FiniteAutomataContainer(get_token_info(TokType::MP_ID).first,
					true));
}

void Scanner::load_numerical_automata() {
	// unimplemented
}

void Scanner::load_ld_automata() {
	// load automata that decides whether or not a character is a digit
	// operation is performed only once before dead state
	// so automata must be reset on every run
	this->is_digit = shared_ptr<FiniteAutomataContainer>(
			new FiniteAutomataContainer("IS_DIGIT", true));
	this->is_digit->add_state("0", true, false);
	this->is_digit->add_state("1", false, true);
	this->is_digit->add_digits("0", "1");
	// load automata that decides whether or not a character is a letter
	// operation is performed only once before dead state
	// so automata must be reset on every run
	this->is_letter = shared_ptr<FiniteAutomataContainer>(
			new FiniteAutomataContainer("IS_LETTER", true));
	this->is_letter->add_state("0", true, false);
	this->is_letter->add_state("1", false, true);
	this->is_letter->add_alphabet("0", "1");
	// reset both
	this->is_letter->reset();
	this->is_digit->reset();
}

int Scanner::get_line_number() {
	// get the scanning line number
	return this->line_number;
}

int Scanner::get_col_number() {
	// get the scanning column number
	return this->col_number;
}

void Scanner::write_tokens_tof(string filename) {
	// unimplemented
}

void Scanner::display_tokens() {
	for (vector<shared_ptr<Token>>::iterator i = this->found_tokens->begin();
			i != found_tokens->end(); i++) {
		shared_ptr<string> string_rep = (*i)->to_string();
		cout << *string_rep << endl;
	}
}

