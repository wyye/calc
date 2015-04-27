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
;

%token <std::string> NAME "name"
%token <double> NUMBER "number"

%type <IASTNode*> prim term expr comparison equality ternary assign statement statements
%type <std::list<IASTNode*>*> func_call_args
%type <std::list<std::string>*> func_def_args

%printer { yyoutput << $$; } <*>;

%%
functions:
	function
	| functions function
	;
function:
	FUNC NAME LPAREN func_def_args RPAREN LCURVEPAREN statements RCURVEPAREN {
		ParserFunc* pf = new ParserFunc;
		pf->name = $2;
		pf->args_num = 0;
		while (!$4->empty()) {
			pf->arg.push_back($4->front());
			++pf->args_num;
			$4->pop_front();
		}
		delete $4;
		pf->body = $7;
		if (0 == driver.functable.put(pf)) {
			driver.error("Function appears second time");
		}
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
statement:
	WHILE LPAREN assign RPAREN statement { 
		ASTNoRetBinaryOpNode* parent = new ASTNoRetBinaryOpNode(WHILE_CYCLE);
		parent->set($3, $5);
		$$ = parent;
	}
	| LCURVEPAREN statements RCURVEPAREN { $$ = $2; }
	| assign SEMICOLON { $$ = $1; }
	;
assign:
	NAME ASSIGN assign {
		ASTBinaryOpNode* parent = new ASTBinaryOpNode(ASSIGN);
		parent->set(new ASTLeafVar($1.c_str()), $3);
		$$ = parent;
	}
	| ternary { $$ = $1; }
	;
ternary:
	equality { $$ = $1; }
	| equality QUESTION assign COLON ternary { 
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
	LPAREN assign RPAREN { $$ = $2; }
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
		ASTFuncCallNode* func = new ASTFuncCallNode($1.c_str());
		while (!$3->empty()) {
			func->set_args($3->front());
			$3->pop_front();
		}
		delete $3;
		$$ = func;
	}
	| NAME INC { 
		ASTLeafVar* leafvar = new ASTLeafVar($1.c_str());
		ASTIncrOpNode* parent = new ASTIncrOpNode(POST_INC);
		parent->set(leafvar);
		$$ = parent;
	}
	| NAME DEC {
		ASTLeafVar* leafvar = new ASTLeafVar($1.c_str());
		ASTIncrOpNode* parent = new ASTIncrOpNode(POST_DEC);
		parent->set(leafvar);
		$$ = parent;
	}
	| NAME { $$ = new ASTLeafVar($1.c_str()); }
	| INC NAME {
		ASTIncrOpNode* parent = new ASTIncrOpNode(PRE_INC);
		parent->set(new ASTLeafVar($2.c_str()));
		$$ = parent;
	}
	| DEC NAME {
		ASTIncrOpNode* parent = new ASTIncrOpNode(PRE_DEC);
		parent->set(new ASTLeafVar($2.c_str()));
		$$ = parent;
	}
	;
func_def_args:
	/* empty */ { $$ = new std::list<std::string>; }
	| NAME { 
		$$ = new std::list<std::string>; 
		$$->push_back($1);
	}
	| func_def_args COMMA NAME { 
		$1->push_back($3);
		$$ = $1;
	}
	;
func_call_args:
	/* empty */ { $$ = new std::list<IASTNode*>; }
	| assign { 
		$$ = new std::list<IASTNode*>;
		$$->push_back($1);
	}
	| func_call_args COMMA assign { 
		$1->push_back($3);
		$$ = $1;
	}
	;
%%

void yy::CalcParser::error (const location_type& l,  const std::string& m)
{
	driver.error(l, m);
}