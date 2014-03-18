/*
 * Driver.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: Jon
 */

#include "Standard.hpp"
#include "Tests.hpp"

int main(int argc, char* argv[]) {
	cout << "[ Mikropascal Compiler ]" << endl;
	if (argc >= 3) {
		// try open file
		FILE* fp = fopen(argv[2], "r");
		if (fp == NULL)
			report_error("General Error", "Could not open specified file.");
		else {
			fclose(fp);
			if (strcmp(argv[1], "--scantest") == 0)
				scanner_test(string(argv[2]));
			else if (strcmp(argv[1], "--parsetest") == 0)
				parser_test(string(argv[2]));
			else
				report_error("General Error", "Command line args specified are invalid...");
		}
	} else {
		report_msg_type("Warning", "No cmd line args, specified... going to default folder.");
		FILE* fp = fopen("/Users/jonfast/Desktop/program.pas", "r");
		if (fp == NULL)
			report_error("General Error", "Could not open the default hardcoded file.");
		else {
			fclose(fp);
			scanner_test("/Users/jonfast/Desktop/program.pas");
		}
		return EXIT_SUCCESS;
	}
}

