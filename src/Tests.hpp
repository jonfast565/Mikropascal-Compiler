/*
 * Tests.hpp
 *
 *  Created on: Jan 21, 2014
 *      Author: jonfast
 */

#ifndef tests
#define tests

#include "Standard.hpp"

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
	shared_ptr<Input> test_input = Input::try_get_input(path);
	test_input->print_input();
	cout << "--- End ---" << endl;
	return 0;
}

#endif
