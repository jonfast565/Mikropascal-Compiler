/*
 * Tests.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#ifndef tests
#define tests

#include "Standard.hpp"
#include "FiniteAutomata.hpp"
#include "Input.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"

int automata_keyword_test_cases() {
	cout << "--- Automata Keyword Tests ---" << endl;
	shared_ptr<FiniteAutomataContainer> while_dfa = shared_ptr<
			FiniteAutomataContainer>(
			new FiniteAutomataContainer("MP_WHILE", true));
	while_dfa->build_keyword("while");
	while_dfa->print();

	// accepts or rejects?
	vector<string> test_strings = { "while", "xxxxxwhile", "whilethewavescrash",
			"somewhile" };
	string accept_string;
	for (vector<string>::iterator i = test_strings.begin();
			i != test_strings.end(); i++) {
		while_dfa->run(*i);
		accept_string = while_dfa->accepted() ? "Accepted." : "Rejected.";
		cout << accept_string << endl;
		while_dfa->reset();
	}
	cout << "--- End ---" << endl;
	return 0;
}

int automata_test_cases() {
	cout << "--- Automata Tests ---" << endl;

	// try out a DFA
	shared_ptr<FiniteAutomataContainer> while_dfa = shared_ptr<
			FiniteAutomataContainer>(
			new FiniteAutomataContainer("WHILE Token", true));
	while_dfa->add_state("q0", true, false);
	while_dfa->add_state("q1", false, false);
	while_dfa->add_state("q2", false, false);
	while_dfa->add_state("q3", false, false);
	while_dfa->add_state("q4", false, false);
	while_dfa->add_state("q5", false, true);
	while_dfa->add_alphabet("q0", "q0");
	while_dfa->remove_transition("q0", 'w');
	while_dfa->add_transition("q0", 'w', "q1");
	while_dfa->add_transition("q1", 'h', "q2");
	while_dfa->add_transition("q2", 'i', "q3");
	while_dfa->add_transition("q3", 'l', "q4");
	while_dfa->add_transition("q4", 'e', "q5");
	while_dfa->print();

	// accepts or rejects?
	vector<string> test_strings = { "while", "xxxxxwhile", "whilethewavescrash",
			"somewhile" };
	string accept_string;
	for (vector<string>::iterator i = test_strings.begin();
			i != test_strings.end(); i++) {
		while_dfa->run(*i);
		accept_string = while_dfa->accepted() ? "Accepted." : "Rejected.";
		cout << accept_string << endl;
		while_dfa->reset();
	}

	cout << "--- End ---" << endl;
	return 0;
}

int input_test_cases(string path) {
	cout << "--- Input Tests ---" << endl;
	shared_ptr<Input> test_input = Input::open_file(path);
	test_input->print_input();
	cout << "--- End ---" << endl;
	return 0;
}

int scanner_test(string filename) {
	cout << "--- Scanner Test ---" << endl;
	shared_ptr<Input> test_input = Input::open_file(filename);
	shared_ptr<Scanner> scanner = shared_ptr<Scanner>(new Scanner(test_input));
	scanner->scan_all();
	scanner->display_tokens();
	scanner->write_tokens_tof(string(filename + "_tokens.txt"));
	cout << "--- End ---" << endl;
	return 0;
}

int parser_test(string filename) {
	cout << "--- Parser Test ---" << endl;
	shared_ptr<Input> test_input = Input::open_file(filename);
	shared_ptr<Scanner> scanner = shared_ptr<Scanner>(new Scanner(test_input));
	shared_ptr<Parser> parser = shared_ptr<Parser>(new Parser(scanner));
	parser->parse_me();
    cout << "--- End ---" << endl;
	return 0;
}

#endif
