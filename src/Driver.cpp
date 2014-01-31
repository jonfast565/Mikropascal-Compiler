/*
 * Driver.cpp
 *
 *  Created on: Jan 19, 2014
 *      Author: Jon
 */

#include "Standard.hpp"
#include "Tests.hpp"

// prototypes
int main(int, char*[]);

int main(int argc, char* argv[]) {
	cout << "--- Mikropascal Compiler ---" << endl;
	if (argc >= 2) {
		// try open file
		FILE* fp = fopen(argv[1], "r");
		if (fp == NULL)
			cout << "Failure: Could not open specified file." << endl;
		else {
			fclose(fp);
			scanner_test(string(argv[1]));
		}
	} else {
		FILE* fp = fopen("/Users/jonfast/Desktop/program.pas", "r");
		if (fp == NULL)
			cout << "Failure: Could not open the default hardcoded file."
					<< endl;
		else {
			fclose(fp);
			scanner_test(string("/Users/jonfast/Desktop/program.pas"));
		}
		return EXIT_SUCCESS;
	}
}

