#include <iostream>

#include "ParserDriver.h"
#include "Interpreter.h"

int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cout << "Usage: ./calc file.txt mode\n";
		std::cout << "modes:\n\t-c\tcompiler\n\t-i\tinterpreter\n";
		exit(-1);
	}

	std::string file_name(argv[1]);
	std::string mode(argv[2]);


	ParserDriver driver;
	if (driver.parse(argv[1])) {
		std::cout << "Parser error\n";
		exit(-1);
	}

	if (strcmp(argv[2], "-i") == 0) {
		Interpreter interpreter(&driver.functable);
		interpreter.run();
	} else if (strcmp(argv[2], "-c") == 0) {
		SSAList ssa;
		driver.functable.get("main")->body->make_ssa(ssa);
		std::map<std::string, int> in;
		std::map<std::string, int> out;
		ssa.make_ssa(in, out);
		ssa.print();
	} else {
		std::cout << "Unknown mode\n";
		exit(-1);
	}
		
	return 0;
}
