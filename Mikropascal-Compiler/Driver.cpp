/*
 * Driver.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: Jon
 */

#include "Standard.hpp"
#include "Tests.hpp"

string get_console_input() {
	string input, first;
	cout << ": ";
	cin >> input;
	istringstream stream_s(input);
	stream_s >> first;
	cin.clear(); cin.ignore(INT_MAX,'\n');
	return first;
}

int main(int argc, char* argv[]) {
	report_msg("Mikropascal Compiler");
	if (argc == 3) {

		// try open file
		FILE* fp = NULL;
		#ifdef __APPLE__
		fp = fopen(argv[2], "r");
		#elif _WIN32
		fopen_s(&fp, argv[2], "r");
		#endif

		// if the file pointer is null
		if (fp == NULL)
			report_error("General Error", "Could not open specified file");
		else {
			fclose(fp);
			if (strcmp(argv[1], "-s") == 0) {
				scanner_test(string(argv[2]));
			} else if (strcmp(argv[1], "-p") == 0) {
				parser_test(string(argv[2]));
			} else if (strcmp(argv[1], "-y") == 0) {
				symbol_test(string(argv[2]));
			} else if (strcmp(argv[1], "-c") == 0) {
				compile_chain(string(argv[2]));
			} else if (strcmp(argv[1], "-d")) {
				code_gen_test();
			} else {
				report_error("General Error", "Command line args specified are invalid");
			}
		}
	} else if (argc == 2 && (strcmp(argv[1], "-i") == 0)) {
		report_msg("Entering interactive mode...");
		report_msg("Press a key then 'Enter' to do something");
		char control_char = 'M';
		do {
			switch(control_char) {
				case 'C':
					report_msg("Goodbye!");
					return 0;
				case 'S':
					report_msg("Please enter a valid filename to test");
					scanner_test(get_console_input());
					break;
				case 'P':
					report_msg("Please enter a valid filename to test");
					parser_test(get_console_input());
					break;
				case 'Y':
					report_msg("Please enter a valid filename to test");
					symbol_test(get_console_input());
					break;
				case 'G':
					report_msg("Please enter a valid filename to compile");
					compile_chain(get_console_input());
					break;
				case 'M':
					report_msg_type("1", "Press G to compile");
					report_msg_type("2", "Press S to run a scan test");
					report_msg_type("3", "Press P to run a parse test");
					report_msg_type("4", "Press Y to run a symbol test");
					report_msg_type("5", "Press M to list menu options");
					report_msg_type("6", "Press C to quit");
					break;
				default:
					report_msg_type("Invalid Entry", "Please try again");
					break;
			}
			// get input and control
			cout << "> ";
			cin >> control_char;
			cin.clear(); cin.ignore(INT_MAX,'\n');
			control_char = toupper(control_char);
		} while (control_char != 'C');
	} else {
		report_msg_type("Warning", "No cmd line args, specified... going to default folder");
		FILE* fp = NULL;
		#ifdef __APPLE__
				fp = fopen("/Users/jonfast/Desktop/program.pas", "r");
		#elif _WIN32
				fopen_s(&fp, "C:\\Users\Jon\\Desktop\\program.pas", "r");
		#endif
		if (fp == NULL) {
			report_error("General Error", "Could not open the default hardcoded file");
			return EXIT_FAILURE;
		} else {
			fclose(fp);
			#ifdef __APPLE__
						compile_chain("/Users/jonfast/Desktop/program.pas");
			#elif _WIN32
						compile_chain("C:\\Users\\Jon\\Desktop\\program.pas");
			#endif
		}
		return EXIT_SUCCESS;
	}
}

