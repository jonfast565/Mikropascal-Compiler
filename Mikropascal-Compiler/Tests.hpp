#ifndef tests
#define tests

#include "Standard.hpp"
#include "FiniteAutomata.hpp"
#include "Input.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Helper.hpp"
#include "Symbols.hpp"
#include "SemanticAnalyzer.hpp"

int automata_keyword_test_cases() {
	cout << "[ Automata Keyword Tests ]" << endl;
	shared_ptr<FiniteMachineContainer> while_dfa = shared_ptr<
    FiniteMachineContainer>(
                             new FiniteMachineContainer("MP_WHILE", true));
	while_dfa->build_keyword("while");
	while_dfa->print();
    
	// accepts or rejects?
	vector<string> test_strings = { "while", "xxxxxwhile", "whilethewavescrash",
        "somewhile" };
	string accept_string;
	for (vector<string>::iterator i = test_strings.begin();
         i != test_strings.end(); i++) {
		while_dfa->run(*i);
		accept_string = while_dfa->accepting() ? "Accepted." : "Rejected.";
		cout << accept_string << endl;
		while_dfa->reset();
	}
	cout << "[ End ]" << endl;
	return 0;
}

int automata_test_cases() {
	cout << "[ Automata Tests ]" << endl;
    
	// try out a DFA
	shared_ptr<FiniteMachineContainer> while_dfa = shared_ptr<
    FiniteMachineContainer>(
                             new FiniteMachineContainer("WHILE Token", true));
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
		accept_string = while_dfa->accepting() ? "Accepted." : "Rejected.";
		cout << accept_string << endl;
		while_dfa->reset();
	}
    
	cout << "[ End ]" << endl;
	return 0;
}

int input_test_cases(string path) {
	cout << "[ Input Tests ]" << endl;
	shared_ptr<Input> test_input = Input::open_file(path);
	test_input->print_input();
	cout << "[ End ]" << endl;
	return 0;
}

int scanner_test(string filename) {
	cout << "[ Scanner Test ]" << endl;
	shared_ptr<Input> test_input = Input::open_file(filename);
    if (test_input != nullptr) {
        shared_ptr<Scanner> scanner = shared_ptr<Scanner>(new Scanner(test_input));
        TokenPtr t;
        do {
            t = scanner->scan_one();
            if (t == nullptr) {
                report_error("Scan Error", "Token returned was invalid");
            }
        } while (t->get_token() != MP_EOF);
        scanner->display_tokens();
    } else {
        return -1;
    }
	cout << "[ End ]" << endl;
	return 0;
}

int parser_test(string filename) {
	cout << "[ Parser Test ]" << endl;
	shared_ptr<Input> input = Input::open_file(filename);
    if (input != nullptr) {
        shared_ptr<Scanner> scanner = shared_ptr<Scanner>(new Scanner(input));
        shared_ptr<SemanticAnalyzer> analyzer = shared_ptr<SemanticAnalyzer>(
                                                                         new SemanticAnalyzer(filename));
        shared_ptr<Parser> parser = shared_ptr<Parser>(new Parser(scanner, analyzer));
        parser->parse();
        parser->get_analyzer()->get_ast()->display_tree();
    }
    cout << "[ End ]" << endl;
	return 0;
}

int symbol_test(string filename) {
	cout << "[ Symbol Table Test ]" << endl;
	shared_ptr<Input> input = Input::open_file(filename);
    if (input != nullptr) {
        shared_ptr<Scanner> scanner = shared_ptr<Scanner>(new Scanner(input));
        shared_ptr<SemanticAnalyzer> analyzer = shared_ptr<SemanticAnalyzer>(
                                                                         new SemanticAnalyzer(filename));
        shared_ptr<Parser> parser = shared_ptr<Parser>(new Parser(scanner, analyzer));
        parser->parse();
        parser->get_analyzer()->print_symbols();
    }
    cout << "[ End ]" << endl;
	return 0;
}

int compile_chain(string filename) {
    cout << "[ Compiling... ]" << endl;
   	InputPtr input = Input::open_file(filename);
    if (input != nullptr) {
        ScannerPtr scanner = ScannerPtr(new Scanner(input));
        SemanticAnalyzerPtr analyzer = SemanticAnalyzerPtr(new SemanticAnalyzer(filename));
        ParserPtr parser = ParserPtr(new Parser(scanner, analyzer));
        parser->parse();
        parser->get_analyzer()->generate_all();
        report_msg_type("Success", "Compilation terminated successfully");
    } else {
        return -1;
    }
    cout << "[ End ]" << endl;
    return 0;
}

int code_gen_test() {
    return 0;
}

#endif
