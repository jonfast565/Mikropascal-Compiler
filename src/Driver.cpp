/*
 * Driver.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: Jon
 */

#include "Standard.hpp"
#include "Input.hpp"
#include "FiniteAutomata.hpp"

// prototypes
int automata_test_cases();
int input_test_cases(string);
int main(int, char*[]);

int main(int argc, char* argv[]) {
	// test cases
	automata_test_cases();
	input_test_cases("/Users/jonfast/Desktop/program.pas");
	getchar();
	return EXIT_SUCCESS;
}

int input_test_cases(string path) {
	cout << "--- Input Tests ---" << endl;
	Input* test_input = Input::try_get_input(path);
	test_input->print_input();
	cout << "--- End ---" << endl;
	return 0;
}

int automata_test_cases() {
	cout << "--- Automata Tests ---" << endl;

	// try out a DFA
	shared_ptr<FiniteAutomataContainer> while_dfa = shared_ptr<FiniteAutomataContainer>(new FiniteAutomataContainer("WHILE Token", true));
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
	vector<string> test_strings = {"while", "xxxxxwhile", "whilethewavescrash", "somewhile"};
	string accept_string;
	for (vector<string>::iterator i = test_strings.begin(); i != test_strings.end(); i++) {
		while_dfa->run(*i);
		accept_string = while_dfa->in_accepting_state() ? "Accepted." : "Rejected.";
		cout << accept_string << endl;
		while_dfa->reset();
	}

	cout << "--- End ---" << endl;
	return 0;
}




