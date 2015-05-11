%skeleton "lalr1.cc"
%require "3.0.4"
%defines
%define parser_class_name {CalcParser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <string>
#include <list>
#include "AbstractSyntaxTree.h"
#include "ParserFunc.h"
#include "HashTable.h"
#include "HelpTools.h"
class ParserDriver;
}

%param { ParserDriver& driver }

%locations
%initial-action
{
	@$.begin.filename = @$.end.filename = &driver.file;
};

%define parse.trace
%define parse.error verbose

%code
{
#include "ParserDriver.h"
}

%define api.token.prefix {TOK_}
%token
	END 0
	ADD "+"
	SUB "-"
	MUL "*"
	DIV "/"
	LPAREN "("
	RPAREN ")"
	LCURVEPAREN "{"
	RCURVEPAREN "}"
	LSQUAREPAREN "["
	RSQUAREPAREN "]"
	NOT "!"
	COLON ":"
	SEMICOLON ";"
	COMMA ","
	ASSIGN "="
	QUESTION "?"
	GREATER ">"
	LESS "<"
	EQUAL "=="
	GREATEREQUAL ">="
	LESSEQUAL "<="
	NOTEQUAL "!="
	INC "++"
	DEC "--"
	FUNC "function"
	WHILE "while"
	IF "if"
	ELSE "else"
;

%token <std::string> NAME "name"
%token <double> NUMBER "number"

%type <IASTNode*> prim term expr comparison equality ternary assign statement statements 
	block modifiable def_modifiable inc_dec initialization any_expr
%type <std::list<IASTNode*>*> func_call_args func_def_args
%type <std::list<double>*> init_list

%printer { yyoutput << $$; } <*>;

%%
functions:
	function
	| functions function
	;
function:
	FUNC NAME { 
		std::map<std::string, unsigned int> temp;
		driver.sym_table_stack.push_back(temp); 
	} LPAREN func_def_args RPAREN LCURVEPAREN statements RCURVEPAREN {
		ParserFunc* pf = new ParserFunc;
		pf->name = $2;
		while (!$5->empty()) {
			pf->arg.push_back($5->front());
			$5->pop_front();
		}
		delete $5;
		pf->body = $8;
		if (0 == driver.functable.put(pf)) {
			driver.error("Function appears second time");
		}
		
		// check is result exist
		std::map<std::string, unsigned int> last = driver.sym_table_stack.back();
		if (!last.count("result")) driver.error("Function '" + $2 + "' is not returning result");
		
		driver.sym_table_stack.pop_back();
	}
	;
statements:
	statement { $$ = $1; }
	| statements statement { 
		ASTNoRetBinaryOpNode* parent = new ASTNoRetBinaryOpNode(STATEMENTS);
		parent->set($1, $2);
		$$ = parent;
	}
	;
block:
	LCURVEPAREN RCURVEPAREN {
		$$ = new ASTEmptyNode();
	}
	| LCURVEPAREN {
		std::map<std::string, unsigned int> temp;
		driver.sym_table_stack.push_back(temp); 
	}
	statements RCURVEPAREN {
		driver.sym_table_stack.pop_back();
		$$ = $3;
	}
	;
statement:
	block { $$ = $1; }
	| WHILE LPAREN any_expr RPAREN statement { 
		ASTNoRetBinaryOpNode* parent = new ASTNoRetBinaryOpNode(WHILE_CYCLE);
		parent->set($3, $5);
		$$ = parent;
	}
	| IF LPAREN any_expr RPAREN block ELSE block { 
		ASTNoRetTernaryOpNode* parent = new ASTNoRetTernaryOpNode(IF); 
		parent->set($3, $5, $7);
		$$ = parent;
	}
	| IF LPAREN any_expr RPAREN block {
		ASTNoRetTernaryOpNode* parent = new ASTNoRetTernaryOpNode(IF);
		parent->set($3, $5, new ASTEmptyNode);
		$$ = parent;
	}
	| initialization SEMICOLON { $$ = $1; }
	| inc_dec SEMICOLON { $$ = $1; }
	;
initialization:
	NAME {
		int create_new = 1;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second != 0) 
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new) {
			driver.sym_table_stack.back()[$1] = driver.last_index;
			driver.sym_table[driver.last_index] = make_pair($1, 0);
			ASTAssignNode* parent = new ASTAssignNode();
			parent->set(new ASTLeafVar(driver.last_index++), new ASTLeafNum(0.0));
			$$ = parent;
		} else {
			$$ = new ASTLeafVar(it_in_stack->second);
		}
	}
	| NAME ASSIGN any_expr {
		int create_new = 1;
		ASTLeafVar* left;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second != 0) 
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new) {
			driver.sym_table_stack.back()[$1] = driver.last_index;
			driver.sym_table[driver.last_index] = make_pair($1, 0);
			left = new ASTLeafVar(driver.last_index++);
		} else {
			left = new ASTLeafVar(it_in_stack->second);
		}
		ASTAssignNode* parent = new ASTAssignNode();
		parent->set(left, $3);
		$$ = parent;
	}
	| NAME LSQUAREPAREN any_expr RSQUAREPAREN ASSIGN LSQUAREPAREN init_list RSQUAREPAREN {
		int create_new = 1;
		IASTNode* left;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second == 0) 
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new == 0 && it == driver.sym_table_stack.rbegin()) {
			driver.error("Array double initialization");
		}
		if ($3->get_op() != NUMBER) driver.error("Array initialization with unknown size");
		double number = dynamic_cast<ASTLeafNum*>($3)->get();
		delete $3;
		if (number < 1.0) driver.error("Array size less than 1");
		if (number > 1e9) driver.error("Array size too big");
		unsigned int array_size = static_cast<unsigned int>(number);
		driver.sym_table_stack.back()[$1] = driver.last_index;
		driver.sym_table[driver.last_index] = make_pair($1, array_size);
		
		// make initialization
		
		left = new ASTEmptyNode();
		
		unsigned int position = 0;
		while (!$7->empty()) {
			ASTIndexNode* index = new ASTIndexNode();
			if (position >= array_size) driver.error("Init list too long");
			index->set(new ASTLeafVar(driver.last_index), new ASTLeafNum(position++));
			ASTAssignNode* parent = new ASTAssignNode();
			parent->set(index, new ASTLeafNum($7->front()));
			$7->pop_front();		
			ASTNoRetBinaryOpNode* statement = new ASTNoRetBinaryOpNode(STATEMENTS);
			statement->set(left, parent);
			left = statement;
		}
		delete $7;
		driver.last_index++;
		$$ = left;
	}
	| NAME LSQUAREPAREN any_expr RSQUAREPAREN ASSIGN any_expr {
		int create_new = 1;
		IASTNode* left;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second == 0) 
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new) {
			driver.error("Array '" + $1 + "' not initialized");
		} else {
			ASTIndexNode* index = new ASTIndexNode();
			index->set(new ASTLeafVar(it_in_stack->second), $3);
			left = index;
		}
		ASTAssignNode* parent = new ASTAssignNode();
		parent->set(left, $6);
		$$ = parent;
	}
	| NAME LSQUAREPAREN any_expr RSQUAREPAREN {
		int create_new = 1;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second == 0) 
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new == 0 && it == driver.sym_table_stack.rbegin()) {
			driver.error("Array double initialization");
		}
		if ($3->get_op() != NUMBER) driver.error("Array initialization with unknown size");
		double number = dynamic_cast<ASTLeafNum*>($3)->get();
		delete $3;
		if (number < 1.0) driver.error("Array size less than 1");
		if (number > 1e9) driver.error("Array size too big");
		unsigned int array_size = static_cast<unsigned int>(number);
		driver.sym_table_stack.back()[$1] = driver.last_index;
		driver.sym_table[driver.last_index++] = make_pair($1, array_size);
		
		$$ = new ASTEmptyNode();
	}
	;
inc_dec:
	modifiable INC { 
		ASTIncrOpNode* parent = new ASTIncrOpNode(POST_INC);
		parent->set($1);
		$$ = parent;
	}
	| modifiable DEC {
		ASTIncrOpNode* parent = new ASTIncrOpNode(POST_DEC);
		parent->set($1);
		$$ = parent;
	}
	| INC modifiable {
		ASTIncrOpNode* parent = new ASTIncrOpNode(PRE_INC);
		parent->set($2);
		$$ = parent;
	}
	| DEC modifiable {
		ASTIncrOpNode* parent = new ASTIncrOpNode(PRE_DEC);
		parent->set($2);
		$$ = parent;
	}
	;
any_expr:
	assign { $$ = $1; }
	| ternary { $$ = $1; }
	;
assign:
	modifiable ASSIGN any_expr {
		ASTAssignNode* parent = new ASTAssignNode();
		parent->set($1, $3);
		$$ = parent;
	}
	;
ternary:
	equality { $$ = $1; }
	| equality QUESTION any_expr COLON ternary { 
		ASTTernaryOpNode* parent = new ASTTernaryOpNode(TERNARY);
		parent->set($1, $3, $5);
		$$ = parent;
	}
	;
equality:
	comparison { $$ = $1; }
	| equality EQUAL comparison { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(EQUALITY);
		parent->set($1, $3);
		$$ = parent;
	}
	| equality NOTEQUAL comparison { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(NEQUALITY);
		parent->set($1, $3);
		$$ = parent;
	}
	;
comparison:
	expr { $$ = $1; }
	| comparison LESS expr { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(LESS);
		parent->set($1, $3);
		$$ = parent;
	}
	| comparison LESSEQUAL expr { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(LESS_EQUAL);
		parent->set($1, $3);
		$$ = parent;
	}
	| comparison GREATER expr { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(GREATER);
		parent->set($1, $3);
		$$ = parent;
	}
	| comparison GREATEREQUAL expr { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(GREATER_EQUAL);
		parent->set($1, $3);
		$$ = parent;
	}
	;
expr:
	term { $$ = $1; }
	| expr ADD term { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(ADD);
		parent->set($1, $3);
		$$ = parent;
	}
	| expr SUB term { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(SUB);
		parent->set($1, $3);
		$$ = parent;
	}
	;
term:
	prim { $$ = $1; }
	| term MUL prim { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(MUL);
		parent->set($1, $3);
		$$ = parent;
	}
	| term DIV prim { 
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(DIV);
		parent->set($1, $3);
		$$ = parent;
	}
	;
prim:
	LPAREN any_expr RPAREN { $$ = $2; }
	| SUB prim { 
		ASTUnaryOpNode* parent = new ASTUnaryOpNode(UNARY_MINUS);
		parent->set($2);
		$$ = parent;
	}
	| NOT prim { 
		ASTUnaryOpNode* parent = new ASTUnaryOpNode(NOT);
		parent->set($2);
		$$ = parent;
	}
	| NUMBER { $$ = new ASTLeafNum($1); }
	| NAME LPAREN func_call_args RPAREN {
		ASTFuncCallNode* func = new ASTFuncCallNode($1);
		while (!$3->empty()) {
			func->set_args($3->front());
			$3->pop_front();
		}
		delete $3;
		$$ = func;
	}
	| modifiable { $$ = $1; }
	| inc_dec { $$ = $1; }
	;
modifiable:
	NAME {
		int create_new = 1;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				create_new = 0;
				break;
			}
		}
		if (create_new) {
			driver.error("Variable '" + $1 + "' not found");
		} else {
			$$ = new ASTLeafVar(it_in_stack->second);
		}
	}
	| NAME LSQUAREPAREN any_expr RSQUAREPAREN {
		int create_new = 1;
		std::list<std::map<std::string, unsigned int> >::reverse_iterator it;
		std::map<std::string, unsigned int>::iterator it_in_stack;
		for (it = driver.sym_table_stack.rbegin(); it != driver.sym_table_stack.rend(); ++it) {
			it_in_stack = it->find($1);
			if (it_in_stack != it->end()) {
				std::pair<std::string, unsigned int> sym_table_record = driver.sym_table[it_in_stack->second];
				if (sym_table_record.second == 0)
					driver.error("Variable name '" + sym_table_record.first + "' the same as array name");
				create_new = 0;
				break;
			}
		}
		if (create_new) {
			driver.error("Array '" + $1 + "' not found");
		} else {
			ASTIndexNode* index = new ASTIndexNode();
			index->set(new ASTLeafVar(it_in_stack->second), $3);
			$$ = index;
		}
	}
	;
init_list:
	/* empty */ { $$ = new std::list<double>; }
	| NUMBER { 
		$$ = new std::list<double>; 
		$$->push_back($1);
	}
	| init_list COMMA NUMBER { 
		$1->push_back($3);
		$$ = $1;
	}
	;
func_def_args:
	/* empty */ { $$ = new std::list<IASTNode*>; }
	| def_modifiable {
		$$ = new std::list<IASTNode*>; 
		$$->push_back($1);
	}
	| func_def_args COMMA def_modifiable { 
		$1->push_back($3);
		$$ = $1;
	}
	;
def_modifiable:
	NAME {
		driver.sym_table_stack.back()[$1] = driver.last_index;
		driver.sym_table[driver.last_index] = make_pair($1, 0);
		$$ = new ASTLeafVar(driver.last_index++);
	}
	| NAME LSQUAREPAREN NUMBER RSQUAREPAREN {
		if ($3 < 0.0) driver.error("Array size less than zero");
		if ($3 > 1e9) driver.error("Array size too big");
		unsigned int array_size = static_cast<unsigned int>($3);
		driver.sym_table_stack.back()[$1] = driver.last_index;
		driver.sym_table[driver.last_index] = make_pair($1, array_size);
		ASTIndexNode* index = new ASTIndexNode();
		index->set(new ASTLeafVar(driver.last_index++), new ASTLeafNum($3));
		$$ = index;
	}
	;
func_call_args:
	/* empty */ { $$ = new std::list<IASTNode*>; }
	| any_expr { 
		$$ = new std::list<IASTNode*>;
		$$->push_back($1);
	}
	| func_call_args COMMA any_expr { 
		$1->push_back($3);
		$$ = $1;
	}
	;
%%

void yy::CalcParser::error (const location_type& l,  const std::string& m)
{
	driver.error(l, m);
}