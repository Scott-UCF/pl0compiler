/*
By Scott Kramarz
Instructions: Run at command line with single argument: <filename> 
The <filename>(.txt optional) should be a file containing pl0 code
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef enum {
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym, slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym,
whilesym, dosym, constsym, varsym, writesym,
readsym, procsym, callsym, elsesym} token_type;

typedef struct Lex {
	int token;
	char name[11];
	int val;
} Lex;

typedef struct symbol{
	int kind;
	char name[11];
	int val;
	int level;
	int address;
	int mark;
} symbol;

symbol symbolTable[500];

int gSymIndex = 0;
int gLexIndex = 0;
int addressIndex = 4;
int lexLevel = 0;

int getToken();
int parse(Lex lexTable[]);
int program(Lex lexTable[]);
int block(Lex lexTable[]);
int statement(Lex lexTable[]);
int expression(Lex lexTable[]);
int term(Lex lexTable[]);
int factor(Lex lexTable[], int rg);
void error(int errNum);
int vm(int compilerDirectiveVm);

int instructions[500][4] = {0};
int instIndex = 0;
int usedRegs[16] = {0};
int inSubStatement = 0;
int findSym(char name[]){
	int i;
	for(i = 0; i < 500; i++){
		if(strcmp(name, symbolTable[i].name) == 0){
			if(lexLevel >= symbolTable[1].level){
				return i;
			}
		}
	}
	error(21);
}

int unusedReg(){
	int i;
	for(i = 0; i < 16; i++){
		if(usedRegs[i] == 0){
			usedRegs[i] = 1;
			return i;
		}
	}
	error(20);
}
void freeRegs(){
	int i;
	for(i = 0; i < 16; i++){
		usedRegs[i] = 0;
	}
}

void generate(int op, int rg, int lx, int v1){
	switch(op){
		//Load Literal into register
		case 1:
			instructions[instIndex][0] = 1;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Return - Not Yet Implemented
		case 2:
			instructions[instIndex][0] = 2;
			instructions[instIndex][1] = 0;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = 0;
			instIndex++;
			break;
		//Load
		case 3:
			instructions[instIndex][0] = 3;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Store
		case 4:
			instructions[instIndex][0] = 4;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Call
		case 5:
			instructions[instIndex][0] = 5;
			instructions[instIndex][1] = 0;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Increment
		case 6:
			instructions[instIndex][0] = 6;
			instructions[instIndex][1] = 0;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Jump
		case 7:
			instructions[instIndex][0] = 7;
			instructions[instIndex][1] = 0;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Conditional Jump
		case 8:
			instructions[instIndex][0] = 8;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//SIO
		case 9:
			instructions[instIndex][0] = 9;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Negation
		case 10:
			instructions[instIndex][0] = 10;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;	
		//Addition
		case 11:
			instructions[instIndex][0] = 11;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Subtraction
		case 12:
			instructions[instIndex][0] = 12;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;		
		//Multiplication
		case 13:
			instructions[instIndex][0] = 13;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Division
		case 14:
			instructions[instIndex][0] = 14;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//isOdd?
		case 15:
			instructions[instIndex][0] = 15;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = 0;
			instructions[instIndex][3] = 0;
			instIndex++;
			break;
		//Modulo
		case 16:
			instructions[instIndex][0] = 16;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Equal To
		case 17:
			instructions[instIndex][0] = 17;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Not Equal To
		case 18:
			instructions[instIndex][0] = 18;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Less Than
		case 19:
			instructions[instIndex][0] = 19;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Less Than or Equal To
		case 20:
			instructions[instIndex][0] = 20;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Greater Than
		case 21:
			instructions[instIndex][0] = 21;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;
		//Greater Than or Equal To
		case 22:
			instructions[instIndex][0] = 22;
			instructions[instIndex][1] = rg;
			instructions[instIndex][2] = lx;
			instructions[instIndex][3] = v1;
			instIndex++;
			break;		
	}
}

void error(int errNum){
	switch(errNum){
		case 1:
			printf("Error ':' expected before =");
			break;
		case 2:
			printf("Error ':=' needs to be followed by a number or appropriate variable");
			break;
		case 3:
			printf("Error Identifier in statement must be followed by ':='");
			break;
		case 4:
			printf("Error Constant declaration must start with an Identifier");
			break;
		case 5:
			printf("Error Constant must be assigned a number");
			break;
		case 6:
			printf("Error Semicolon expected");
			break;
		case 7:
			printf("Error Variable declaration must start with an Identifier");
			break;
		case 8:
			printf("Error Identifiers in statement must be followed by ':='");
			break;
		case 9:
			printf("Error Right Parenthesis expected");
			break;
		case 10:
			printf("Error Identifier or Number expected");
			break;
		case 11:
			printf("Error invalid Identifier - Procedures are not part of valid expressions");
			break;
		case 12:
			printf("Error End expected after statement block");
			break;
		case 13:
			printf("Error Then expected after If");
			break;
		case 14:
			printf("Error Do expected after While");
			break;
		case 15:
			printf("Error '=' expected after Identifier in constant declaration");
			break;
		case 16:
			printf("Error '.' expected at end of program");
			break;
		case 17:
			printf("Error symbol found after end of program");
			break;
		case 18:
			printf("\nError identifier too long");
			break;
		case 19:
			printf("Error assignment statement using constant or procedures are invalid");
			break;
		case 20:
			printf("Error Register file limit reached");
			break;
		case 21:
			printf("Error Identifier not declared");
			break;
		case 22:
			printf("Error Invalid Expression");
			break;
		case 23:
			printf("Error Identifier expected");
			break;
		case 25:
			printf("\nInvalid Symbol");
			break;
		case 26:
			printf("Error const, var, procedure must be followed by identifier.");
		case 50:
			break;
		default:
			printf("Error - Unspecified Error");
			break;
	}
	printf("\nCompiler has stopped due to syntax error\n");
	exit(0);
}

int main(int argc, char* argv[]){
	char *fName = malloc(sizeof(char)*255);
	int compilerDirectiveLex = 0;
	int compilerDirectiveAss = 0;
	int compilerDirectiveVm = 0;
	char *code;
	char resWords[13][10] = {"begin", "end", "if", "then", "while", "do", "const", "var", "write", "read", "procedure", "call", "else"};
	int i = 0, j = 0, k = 0, temp = 0, lexIndex = 0, isResWord = 0, isComment = 0, lexEndIndex = 0;
	char buffer[12];
	Lex lexTable[500];
	if(argc < 2){
		printf("\nRuntime Error: File name parameter required\n");
		return 0;
	}
	if(argc >= 2){
		for(i = 2; i < argc; i++){
			if(strcmp(argv[i], "-l") == 0)
				compilerDirectiveLex = 1;
			if(strcmp(argv[i], "-v") == 0)
				compilerDirectiveVm = 1;
			if(strcmp(argv[i], "-a") == 0)
				compilerDirectiveAss = 1;
		}
	}
	strcpy(fName, argv[1]);
	
	FILE *file = fopen(fName, "r");
	
	if(file == NULL)  
	{
		//Try again with .txt appended just incase the user forgot or the OS needs it.
		fName = strcat(fName, ".txt");
		file = fopen(fName, "r");
		if (file == NULL)
		{
			printf("\nRuntime Error: Instruction file not found\n" );
			return 0;
		}
	} 
	
	fseek(file, 0, SEEK_END); 
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	//Allocate memory for  the source code
	code = malloc(sizeof(char)*(length+1));
	//Get the code from the file
	for(i = 0;((code[i] = fgetc(file)) != EOF); i++)
		;
	/*
	if(compilerDirectiveLex){
		printf("Source Program: %s\n", argv[1]);
		for(i = 0; i < length; i++)
			printf("%c", code[i]);
	}
	*/
	for(i = 0; i < length+1;){
		//Code begins with a letter
		if((code[i] >= 'A' && code[i] <= 'Z') || (code[i] >= 'a' && code[i] <= 'z')){
			for(k = 0; k < 12; k++)
				buffer[k] = '\0';
			for(j = 0;(code[i] >= 'A' && code[i] <= 'Z') || (code[i] >= 'a' && code[i] <= 'z') || (code[i] >= '0' && code[i] <= '9');i++, j++){
				if(j > 11){
					error(18);
					j = 11;
					continue;
				}
				buffer[j] = code[i];
			}
			if(!((code[i] >= 'A' && code[i] <= 'Z') || (code[i] >= 'a' && code[i] <= 'z') || (code[i] >= '0' && code[i] <= '9') || (code[i] == ' ' || code[i] == ',' || code[i] == ';' || code[i] == '\n' || code[i] == '.' || code[i] == 13 || code[i] == '	'))){
				printf("\nError: Invalid Identifier");
				error(50);
				continue;
			}
			buffer[j] = '\0';

			if(strlen(buffer) == 0)
				continue;
			//If it's a reserved word
			isResWord = 0;
			for(k = 0; k < 13; k++){
				if(strcmp(buffer, resWords[k]) == 0){
					lexTable[lexIndex].token = 21 + k;
					strcpy(lexTable[lexIndex].name, buffer);
					lexIndex++;
					isResWord = 1;
					break;
				}
			}
			if(isResWord)
				continue;
			//Otherwise it's an identifier
			lexTable[lexIndex].token = 2;
			strcpy(lexTable[lexIndex].name, buffer);
			lexIndex++;
			continue;
			
		}
		//Starts with a number
		else if(code[i] >= '0' && code[i] <= '9'){
			for(j = 0; (code[i] >= '0' && code[i] <= '9'); i++, j++){
				if(j >= 5){
					printf("\nError: Number too large");
					error(50);
					j = 4;
					continue;
				}
				buffer[j] = code[i];
			}
			if((code[i] > '9' || code [i] < '0') && (!(code[i] == ' ' || code[i] == ';' || code[i] == '\n'))){
				printf("\nError: Invalid Identifier.");
				for(;(code[i] >= 'A' && code[i] <= 'Z') || (code[i] >= 'a' && code[i] <= 'z') || (code[i] >= '0' && code[i] <= '9');i++)
					;
				continue;
			}
			lexTable[lexIndex].token = 3;
			temp = 0;
			for(k = 0; k < j; k++){
				temp *= 10;
				temp += buffer[k] - '0';
			}
			lexTable[lexIndex].val = temp;
			lexIndex++;
		}
		//Special Symbol
		else if(code[i] == '+' || code[i] == '-' || code[i] == '*' || code[i] == '/' || code[i] == '(' || code[i] == ')' || code[i] == '=' || code[i] == ',' || code[i] == '.' || code[i] == '<' || code[i] == '>' || code[i] == ';' || code[i] == ':'){
			switch(code[i]){
				case '+' :
					lexTable[lexIndex].token = 4;
					strcpy(lexTable[lexIndex].name, "+");
				break;
				case '-' :
					lexTable[lexIndex].token = 5;
					strcpy(lexTable[lexIndex].name, "-");
				break;
				case '*' :
					lexTable[lexIndex].token = 6;
					strcpy(lexTable[lexIndex].name, "*");
				break;
				case '/' :
					if(code[i+1] == '*'){
						i++;
						isComment = 1;
						for(isComment = 1;isComment; i++){
							if(code[i] == '*'){
								if(code[i+1] == '/'){
									i++;
									isComment = 0;
									break;
								}
							}
						}
						break;
					}
					lexTable[lexIndex].token = 7;
					strcpy(lexTable[lexIndex].name, "/");
				break;
				case '(' :
					lexTable[lexIndex].token = 15;
					strcpy(lexTable[lexIndex].name, "(");
				break;
				case ')' :
					lexTable[lexIndex].token = 16;
					strcpy(lexTable[lexIndex].name, ")");
				break;
				case '=' :
					lexTable[lexIndex].token = 9;
					strcpy(lexTable[lexIndex].name, "=");
				break;
				case ',' :
					lexTable[lexIndex].token = 17;
					strcpy(lexTable[lexIndex].name, ",");
				break;
				case '.' :
					lexTable[lexIndex].token = 19;
					strcpy(lexTable[lexIndex].name, ".");
				break;
				case '<' :
					if(code[i+1] == '='){
						lexTable[lexIndex].token = leqsym;
						strcpy(lexTable[lexIndex].name, "<=");
						i++;
						break;
					}
					if(code[i+1] == '>'){
						lexTable[lexIndex].token = 10;
						strcpy(lexTable[lexIndex].name, "<>");
						i++;
						break;
					}
					lexTable[lexIndex].token = lessym;
					strcpy(lexTable[lexIndex].name, "<");
				break;
				case '>' :
					if(code[i+1] == '='){
						lexTable[lexIndex].token = geqsym;
						strcpy(lexTable[lexIndex].name, ">=");
						i++;
						break;
					}
					lexTable[lexIndex].token = gtrsym;
					strcpy(lexTable[lexIndex].name, ">");
				break;
				case ':' :
					if(code[i+1] == '='){
						lexTable[lexIndex].token = 20;
						strcpy(lexTable[lexIndex].name, ":=");
						i++;
						break;
					}
					else{
						printf("\nError: Invalid Symbol");
						error(50);
					};
				break;
				case ';' :
					lexTable[lexIndex].token = 18;
					strcpy(lexTable[lexIndex].name, ";");
				break;			
			}
			lexIndex++;
			i++;
		}
		else if(code[i] == '&' || code[i] == '%' || code[i] == '^' || code[i] == '#' || code[i] == '@' || code[i] == '`' || code[i] == '~' || code[i] == '|' || code[i] == '{' || code[i] == '}' || code[i] == '[' || code[i] == ']' || code[i] == '?')
			error(25);
		else{
			i++;
		}
	}
	lexEndIndex = lexIndex;
	//Print the Lex Table
	
	if(compilerDirectiveLex){
		printf("\nLexeme Table\nlexeme\ttoken type\n");
		for(i = 0; i < lexEndIndex; i++){
			if(lexTable[i].token == 3)
				printf("%d\t%d\n", lexTable[i].token, lexTable[i].val);
			else
				printf("%s\t%d\n", lexTable[i].name, lexTable[i].token);
		}
	
		printf("\n\nLexeme List:\n");
		for(i = 0; i < lexEndIndex; i++){
			if(lexTable[i].token == 2)
				printf("%d %s ", lexTable[i].token, lexTable[i].name);
			else if(lexTable[i].token == 3)
				printf("%d %d ", lexTable[i].token, lexTable[i].val);
			else
				printf("%d ", lexTable[i].token);
		}
		printf("\n");
	}
	
	parse(lexTable);
	if((gLexIndex != lexIndex))
		error(17);

	
	if(compilerDirectiveAss){
		printf("\nParssed Successfully, Syntax is correct\n");
		printf("\nAssembled Code\n");
		for(i = 0; i < instIndex; i++){
			for(k = 0; k < 4; k++){
				printf("%02d ", instructions[i][k]);
			}
			printf("\n");
		}
	}
	vm(compilerDirectiveVm);
}


int parse(Lex lexTable[]){
	program(lexTable);
	return 0;
}

int program(Lex lexTable[]){
	generate(7, 0, 0, 1);
	block(lexTable);
	generate(9, 0, 0, 3);
	if(lexTable[gLexIndex].token != periodsym)
		error(16);
	gLexIndex++;
	return 0;
}
int block(Lex lexTable[]){
	char name[11];
	int val;
	int varCount = 0;
	if(lexTable[gLexIndex].token == constsym){
		gLexIndex++;
		if(lexTable[gLexIndex].token != identsym){
			error(4);
		}
		strcpy(name, lexTable[gLexIndex].name);
		gLexIndex++;
		if(lexTable[gLexIndex].token != eqsym)
			error(15);
		gLexIndex++;
		if(lexTable[gLexIndex].token != numbersym)
			error(5);
		val = lexTable[gLexIndex].val;
		gLexIndex++;
		symbolTable[gSymIndex].kind = 1;
		symbolTable[gSymIndex].val = val;
		strcpy(symbolTable[gSymIndex].name, name);
		gSymIndex++;
		while(lexTable[gLexIndex].token == commasym){
			gLexIndex++;
			if(lexTable[gLexIndex].token != identsym)
				error(4);
			strcpy(name, lexTable[gLexIndex].name);
			gLexIndex++;
			if(lexTable[gLexIndex].token != eqsym)
				error(3);
			gLexIndex++;
			if(lexTable[gLexIndex].token != numbersym)
				error(5);
			val = lexTable[gLexIndex].val;
			gLexIndex++;
		
			symbolTable[gSymIndex].kind = 1;
			symbolTable[gSymIndex].val = val;
			strcpy(symbolTable[gSymIndex].name, name);
			gSymIndex++;
		}
		
		if(lexTable[gLexIndex].token != semicolonsym)
			error(6);
		gLexIndex++;
	}
	if(lexTable[gLexIndex].token == varsym){
		gLexIndex++;
		varCount++;
		if(lexTable[gLexIndex].token != identsym)
			error(7);
		strcpy(name, lexTable[gLexIndex].name);
		gLexIndex++;
		
		symbolTable[gSymIndex].kind = 2;
		symbolTable[gSymIndex].level = lexLevel;
		symbolTable[gSymIndex].val = 0;
		symbolTable[gSymIndex].address = addressIndex++;
		strcpy(symbolTable[gSymIndex].name, name);
		gSymIndex++;

		while(lexTable[gLexIndex].token == commasym){
			gLexIndex++;
			varCount++;
			if(lexTable[gLexIndex].token != identsym)
				error(7);
			strcpy(name, lexTable[gLexIndex].name);
			gLexIndex++;
		
			symbolTable[gSymIndex].kind = 2;
			symbolTable[gSymIndex].level = 0;
			symbolTable[gSymIndex].val = 0;
			symbolTable[gSymIndex].address = addressIndex++;
			strcpy(symbolTable[gSymIndex].name, name);
			gSymIndex++;
		}
		
		if(lexTable[gLexIndex].token != semicolonsym)
			error(6);
		gLexIndex++;
	}

	
	while(lexTable[gLexIndex].token == procsym){
		gLexIndex++;
		if(lexTable[gLexIndex].token != identsym)
			error(26);
		symbolTable[gSymIndex].kind = 3;
		symbolTable[gSymIndex].level = lexLevel;
		symbolTable[gSymIndex].address = instIndex;
		strcpy(symbolTable[gSymIndex].name, lexTable[gLexIndex].name);
		gSymIndex++;
		gLexIndex++;
		if(lexTable[gLexIndex].token != semicolonsym)
			error(6);
		gLexIndex++;
		lexLevel++;
		block(lexTable);
		lexLevel--;
		generate(2, 0, 0, 0);
		if(lexTable[gLexIndex].token != semicolonsym)
			error(6);
		gLexIndex++;
	}
	generate(6, 0, 0, 4 + varCount);
	instructions[0][3] = instIndex;
	statement(lexTable);

}
int statement(Lex lexTable[]){
	int symIndex;
	int exprReg, exprReg2;
	int conditionType;
	int conditionReg;
	int jpcGen;
	int rwReg;
	//Identifier Starting a statement
	if(lexTable[gLexIndex].token == identsym){
		//Find the symbol table index of identifier
		symIndex = findSym(lexTable[gLexIndex].name);
		if(symbolTable[symIndex].kind != 2){
			error(19);
		}
		gLexIndex++;
		if(lexTable[gLexIndex].token != becomessym)
			error(8);
		gLexIndex++;
		//Find generate the expression assigned to the Idenfier
		exprReg = expression(lexTable);
		//Store it in the appropriate location in the stack
		generate(4, exprReg, 0, symbolTable[symIndex].address);
	}
	//Begin symbol - The block of repeated statements
	else if(lexTable[gLexIndex].token == callsym){
		gLexIndex++;
		if(lexTable[gLexIndex].token != identsym)
			error(26);
		symIndex = findSym(lexTable[gLexIndex].name);
		gLexIndex++;
		generate(5, 0, symbolTable[symIndex].level, symbolTable[symIndex].address);
		
	}
	else if(lexTable[gLexIndex].token == beginsym){
		gLexIndex++;
		statement(lexTable);
		while(lexTable[gLexIndex].token == semicolonsym){
			gLexIndex++;
			statement(lexTable);
		}
		if(lexTable[gLexIndex].token != endsym){
			if(lexTable[gLexIndex].token == identsym || lexTable[gLexIndex].token == whilesym || lexTable[gLexIndex].token == beginsym || lexTable[gLexIndex].token == ifsym || lexTable[gLexIndex].token == readsym || lexTable[gLexIndex].token == writesym){
				error(6);
			}
			else if(lexTable[gLexIndex].token == numbersym){
				error(22);
			}
			error(12);
		}
		gLexIndex++;
	}
	else if(lexTable[gLexIndex].token == ifsym){
		gLexIndex++;
		if(lexTable[gLexIndex].token == oddsym){
			gLexIndex++;
			exprReg = expression(lexTable);
			generate(15, exprReg, exprReg, 0);
		}
		else{
			exprReg = expression(lexTable);
			gLexIndex++;
			switch(lexTable[gLexIndex - 1].token){
				case eqsym:
					conditionType = 17;
					break;
				case neqsym:
					conditionType = 18;
					break;
				case lessym:
					conditionType = 19;
					break;
				case leqsym:
					conditionType = 20;
					break;
				case gtrsym:
					conditionType = 21;
					break;
				case geqsym:
					conditionType = 22;
					break;
			}
			exprReg2 = expression(lexTable);
			conditionReg = unusedReg();
			generate(conditionType, conditionReg, exprReg, exprReg2);
		}
		if(lexTable[gLexIndex].token != thensym)
			error(13);
		gLexIndex++;
		//If the if is true, continue through code otherwise jump past
		//Reserve space in code for JPC
		jpcGen = instIndex;
		instIndex++;
		statement(lexTable);
		//Generate function does not support creation of out of order instructions so this JPC is made manually.
		instructions[jpcGen][0] = 8;
		instructions[jpcGen][1] = conditionReg;
		instructions[jpcGen][2] = 0;
		instructions[jpcGen][3] = instIndex;
		
		if(lexTable[gLexIndex].token == elsesym){
			gLexIndex++;
			statement(lexTable);
		}
	}
	
	else if(lexTable[gLexIndex].token == whilesym){
		gLexIndex++;
		inSubStatement = 1;
		if(lexTable[gLexIndex].token == oddsym){
			gLexIndex++;
			exprReg = expression(lexTable);
			generate(15, exprReg, exprReg, 0);
		}
		else{
			exprReg = expression(lexTable);
			gLexIndex++;
			switch(lexTable[gLexIndex - 1].token){
				case eqsym:
					conditionType = 17;
					break;
				case neqsym:
					conditionType = 18;
					break;
				case lessym:
					conditionType = 19;
					break;
				case leqsym:
					conditionType = 20;
					break;
				case gtrsym:
					conditionType = 21;
					break;
				case geqsym:
					conditionType = 22;
					break;
			}
			exprReg2 = expression(lexTable);
			conditionReg = unusedReg();
			generate(conditionType, conditionReg, exprReg, exprReg2);
		}
		if(lexTable[gLexIndex].token != dosym)
			error(14);
		gLexIndex++;
		//Jump to JPC
		//Jump space holder
		jpcGen = instIndex;
		instIndex++;
		statement(lexTable);
		//The jump instruction at the start
		instructions[jpcGen][0] = 7;
		instructions[jpcGen][1] = 0;
		instructions[jpcGen][2] = 0;
		instructions[jpcGen][3] = instIndex;
		generate(8, conditionReg, 0, jpcGen + 1);
		//JPC should then go to Jump + 1 if condition is true and repeat until condition is no longer true and move on after
		inSubStatement = 0;
	}
	else if(lexTable[gLexIndex].token == readsym){
		gLexIndex++;
		symIndex = findSym(lexTable[gLexIndex].name);
		gLexIndex++;
		rwReg = unusedReg();
		generate(9, rwReg, 0, 2);
		generate(4, rwReg, 0, symbolTable[symIndex].address);
	}
	else if(lexTable[gLexIndex].token == writesym){
		gLexIndex++;
		if(lexTable[gLexIndex].token != identsym)
			error(23);
		symIndex = findSym(lexTable[gLexIndex].name);
		gLexIndex++;
		rwReg = unusedReg();
		generate(3, rwReg, 0, symbolTable[symIndex].address);
		generate(9, rwReg, 0, 1);
	}
	if(inSubStatement == 0)
		freeRegs();
}

//Condition is built into statement

int expression(Lex lexTable[]){
	int flagNegation = 0;
	int termRegs[14] = {0};
	int usedTermRegs = 0;
	if(lexTable[gLexIndex].token == plussym){
		gLexIndex++;
	}
	else if(lexTable[gLexIndex].token == minussym){
		gLexIndex++;
		flagNegation = 1;
	}
	termRegs[0] = term(lexTable);
	usedTermRegs++;
	if(flagNegation)
		generate(10, termRegs[0], termRegs[0], 0);
	while(lexTable[gLexIndex].token == plussym || lexTable[gLexIndex].token == minussym){
		flagNegation = 0;
		if(lexTable[gLexIndex].token == plussym)
			gLexIndex++;
		else if(lexTable[gLexIndex].token == minussym){
			gLexIndex++;
			flagNegation = 1;
		}
		termRegs[usedTermRegs] = term(lexTable);
		usedTermRegs++;
		if(flagNegation){
			generate(12, termRegs[0], termRegs[0], termRegs[usedTermRegs - 1]);
		}
		else{
			generate(11, termRegs[0], termRegs[0], termRegs[usedTermRegs - 1]);
		}
	}
	return termRegs[0];
}
int term(Lex lexTable[]){
	int returnReg = unusedReg();
	int divFlag = 0;
	factor(lexTable, returnReg);
	if(lexTable[gLexIndex].token == multsym || lexTable[gLexIndex].token == slashsym){
		if(lexTable[gLexIndex].token == slashsym)
			divFlag = 1;
		gLexIndex++;
		factor(lexTable, 15);
		if(divFlag == 0)
				generate(13, returnReg, returnReg, 15);
			else
				generate(14, returnReg, returnReg, 15);
		while(lexTable[gLexIndex].token == multsym || lexTable[gLexIndex].token == slashsym){
			divFlag = 0;
			if(lexTable[gLexIndex].token == slashsym)
				divFlag == 1;
			gLexIndex++;
			factor(lexTable, 15);
			if(divFlag == 0)
				generate(13, returnReg, returnReg, 15);
			else
				generate(14, returnReg, returnReg, 15);
		}
	}
	return returnReg;	
}
int factor(Lex lexTable[], int rg){
	int symIndex;
	if(lexTable[gLexIndex].token == identsym){
		symIndex = findSym(lexTable[gLexIndex].name);
		if(symbolTable[symIndex].kind == 1){
			generate(1, rg, 0, symbolTable[symIndex].val);
		}
		else if(symbolTable[symIndex].kind == 2){
			generate(3, rg, 0, symbolTable[symIndex].address);
		}
		else{
			error(11);
		}
		gLexIndex++;
		
	}
	else if(lexTable[gLexIndex].token == numbersym){
		generate(1, rg, 0, lexTable[gLexIndex].val);
		gLexIndex++;
		usedRegs[rg] = 1;
	}
	else if(lexTable[gLexIndex].token == lparentsym){
		gLexIndex++;
		expression(lexTable);
		if(lexTable[gLexIndex].token != rparentsym)
			error(9);
	}
	else
		error(10);
	return 0;
}

void printStack(int sp, int bp, int *stack, int lex){
     int i;
     if (bp == 1) {
     	if (lex > 0) {
	   printf("|");
	   }
     }	   
     else {
     	  //Print the lesser lexical level
     	  printStack(bp-1, stack[bp + 2], stack, lex-1);
	  printf("|");
     }
     //Print the stack contents - at the current level
     for (i = bp; i <= sp; i++) {
     	 printf("%3d ", stack[i]);	
     }
}

int base(int l, int bp, int *stack){
	int base = bp;
	while(l > 0){
		base = stack[base + 1];
		l--;
	}		
	return base;
}

int vm(int compilerDirectiveVm){
	char op[4] = {'L', 'I', 'T', '\0'};
	int run = 1;
	int i, j, k;
	int r[16] = {0};
	int inst[4]; //Instruction register, [OP, R, L, M]
	int sp = 0, bp = 1, pc = 0, lex = 0;
	int stack[2000] = {0};

	i = 0;
	j = 0;
	k = 0;

	//Begin running virtual machine
	if(compilerDirectiveVm)
		printf("\n OP Rg Lx Vl [ PC BP SP]  Stack\n");
	while(run == 1){
		for(i = 0; i < 4; i++){
			inst[i] = instructions[pc][i];
		}
			//printf("%2d %2d %2d %2d\n", inst[0], inst[1], inst[2], inst[3]);
		pc++;
		switch(inst[0]){
			//Literal into register
			case 1:
				op[0] = 'L';
				op[1] = 'I';
				op[2] = 'T';
				r[inst[1]] = inst[3];
				break;
			//Return
			case 2:
				op[0] = 'R';
				op[1] = 'T';
				op[2] = 'N';
				sp = bp -1;
				bp = stack[sp+3];
				pc = stack[sp+4];
				lex--;
				break;
			//Load
			case 3:
				op[0] = 'L';
				op[1] = 'O';
				op[2] = 'D';
				r[inst[1]] = stack[base(inst[2], bp, stack) + inst[3]];
				break;
			//Store
			case 4:
				op[0] = 'S';
				op[1] = 'T';
				op[2] = 'O';
				stack[base(inst[2], bp, stack) + inst[3]] = r[inst[1]];
				break;
			//Call
			case 5:
				op[0] = 'C';
				op[1] = 'A';
				op[2] = 'L';
				stack[sp+1] = 0;
				stack[sp+2] = base(inst[2], bp, stack);
				stack[sp+3] = bp;
				stack[sp+4] = pc;
				bp = sp+1;
				sp = sp+4;
				pc = inst[3];
				lex++;
				break;
			//Increment
			case 6:
				op[0] = 'I';
				op[1] = 'N';
				op[2] = 'C';
				sp = sp + inst[3];
				break;
			//Jump
			case 7:
				op[0] = 'J';
				op[1] = 'M';
				op[2] = 'P';
				pc = inst[3];
				break;
			//Conditional Jump
			case 8:
				op[0] = 'J';
				op[1] = 'P';
				op[2] = 'C';
				if(r[inst[1]] == 1)
					pc = inst[3];
				break;
			//System I/O
			case 9:
				op[0] = 'S';
				op[1] = 'I';
				op[2] = 'O';
				switch(inst[3]){
					case 1:
						printf("%d\n", r[inst[1]]);
						break;
					case 2:
						scanf("%d", &(r[inst[1]]));
						break;
					case 3:
						run = 0;
						break;
				}
				break;
			//Negation
			case 10:
				op[0] = 'N';
				op[1] = 'E';
				op[2] = 'G';
				r[inst[1]] = -r[inst[2]];
				break;
			//Addition
			case 11:
				op[0] = 'A';
				op[1] = 'D';
				op[2] = 'D';
				r[inst[1]] = r[inst[2]] + r[inst[3]];
				break;
			//Subtraction
			case 12:
				op[0] = 'S';
				op[1] = 'U';
				op[2] = 'B';
				r[inst[1]] = r[inst[2]] - r[inst[3]];
				break;
			//Multiplication
			case 13:
				op[0] = 'M';
				op[1] = 'U';
				op[2] = 'L';
				r[inst[1]] = (r[inst[2]] * r[inst[3]]);
				break;
			//Division
			case 14:
				op[0] = 'D';
				op[1] = 'I';
				op[2] = 'V';
				r[inst[1]] = r[inst[2]] / r[inst[3]];
				break;
			//isOdd
			case 15:
				op[0] = 'O';
				op[1] = 'D';
				op[2] = 'D';
				r[inst[1]] = r[inst[1]] % 2;
				break;
			//Modulo
			case 16:
				op[0] = 'M';
				op[1] = 'O';
				op[2] = 'D';
				r[inst[1]] = r[inst[2]] % r[inst[3]];
				break;
			//Logical Equal To
			case 17:
				op[0] = 'E';
				op[1] = 'Q';
				op[2] = 'L';
				r[inst[1]] = (r[inst[2]] == r[inst[3]]);
				break;
			//Logical Not Equal To
			case 18:
				op[0] = 'N';
				op[1] = 'E';
				op[2] = 'Q';
				r[inst[1]] = (r[inst[2]] != r[inst[3]]);
				break;
			//Logical Less Than
			case 19:
				op[0] = 'L';
				op[1] = 'S';
				op[2] = 'S';
				r[inst[1]] = (r[inst[2]] < r[inst[3]]);
				break;
			//Logical Less Than Or Equal
			case 20:
				op[0] = 'L';
				op[1] = 'E';
				op[2] = 'Q';
				r[inst[1]] = (r[inst[2]] <= r[inst[3]]);
				break;
			//Logical Greater Than
			case 21:
				op[0] = 'G';
				op[1] = 'T';
				op[2] = 'R';
				r[inst[1]] = (r[inst[2]] > r[inst[3]]);
				break;
			//Logical Greater Than Or Equal
			case 22:
				op[0] = 'G';
				op[1] = 'E';
				op[2] = 'Q';
				r[inst[1]] = (r[inst[2]] >= r[inst[3]]);
				break;
		}
		if(compilerDirectiveVm){
			//Printing the operating data
			printf("%-4s%3d%3d%3d[%3d%3d%3d]", op, inst[1], inst[2], inst[3], pc, bp, sp);
			printStack(sp, bp, stack, lex);		
			printf("\n\tRegisters:[ %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d ] \n", r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15]);
			printf("\n");
			fflush(stdout); 
		}

	}
	return 0;
}
