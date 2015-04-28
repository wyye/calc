CXXFLAGS = -g -Wall

objects = HelpTools.o Interpreter.o AbstractSyntaxTree.o ParserFunc.o HashTable.o ParserDriver.o SSA.o CalcParser.o CalcScanner.o

.PHONY: all 
all: calc

CalcParser.o: CalcParser.yy
	bison CalcParser.yy
	$(CXX) $(CXXFLAGS) CalcParser.tab.cc -c -o CalcParser.o
	
CalcScanner.o: CalcScanner.l CalcParser.o
	flex CalcScanner.l
	$(CXX) $(CXXFLAGS) lex.yy.c -c -o CalcScanner.o

Interpreter.o: Interpreter.h Interpreter.cpp

AbstractSyntaxTree.o: AbstractSyntaxTree.h AbstractSyntaxTree.cpp

ParserFunc.o: ParserFunc.h ParserFunc.cpp

HashTable.o: HashTable.h HashTable.cpp

ParserDriver.o: ParserDriver.h ParserDriver.cpp CalcParser.o

SSA.o: SSA.h SSA.cpp

HelpTools.o: HelpTools.h HelpTools.cpp

calc: $(objects) main.cpp
	$(CXX) $(CXXFLAGS) $(objects) main.cpp -o calc
	
.PHONY: clean
clean: 
	rm -f $(objects) calc position.hh stack.hh location.hh CalcParser.tab.cc CalcParser.tab.hh lex.yy.c
