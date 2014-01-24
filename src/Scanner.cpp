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
	this->string_ptr = input_ptr->detach_input();
	this->found_tokens = shared_ptr<vector<shared_ptr<Token>>>(new vector<shared_ptr<Token>>);
	this->keyword_automata = shared_ptr<
			vector<shared_ptr<FiniteAutomataContainer>>>(new vector<shared_ptr<FiniteAutomataContainer>>);
	this->load_keyword_automata();
	this->load_ld_automata();
	this->load_id_automata();
	this->load_numerical_automata();
	this->col_number = 0;
	this->line_number = 0;
	this->file_ptr = this->get_begin_fp();
}

// destructor, releases all objects
Scanner::~Scanner() {
	// dtor
	this->input_ptr.reset();
	this->string_ptr.reset();
	this->found_tokens.reset();
	this->keyword_automata.reset();
	this->is_digit.reset();
	this->is_letter.reset();
}

// get an iterator that returns the beginning of the input stream
string::iterator Scanner::get_begin_fp() {
	// get a pointer to the start of the file
	return this->string_ptr->begin();
}

// get an iterator that goes to the end of the input stream
string::iterator Scanner::get_end_fp() {
	// get a pointer to the end of the file
	return this->string_ptr->end();
}

// set the file pointer to the beginning of the input stream
void Scanner::set_fp_begin() {
	// push the file pointer back to the input's beginning
	this->file_ptr = this->string_ptr->begin();
}

// reset
void Scanner::reset() {
	// reset all FSAs, delete all tokens, move file pointer to beginning
	this->reset_all_kword_automata();
	this->keyword_automata->clear();
	this->set_fp_begin();
}

// re-initialize all automata dealing with keyword matching
void Scanner::reset_all_kword_automata() {
	// reset all automata to their initial state
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
			this->keyword_automata->begin(); i != this->keyword_automata->end();
			i++) {
		if (i != this->keyword_automata->end())
			(*i)->reset();
	}
}

// step one character over all automata currently initialized
void Scanner::step_all_kword(char next) {
	// run character 'next' on all FSAs
	for (vector<shared_ptr<FiniteAutomataContainer>>::iterator i =
			this->keyword_automata->begin(); i != this->keyword_automata->end();
			i++) {
		if (i != this->keyword_automata->end())
			(*i)->step(next);
	}
}

// find any keyword that has accepted
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

// get the first accepted token
TokType Scanner::get_first_accepted_kword() {
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

// scan over all token until the file pointer reaches the end of file
void Scanner::scan_all() {
	while (this->file_ptr != this->string_ptr->end()) {
		scan_one();
	}
}

// scan over one token (this is the driver method!!!... crickey critical mate)
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
	else if (this->peek()
			== get_token_info(TokType::MP_BRACKET_LEFT).second[0]) {
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

	// now look for other tokens that are actually important
	else {
		// keyword tokens
		this->reset_all_kword_automata();
		// until the end of the token has been reached
		while (true) {
			// get the first character
			char look = this->peek();
			// step all FSAs by this character
			this->step_all_kword(look);
			this->step_id(look);
			// push the character onto the temp token buffer
			scan_buf.push_back(look);
			// check to see if any automata has accepted
			// if this is the case, break
			if (this->check_any_kword_accepted() == true) {
				// get accepted keyword
				TokType kword = this->get_first_accepted_kword();
				// worry about special cases (these are rather discriminating
				// for Mikropascal it seems)
				// boolean operations with two symbols (a bit hacky, so generalize later)
				if (kword == MP_EQUALS | kword == MP_LESSTHAN
						| kword == MP_GREATERTHAN | kword == MP_COLON) {
					// one char of lookahead required to ascertain whether or not
					// we can ascertain the token type
					vector<string> match_letters;
					match_letters.push_back("=>");
					pair<TokType, vector<char>> lk_for_binary_op = this->quick_lookahead_kword(kword, match_letters);
					kword = lk_for_binary_op.first;
					scan_buf.insert(scan_buf.end(), lk_for_binary_op.second.begin(), lk_for_binary_op.second.end());
					// readln and writeln
				} else if (kword == MP_READ | kword == MP_WRITE) {
					// two chara of lookahead required to ascertain whether or not
					// we can ascertain the token type
					vector<string> match_letters;
					match_letters.push_back("l");
					match_letters.push_back("n");
					pair<TokType, vector<char>> lk_for_binary_op = this->quick_lookahead_kword(kword, match_letters);
					kword = lk_for_binary_op.first;
					scan_buf.insert(scan_buf.end(), lk_for_binary_op.second.begin(), lk_for_binary_op.second.end());
				}
				// convert input stream to a string
				string buf_lexeme = string(scan_buf.begin(), scan_buf.end());
				// generate token
				shared_ptr<Token> new_kword = shared_ptr<Token>(
						new Token(this->get_line_number(),
								this->get_col_number(), kword, buf_lexeme));
				// save token
				this->found_tokens->push_back(new_kword);
				// move the file pointer ahead by 1 (as per contract)
				this->right();
				// we're done here, so we can return the new token
				return new_kword;
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
					TokType::MP_MALFORMED, "INVALID"));
}

// lookahead for multi-char operators
pair<TokType, vector<char>> Scanner::quick_lookahead_kword(TokType old_type,
		vector<string> looking_for) {
	// save a vector of the lookahead chars found that were successful
	// also, save a new token type, if applicable.
	// finally, we need to determine if a new type was in fact found
	// and if not, make sure we don't break things
	vector<char> lookahead_chars_found;
	TokType new_type;
	bool new_type_found = false;
	// variable arguments (vector) indicate levels of lookahead (1, 2, 10 chars)
	// strings indicate what letters should be looked for at each level
	for (vector<string>::iterator i = looking_for.begin(); i != looking_for.end(); i++) {
		// get the string for lookahead level
		string lookahead_chooser = *i;
		// determine if there is a lookahead character in the input string that
		// satisfies the character we are looking for
		for (string::iterator j = lookahead_chooser.begin(); j != lookahead_chooser.end(); j++) {
			if (this->next() == *j) {
				// step everything another time
				this->step_all_kword(*j);
				lookahead_chars_found.push_back(*j);
				// move right once
				this->right();
			}
		}
		// check if a keyword has been accepted for this level
		if (this->check_any_kword_accepted() == true) {
			new_type = this->get_first_accepted_kword();
			new_type_found = true;
		}
		// else keep what was the original
		// or perhaps malformed... error case can go here later
	}
	// done
	// indicate the return type based on what was found after investigation
	if (new_type_found)
		return pair<TokType, vector<char>>(new_type, lookahead_chars_found);
	else
		return pair<TokType, vector<char>>(old_type, lookahead_chars_found);
}

// look at the current character under the file pointer
char Scanner::peek() {
	// look at the current char in the input
	if (this->file_ptr != this->string_ptr->end())
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

// move the file pointer to the right twice (deprecated)
bool Scanner::right_two() {
	// go to the right two, if you went back
	// starts cleanly at the next token
	bool success = true;
	for (auto i = 0; i < 2; i++)
		success &= this->right();
	return success;
}

// move the file pointer left
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

// load automata for scanning valid identifiers
void Scanner::load_id_automata() {
	// create the id scan automata
	this->id_automata = shared_ptr<FiniteAutomataContainer>(
			new FiniteAutomataContainer(get_token_info(TokType::MP_ID).first,
					true));
	// build custom id automata (based on homework)
	this->id_automata->add_state("0", true, false);
	this->id_automata->add_state("1", false, true);
	this->id_automata->add_state("2", false, true);
	this->id_automata->add_state("3", false, false);
	this->id_automata->add_state("4", false, false);
	// add transitions
	this->id_automata->add_alphabet("0", "1");
	this->id_automata->add_alphabet("1", "1");
	this->id_automata->add_digits("1", "1");
	this->id_automata->add_transition("1", '_', "4");
	this->id_automata->add_alphabet("4", "1");
	this->id_automata->add_digits("4", "1");
	// other part
	this->id_automata->add_transition("0", '_', "3");
	this->id_automata->add_alphabet("3", "2");
	this->id_automata->add_digits("3", "2");
	this->id_automata->add_alphabet("2", "2");
	this->id_automata->add_digits("2", "2");
	this->id_automata->add_transition("2", '_', "3");
}

// check if the identifier is accepted
bool Scanner::check_id_accepted() {
	return this->id_automata->accepted();
}

// step through the identifier FA on char
void Scanner::step_id(char next) {
	this->id_automata->step(next);
}

// reset identifier FA
void Scanner::reset_id() {
	this->id_automata->reset();
}

// load automata for scanning numeric literals
void Scanner::load_numerical_automata() {
	// unimplemented
}

// load automata for determining if an input symbol is a letter or digit
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

// step over a letter
void Scanner::step_letter(char next) {
	this->is_letter->step(next);
}

// step over a digit
void Scanner::step_digits(char next) {
	this->is_digit->step(next);
}

// accept a letter
bool Scanner::accept_letter() {
	return this->is_letter->accepted();
}

// accept a digit
bool Scanner::accept_digit() {
	return this->is_digit->accepted();
}

// get the line number
int Scanner::get_line_number() {
	// get the scanning line number
	return this->line_number;
}

// get the column number
int Scanner::get_col_number() {
	// get the scanning column number
	return this->col_number;
}

// write tokens to a file
void Scanner::write_tokens_tof(string filename) {
// unimplemented
}

// display tokens on the screen
void Scanner::display_tokens() {
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++) cout << "-";
	cout << endl;
	stringstream ss;
	ss << setw(1) << std::left << "| ";
	ss << setw(25) << std::left << "Token:";
    ss << setw(10) << std::left << "Line:";
	ss << setw(10) << std::left << "Column:";
    ss << setw(30) << std::left << "Lexeme:";
	cout << ss.str() << endl;
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++) cout << "-";
	cout << endl;
	for (vector<shared_ptr<Token>>::iterator i = this->found_tokens->begin();
			i != found_tokens->end(); i++) {
		shared_ptr<string> string_rep = (*i)->to_string();
		cout << "| " << *string_rep << endl;
	}
	cout << "+";
	for (auto i = 0; i < 31 + 20 + 25; i++) cout << "-";
	cout << endl;
}

