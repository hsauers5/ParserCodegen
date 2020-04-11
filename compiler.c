/*
    Tiny-PL/0 Compiler
    Harry Sauers
    Alex Ogilbee
    COP 3402 Spring 2020
*/


/*
Structure:
    - LexicalAnalyzer (Scanner) | lex_main()
    - Parser | parser()
    - CodeGen | codegen()
    - VM (HW1) | vm_main()
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_TABLE_SIZE 100

/* Catch Errors & Exit Program for Parser */

int HAS_ERROR = 0;
void error(int err) {
    HAS_ERROR = 1;
    int exit_program = 0; // 1 to exit, 0 to not
    
    switch(err) {
        case 1:
            printf("\n Error 001, period expected.\n");
            break;
        case 2:
            printf("\nError 002, identsym expected\n");
            break;
        case 3:
            printf("\nError 003 eqsym expected\n");
            break;
        case 4:
            printf("\nError 004 number expected\n");
            break;
        case 5:
            printf("\nError 005 semicolonsym expected\n");
            break;
        case 6:
            printf("\nError 006 becomessym expected\n");
            break;
        case 7:
            printf("\nError 007 endsym expected\n");
            break;
        case 8:
            printf("\nError 008 thensym expected\n");
            break;
        case 9:
            printf("\nError 009 dosym expected\n");
            break;
        case 10:
            printf("\nError 010 relation expected\n");
            break;
        case 11:
            printf("\nError 011 rparensym expected\n");
            break;
        case 12:
            printf("\nError 012 Factor conditions not met\n");
            break;
        case 13:
            printf("\nError 013 program too long\n");
            break;
		case 14:
			printf("\nError 014 symbol not in symbol table\n");
           break;
        } 
            
        if (exit_program == 1) {
            printf("Exiting...\n");
            exit(-1);
        }
}

/* LexicalAnalyzer */
/* ================================================================================== */

typedef struct {
	char * lexeme;
	int token_type;
} wordy;

typedef enum { 
    nulsym = 1, identsym, numbersym, plussym, minussym,	//1-5
    multsym,  slashsym, oddsym, eqlsym, neqsym, lessym, leqsym,		//6-12
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,	//13-18
    periodsym, becomessym, beginsym, endsym, ifsym, thensym, //19-24
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,	//25-31
    readsym , elsesym 	//32-33
} token_type;

char * reserved_words[] = {
    "const", "var", "procedure", "call", "begin", "end", "if", "then", "else", "while", "do", "read", "write", "odd"
};
int RESERVED_WORDS_LEN = 14;

char special_symbols[] = {
    '+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'
};
int SPECIAL_SYMBOLS_LEN = 14;

char invisible_chars[] = {
    '\t', ' ', '\n', '\r', '\v', '\f'
};
int INVISIBLE_CHARS_LEN = 6;

char * program_string = "";


// dynamic strcat
char * dynamic_strcat(char * base, char * added) {
    // resize the base String
    char * conjoined = (char *) malloc(strlen(base) + strlen(added) + 1);

    if (conjoined == NULL) {
        printf("Error: failed to create char array.\n");
        return NULL;
    }

    // strcat and go
    strcpy(conjoined, base);

    return strcat(conjoined, added);
}

// Checks if word is a reserved word
int is_reserved_word(char * word) {
    for (int i = 0; i < RESERVED_WORDS_LEN; i++) {
        if (strcmp(reserved_words[i], word) == 0) {
            return 1;
        }
    }
    
    return 0;
}

// Checks if symbol is special symbol
int is_special_symbol(char symbol) {
    for (int i = 0; i < SPECIAL_SYMBOLS_LEN; i++) {
        if (special_symbols[i] == symbol) {
            return 1;
        }
    }
    
    return 0;
}

// Checks if symbol is whitespace
int is_invisible_char(char symbol) {
    for (int i = 0; i < INVISIBLE_CHARS_LEN; i++) {
        if (invisible_chars[i] == symbol) {
            return 1;
        }
    }
    
    return 0;
}

// Checks if character is a number
int is_number(char c) {
    int n = c - '0';
    int nums[] = {0,1,2,3,4,5,6,7,8,9};
    
    for (int i = 0; i < 10; i++) {
        if (n == nums[i]) {
            return 1;
        }
    }
    return 0;
}

// Checks if character is a letter
int is_letter(char c) {
    int letter_min_1 = 'A';
    int letter_max_1 = 'Z';
    int letter_min_2 = 'a';
    int letter_max_2 = 'z';
    
    // check if within valid letter ranges
    if ( (c >= letter_min_1 && c <= letter_max_1) || (c >= letter_min_2 && c <= letter_max_2) ) {
        return 1;
    } else {
        return 0;
    }
}

// checks if all chars are numbers
int is_only_numbers(char * word) {
    for (int i = 0; i < strlen(word); i++) {
        if (! is_number(word[i])) {
            return 0;
        }
    }
    return 1;
}

// checks if word is made up of ONLY numbers and/or letters
int is_letters_and_numbers(char * word) {
    for (int i = 0; i < strlen(word); i++) {
        if (! is_letter(word[i]) && ! is_number(word[i])) {
            return 0;
        }
    }
    return 1;
}

// checks if word is too long
int is_word_too_big(char * word, int max) {
    if (strlen(word) <= max) {
        return 0;
    } else {
        return 1;
    }
}

void MalformedNumberError(void) {
    printf("Error: Malformed number.\n");
    exit(0);
}

void SyntaxError(void) {
    printf("Error: Syntax error.\n");
    exit(0);
}

void NumberTooBigError(void) {
    printf("Error: number too big\n");
    exit(0);
}

void StringTooLongError(void) {
    printf("Error: string too long\n");
    exit(0);
}

void MalformedStringError(void) {
    printf("Error: Malformed string. \n");
    exit(0);
}

void EOF_COMMENT_ERROR(void) {
    printf("Error: EOF reached while in comment.\n");
    exit(0);
}

void InvalidSymbolError(void) {
    printf("Error: invalid symbol. \n");
    exit(0);
}

char * INPUT_FILE;
wordy * word_list;
int word_count = 0;
int lex_main(void) {
    // so we want to have an input reader
    FILE *fp = fopen(INPUT_FILE, "r");
    FILE *out = fopen("lexer_output.txt", "w");
    
    if (fp == NULL) {
        printf("Error: Could not locate file.\n");
        exit(-1);
    }
    word_list = (wordy *)calloc(1, sizeof(wordy));
	int temp_token;
	char * current_word;
	word_list[0].lexeme = "poopy";
	word_list[0].token_type = 22;
    
    // begin output formatting:
    fprintf(out, "Source Program: \n");
    
    // and copy to output file
    char temp_char = fgetc(fp);
    while (temp_char != EOF) {

        char tmp[2];
        tmp[0] = temp_char;
        
        program_string = dynamic_strcat(program_string, tmp);
        
        fputc(temp_char, out);
        temp_char = fgetc(fp);
    }
    
    // printf("\n\n%s\n\n===================\n\n", program_string);
    
    // while we read it
    int i = 0;
    int in_comment = 0;
    int program_length = strlen(program_string);
    while (i < program_length) {
        char c = program_string[i];
        int token;
        
        // we run some methods that determine what the word is
        // check if a comment, if so then set comment var to 1? until newline OR */
        if (c == '/') {
            if (i + 1 < program_length) {
                if (program_string[i+1] == '*') {
                    in_comment = 1;
                    i += 2;
                }
            }
        }
        // if outside program
        if (i >= program_length && in_comment) {
            // ERROR
            EOF_COMMENT_ERROR();
        }
        
        // while in comment, 
        while (i < program_length && in_comment) {
            c = program_string[i];
            if (c == '*') {
                i += 1;
                
                if (i >= program_length) {
                    EOF_COMMENT_ERROR();
                }
                
                c = program_string[i];
                if (c == '/') {
                    // no longer in comment
                    in_comment = 0;
                    i += 1;
                    continue;
                }
            }
        }
        
        // filter out the whitespace
        if (is_invisible_char(c)) {
            i += 1;
            continue;
        }
        
        // build word
        current_word = (char *) calloc(2, sizeof(char));
        current_word[0] = c;
        int j = i + 1;
        
        if (is_special_symbol(current_word[0]) == 0) {
            while ( (is_invisible_char(program_string[j]) == 0) && (is_special_symbol(program_string[j]) == 0) && j < program_length) {
                char new[2];
                
                sprintf(new, "%c", program_string[j]);
                
                current_word = dynamic_strcat(current_word, new);
                j += 1;
            }
        }
        
        // check for special characters
        if (is_special_symbol(c)) {
            // get token
            switch(c) {
                case '+':
                    token = plussym;
                    break;
                case '-':
                    token = minussym;
                    break;
                case '*':
                    token = multsym;
                    break;
                case '/':
                    token = slashsym;
                    break;
                case '(':
                    token = lparentsym;
                    break;
                case ')':
                    token = rparentsym;
                    break;
                case '=':
                    token = eqlsym;
                    break;
                case ',':
                    token = commasym;
                    break;
                case '.':
                    token = periodsym;
                    break;
                case '<':
                    if (program_string[i+1] == '=') {
                        current_word = "<=";
                        token = leqsym;
                        j += 1;
                    } else {
                        token = lessym;
                    }
                    break;
                case '>':
                    if (program_string[i+1] == '=') {
                        current_word = ">=";
                        token = geqsym;
                        j += 1;
                    } else {
                        token = gtrsym;
                    }
                    break;
                case ';':
                    token = semicolonsym;
                    break;
                case ':':
                    if (program_string[i+1] == '=') {
                        current_word = ":=";
                        token = becomessym;
                        j += 1;
                    } else {
                        token = eqlsym;
                    }
                    break;
            }
            
        }
        // check if the string is a reserved word
        else if (is_reserved_word(current_word)) {
            // do stuff
            // printf("Reserved word found\n");
           
            // get token
            int index = -1;
            
            for (int i = 0; i < RESERVED_WORDS_LEN; i++) {
                if (strcmp(reserved_words[i], current_word) == 0) {
                    index = i;
                    break;
                }
            }
            
            switch(index) {
                case 0:
                    token = constsym;
                    break;
                case 1:
                    token = varsym;
                    break;
                case 2:
                    token = procsym;
                    break;
                case 3: 
                    token = callsym;
                    break;
                case 4:
                    token = beginsym;
                    break;
                case 5:
                    token = endsym;
                    break;
                case 6:
                    token = ifsym;
                    break;
                case 7: 
                    token = thensym;
                    break;
                case 8:
                    token = elsesym;
                    break;
                case 9:
                    token = whilesym;
                    break;
                case 10:
                    token = dosym;
                    break;
                case 11:
                    token = readsym;
                    break;
                case 12:
                    token = writesym;
                    break;
                case 13:
                    token = oddsym;
                    break;
            }
            
            
        // not reserved word
        } else {
            // let = word
            // 4 things to check: (return if any fail)
            
            // if let[0] is a number, check all for nonletters and return accordingly
            if (is_number(current_word[0])) {
                // if all numbers: treat as token #3 (literal), else: MalformedNumberError(), return/exit
                if (is_only_numbers(current_word)) {
                    token = numbersym;
                } else {
                    // printf("%s", current_word);
                    MalformedNumberError();
                }
                
                // check strlen <= 5: if true, we're good. else: NumberTooBigError(), return/exit
                if (is_word_too_big(current_word, 5)) {
                
                    // printf("%s", current_word);
                    NumberTooBigError();
                }
            }
            
            // if let[0] is a LETTER,
            else if (is_letter(current_word[0])) {
                // check if length <= 11
                if (is_word_too_big(current_word, 11)) {
                    // if true: nothing (see below) else: StringTooLongError(), return/exit
                    
                    printf("%s", current_word);
                    StringTooLongError();   
                }
                
                // if let[0] is a letter, check if subsequent chars are letters or numbers ONLY (not exclusive)
                if (is_letters_and_numbers(current_word)) {
                    // if true: token #2, else: MalformedStringError(), return/exit
                    token = identsym;
                } else {
                
                    // printf("%s", current_word);
                    MalformedStringError();
                }
            } else if (is_special_symbol(current_word[0]) == 0){
                InvalidSymbolError();
            }
        
        }
        
        // append {word/lexeme, token} to word_list[array]
        i = j;

		word_count++;
		word_list = (wordy *)realloc(word_list, sizeof(wordy) * word_count);
		word_list[word_count - 1].lexeme = current_word;
		word_list[word_count - 1].token_type = token;
    }
    
	fprintf(out, "\nLexeme Table: \n");
	fprintf(out, "lexeme\t\t\ttoken type \n");

    
	for (i = 0; i < word_count; i++) {
		fprintf(out, "%s\t\t\t%d \n", word_list[i].lexeme, word_list[i].token_type);
	}
	
    
	fprintf(out, "\nLexeme List \n");

    
	for (i = 0; i < word_count; i++) {
		fprintf(out, "%d ", word_list[i].token_type);

		if (word_list[i].token_type == identsym || word_list[i].token_type == numbersym)
			fprintf(out, "%s ", word_list[i].lexeme);
	}
	
	fprintf(out, "\n");

	free(current_word);
	// free(word_list);
	free(program_string);
    fclose(fp);
    fclose(out);
}

/* ================================================================================== */



/* Parser & CodeGen */
/* ================================================================================== */

 typedef struct {
     int op; //Opcode
     int R;
     int L;  //L
     int M;  //M
 } instruction;
 
void emit(int op, int r, int l, int m, instruction* code);

instruction assembly_array[MAX_SYMBOL_TABLE_SIZE];


// Recursive Descent Parser

typedef struct { 
    int kind; 
    // const = 1, var = 2, proc = 3
    
    char name[10];
    // name up to 11 chars
    
    int val; 
    // number (ASCII value) 
    
    int level; 
    // L level
    
    int addr; 
    // M address

	int mark;
	// 0 if to be used, 1 if its been "deleted"
} symbol; 
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int tp = 1; // 0 is sentinel: tp = table pointer.
int num_of_vars [MAX_SYMBOL_TABLE_SIZE];	// temporary solution to variable index
int num_vars_to_remove;

int is_valid_token(wordy check_token) {
    if (check_token.token_type >= 0 && check_token.token_type <= 33) {
        return 1;
    }
    
    return 0;
}
int token_counter = 0;

int get_token(void) {
    wordy val = word_list[token_counter];
    token_counter += 1;
    
  //   printf("%s\n", val.lexeme);
    
    if (is_valid_token(val)) {
        return val.token_type;
    }
    
    // printf("\nAAAAA! Invalid token!\n");
    
    return 0;
}

int is_relation(int check_token) {
    if ((check_token == eqlsym) || (check_token == neqsym) || (check_token == lessym) || (check_token == leqsym) || (check_token == gtrsym) || (check_token == geqsym)) {
        return 1;
    } else {
        return 0;
    }
}

int parser_program();
int parser_block();
int parser_statement(int lex_level);
int parser_condition(int lex_level);
int parser_expression(int lex_level);
int parser_term(int lex_level);
int parser_factor(int lex_level);
int find_in_symbol_table(char * name);
int mark_the_table(int lex_level);

int cx = 0;
int ctemp, cx1, cx2;

int reg_counter;
int loc, loc_two, loc_three;

int TOKEN;
int parser(void) {    
    parser_program();
    if (HAS_ERROR == 0) {
        printf("No errors, program is syntactically correct.\n");   
    }
}

int parser_program() {

	// initialize num_of_vars array
	int i;
	for (i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
		num_of_vars[i] = 0;

    TOKEN = get_token();
    parser_block(0);
    
    TOKEN = get_token();
    
    if (TOKEN != periodsym) {
            error(1);
    }
	emit(11,0,0,3, assembly_array);	// end of program
}

int parser_block(int lex_level) {
//	emit(6,0,0,4, assembly_array);	// first init

    if (TOKEN == constsym) {
        do {
			symbol_table[tp].kind = 1; // const
          TOKEN = get_token();
            if (TOKEN != identsym) {
                
                error(2);
                return 0;
			}
			strcpy(symbol_table[tp].name, word_list[token_counter - 1].lexeme);
            
            TOKEN = get_token();
            if (TOKEN != eqlsym) {
                
                error(3);
                return 0;
            }
            
            TOKEN = get_token();
            if (TOKEN != numbersym) {
                
                error(4);
                return 0;
            }
			symbol_table[tp].val = atoi(word_list[token_counter - 1].lexeme);
			symbol_table[tp].level = 0;
			symbol_table[tp].addr = 0;
          symbol_table[tp].mark = 0;
			tp++;

            TOKEN = get_token();
        } while (TOKEN == commasym);
        
        if (TOKEN != semicolonsym) {
            printf("const error");
                error(5);
            return 0;
        }
        
        TOKEN = get_token();
    }

	if (TOKEN == varsym) {
		do {
			symbol_table[tp].kind = 2; // 2 for var
			TOKEN = get_token();
			if (TOKEN != identsym) {
                error(2);
				return 0;
			}
			strcpy(symbol_table[tp].name, word_list[token_counter - 1].lexeme);
			symbol_table[tp].val = 0;
			symbol_table[tp].level = lex_level;
			symbol_table[tp].addr = num_of_vars[lex_level] + 4; // 4 for SL, DL, RA, FV.
			symbol_table[tp].mark = 0;
			tp++;
			num_of_vars[lex_level]++;

			TOKEN = get_token();
//			emit(6,0,0,1, assembly_array);		// create a new variable in the stack
		} while (TOKEN == commasym);

		if (TOKEN != semicolonsym) {
				                printf("var error");
                    error(5);
				    return 0;
				}
				
				TOKEN = get_token();
			}
		emit(6, 0, 0, 4+num_of_vars[lex_level], assembly_array);

    if (TOKEN == numbersym) {
        do {
            TOKEN = get_token();
            if (TOKEN != identsym) {
                
                error(2);
                return 0;
            }

            TOKEN = get_token();
        } while (TOKEN == commasym);

        if (TOKEN != semicolonsym) {
                        printf("number error");
            error(5);
            return 0;
        }
        TOKEN = get_token();
    }
    
    // @TODO NEW =============================================================================================
    while (TOKEN == procsym) {
		symbol_table[tp].kind = 3; // 3 for procedure
        TOKEN = get_token();
        if (TOKEN != identsym) {
            error(2);
            return 0;
        }
			strcpy(symbol_table[tp].name, word_list[token_counter - 1].lexeme);
			symbol_table[tp].val = 0;
			symbol_table[tp].level = lex_level;
			symbol_table[tp].addr = cx + 1;
			symbol_table[tp].mark = 0;
			tp++;
        TOKEN = get_token();
        if (TOKEN != semicolonsym) {
			            printf("proc error");
            error(5);
            return 0;
        }

        TOKEN = get_token();

		// enter the new block with lex level one higher
		int ctemp_proc = cx;
		emit(7, 0, 0, 0, assembly_array);	// jump past the procedure

        parser_block(lex_level+1);

		assembly_array[ctemp_proc].M = cx + 1;
		emit(2, 0, 0, 0, assembly_array);	// return after done with procedure

		// exit the block by marking all "deleted" variables
		num_vars_to_remove = mark_the_table(lex_level);
		num_of_vars[lex_level + 1] -= num_vars_to_remove;

		TOKEN = get_token();
        if (TOKEN != semicolonsym) {
			            printf("proc error 2?");
            error(5);
            return 0;
        }

        TOKEN = get_token();
    }
    
    parser_statement(lex_level);
}

int parser_statement(int lex_level) {
	reg_counter = 0;
    if (TOKEN == identsym) {
		loc_two = find_in_symbol_table(word_list[token_counter - 1].lexeme);
        TOKEN = get_token();
        if (TOKEN != becomessym) {
            
            error(6);
            return 0;
        }
        TOKEN = get_token();
        parser_expression(lex_level);
			int delta_level = lex_level - symbol_table[loc_two].level;
			emit(4, 0, delta_level, symbol_table[loc_two].addr, assembly_array);
    }
    // @TODO NEW ==========================================================================================
    else if (TOKEN == callsym) {
        TOKEN = get_token();
        if (TOKEN != identsym) {
            error(2);
            return 0;
        }
		loc_three = find_in_symbol_table(word_list[token_counter - 1].lexeme);
		// want to emit call to the proc's cx, which is stored in symbol_table.addr
		emit(5, 0, symbol_table[loc_three].level, symbol_table[loc_three].addr, assembly_array);
        TOKEN = get_token();
    }
    else if (TOKEN == beginsym) {
        TOKEN = get_token();
        parser_statement(lex_level);
        while (TOKEN == semicolonsym) {
            TOKEN = get_token();
            parser_statement(lex_level);
        }
        if (TOKEN != endsym) {
            error(7);
            return 0;
        }
    }
    else if (TOKEN == ifsym) {
        TOKEN = get_token();
        parser_condition(lex_level);
        if (TOKEN != thensym) {
            error(8);
            return 0;
        }
        TOKEN = get_token();
		ctemp = cx;
		emit(8, 0, 0, 0, assembly_array); //jpc
        parser_statement(lex_level);
		assembly_array[ctemp].M = cx;
    }
    else if (TOKEN == whilesym) {
		cx1 = cx;
        TOKEN = get_token();
        parser_condition(lex_level);
		cx2 = cx;
		emit(8, 0, 0, 0, assembly_array);	//jpc
        if (TOKEN != dosym) {
            
            error(9);
            return 0;
        }
        TOKEN = get_token();
        parser_statement(lex_level);
		emit(7, 0, 0, cx1, assembly_array);
		assembly_array[cx2].M = cx;
    }
    // write
    else if (TOKEN == writesym) {
        TOKEN = get_token();
        parser_expression(lex_level);
        emit(9, reg_counter, 0, 1, assembly_array);
    }
    // read
    else if (TOKEN == readsym) {
        // no idea what I'm doing here
        TOKEN = get_token();
        emit(10, reg_counter, 0, 1, assembly_array); // 1 or 2 for m?
			if (TOKEN != identsym) 
				error(2);
			loc = find_in_symbol_table(word_list[token_counter - 1].lexeme);
			// emit a LOD, reg_counter, L, M, ass
			emit(3, reg_counter, symbol_table[loc].level, symbol_table[loc].addr, assembly_array);	// lod
    }
}

int parser_condition(int lex_level) {
    if (TOKEN == oddsym) {
        TOKEN = get_token();
        parser_expression(lex_level);
    } else {
        parser_expression(lex_level);
        if (! is_relation(TOKEN)) {
            error(10);
            return 0;
        }
        TOKEN = get_token();
        parser_expression(lex_level);
    }
}

int parser_expression(int lex_level) {
    int addop;
    if (TOKEN == plussym || TOKEN == minussym) {
        addop = TOKEN;
        TOKEN = get_token();
        parser_term(lex_level);
        
        if (addop == minussym) {
            emit(12, reg_counter, 0, 1, assembly_array); 
        }
    } else {
        parser_term(lex_level);
    }
    
    while (TOKEN == plussym || TOKEN == minussym) {
        addop = TOKEN;
        TOKEN = get_token();
			reg_counter++;
        parser_term(lex_level);
			reg_counter--;
        
        if (addop == plussym) {
            emit(13, reg_counter, reg_counter, reg_counter + 1, assembly_array); 
        } else {
            emit(14, reg_counter, reg_counter, reg_counter + 1, assembly_array); 
        }
    }
}

int parser_term(int lex_level) {
    int mulop;
    parser_factor(lex_level);
    
    while (TOKEN == multsym || TOKEN == slashsym) {
        mulop = TOKEN;
        TOKEN = get_token();
		reg_counter++;
        parser_factor(lex_level);
		reg_counter--;
        
        if (TOKEN == multsym) {
            emit(15, reg_counter, reg_counter, reg_counter+1, assembly_array); 
        } else if (TOKEN == slashsym) {
            emit(16, reg_counter, reg_counter, reg_counter+1, assembly_array); 
        }
    }
}

// @TODO finish this
int parser_factor(int lex_level) {
//    while ((TOKEN==identsym) || (TOKEN==numbersym) || (TOKEN==lparentsym)) {
        if (TOKEN == identsym) {
			// find the l and m from the symbol table
			loc = find_in_symbol_table(word_list[token_counter - 1].lexeme);
			// emit a LOD, reg_counter, L, M, ass
			int delta_level = lex_level - symbol_table[loc].level;
			if (symbol_table[loc].kind == 2) {	// if its a variable
				emit(3, reg_counter, delta_level, symbol_table[loc].addr, assembly_array);	// lod
			} else {
				emit(1, reg_counter, 0, symbol_table[loc].val, assembly_array);
			}
            TOKEN = get_token();
        } else if (TOKEN == numbersym) {
			emit(1, reg_counter, 0, atoi(word_list[token_counter - 1].lexeme), assembly_array); // lit
            TOKEN = get_token();
        } else if (TOKEN == lparentsym) {
            TOKEN = get_token();
				reg_counter++;
            parser_expression(lex_level);
				reg_counter--;

            if (TOKEN != rparentsym) {
                error(11);
                return 0;
            }

            TOKEN = get_token();
        } else {
            error(12);
            return 0;
        }
//    }
}


/* ================================================================================== */


/* CodeGen */
/* ================================================================================== */


void emit(int op, int r, int l, int m, instruction* code) {
    if (cx > MAX_SYMBOL_TABLE_SIZE) {
        error(13);
        return;
    } else {
        code[cx].op = op; 	//opcode
        code[cx].R = r; // register
        code[cx].L = l;	// lexicographical level
        code[cx].M = m;	// modifier
        cx++;
    }
}

int find_in_symbol_table(char * name) {
	int seek;

	for (seek = tp; seek > 0; seek--) {
		if (symbol_table[seek].mark == 0 &&
			  strcmp(symbol_table[seek].name, name) == 0) {
			printf("found %s at level %d.\n", symbol_table[seek].name, symbol_table[seek].level);
			return seek;
		}
	}
	error(14);
}

int mark_the_table(int lex_level) {
	// passes the level one lower than the one we're removing
	int seek = tp - 1;
	int ret = 0;

	while (symbol_table[seek].level > lex_level) {
		symbol_table[seek].mark = 1;
		ret++;
		seek--;
		printf("removed %s at level %d.\n", symbol_table[seek].name, symbol_table[seek].level);
	}

	return ret;
}

int codegen(void) {
    char * output = "";
    int counter = 0;
    while (&assembly_array[counter] != NULL && assembly_array[counter].op != 0 && counter < 100) {
        char buf[3];
        
        sprintf(buf, "%d ", assembly_array[counter].op);
        output = dynamic_strcat(output, buf);
        
        sprintf(buf, "%d ", assembly_array[counter].R);
        output = dynamic_strcat(output, buf);
        
        sprintf(buf, "%d ", assembly_array[counter].L);
        output = dynamic_strcat(output, buf);
        
        sprintf(buf, "%d ", assembly_array[counter].M);
        output = dynamic_strcat(output, buf);
        
        output = dynamic_strcat(output, "\n");
        // printf("OUTPUT: %s", output);

		counter++;
    }
    
    FILE * fp;
    fp = fopen("vm_input.txt", "w");
    // fprintf(fp, "Line\tOP\tR L M\n");
    fprintf(fp, "%s", output);
    fclose(fp);
    
    return 0;
}

/* ================================================================================== */


/* VM - pmachine */
/* ================================================================================== */

// Alex Ogilbee, Harry Sauers
// COP 3402, Spring 2020
// HW1: PM/0, pmachine.c

#define MAX_DATA_STACK_HEIGHT 40
#define MAX_CODE_LENGTH 200
#define MAX_LEXI_LEVEL 3
#define REG_FILE_LENGTH 8

// stack, init to 0
int stack[MAX_DATA_STACK_HEIGHT];

int lex[MAX_LEXI_LEVEL];

// holds indexes for pipe-delimiting, init to 0
int pipes[MAX_DATA_STACK_HEIGHT];

// output mega strings (one is array of strings, two is mega-string)
char ** output_one;
char * output_two = {"\t\tpc\tbp\tsp\tregisters\nInitial values\t"};

void update_output_one(char * OP, int R, int L, int M, int i);
void update_output_two();
char * dynamic_strcat(char * base, char * added);
char * convert_tabs_to_spaces(char * str);

int halt = 0;

// registers
int PC = 0;
int IR = 0;
int BP = 1; // base pointer
int SP = 0; // stack pointer

int REG[REG_FILE_LENGTH];

/* Begin P-Machine */
// instruction format from file is {OP LEVEL M} space-separated.

/* // SEE ABOVE
typedef struct instruction {
    int op;
    int reg;
    int l;
    int m;
} instruction;
*/

/* find base L levels down */
// l stand for L in the instruction format
int base(int l, int base) {  
  int b1; 
  b1 = base; 
  while (l > 0) {
    b1 = stack[b1 + 1];
    l--;
  }
  return b1;
}

/*

// LIT
int LIT(int R, int zero, int M) {
    REG[R] = M;
}

*/

// MATH
// OP determines operation to perform
int MATH(int OP, int R, int L, int M) {
    // i = R, j=L, R=REG, k=M
    switch(OP) {
        case 12:
            // NEG
            	REG[R] = -REG[L];
            break;
        case 13:
            // ADD
            	REG[R] = REG[L] + REG[M];
            break;
        case 14: 
            // SUB
            	REG[R] = REG[L] - REG[M];
            break;
        case 15:
            // MUL
            	REG[R] = REG[L] * REG[M];
            break;
        case 16:
            // DIV
            	REG[R] = REG[L] / REG[M];
            break;
        case 17:
            // ODD
            	REG[R] = REG[R] % 2;
            break;
        case 18:
            // MOD
            	REG[R] = REG[L] % REG[M];
            break;
        case 19:
            // EQL
            	REG[R] = (REG[L] == REG[M]);
            break;
        case 20:
            // NEQ
            	REG[R] = (REG[L] != REG[M]);
            break;
        case 21:
            // LSS
            	REG[R] = (REG[L] < REG[M]);
            break;
        case 22:
            // LEQ
            	REG[R] = (REG[L] <= REG[M]);
            break;
        case 23:
            // GTR
            	REG[R] = (REG[L] > REG[M]);
            break;
        case 24:
            // GEQ
            	REG[R] = (REG[L] >= REG[M]);
            break;
    }
}

/*

int RTN(int zero1, int zero2, int zero3) {
    SP = BP - 1;
    BP = stack[SP + 3];
    PC = stack[SP + 4];
}

int LOD(int R, int L, int M) {
    REG[R] = stack[base(L, BP) + M];
}

int STO(int R, int L, int M) {
    stack[base(L, BP) + M] = REG[R];
}

int CAL(int zero, int L, int M) {
    stack[SP + 1] = 0;
    stack[SP + 2] = base(L, BP);
    stack[SP + 3] = BP;
    stack[SP + 4] = PC;
    BP = SP + 1;
    PC = M;
}

int INC(int zero1, int zero2, int M) {
    SP += M;
}

int JMP(int zero1, int zero2, int M) {
    PC = M;
}

int JPC(int R, int zero, int M) {
    if (stack[SP] == 0) {
        PC = M;
    }
    // SP += 1;
}

int SIO(int R, int zero, int M) {
    switch(M) {
        case 1:
            printf("%d", REG[SP]);
            // SP += 1;
            break;
        case 2:
            SP -= 1;
            // read user input into stack
            printf("Enter integer to push to stack: ");
            scanf("%d", &REG[SP]);
            break;
        case 3:
            halt = 1;
    }
}
*/

// ISA
int do_operation(instruction instr) {
    int M = instr.M;
    int L = instr.L;
    int operation = instr.op;
    int R = instr.R;
	char * tabless_str;

	PC++;
	tabless_str = convert_tabs_to_spaces(output_one[PC-1]);
	output_two = dynamic_strcat(output_two, tabless_str);
	output_two = dynamic_strcat(output_two, "\t");
	
    // printf("%d\n", operation);
    
    switch(operation) {
        case 01:
            // LIT, R, 0, M
            // Push literal M onto data or stack
            REG[R] = M;
            break;
        case 02:
            // RTN, 0, 0, 0
            // Operation to be performed on the data at the top of the stack
            pipes[BP] = 0;
            SP = BP - 1;
            BP = stack[SP + 3];
            PC = stack[SP + 4];
            break;
        case 03:
            // LOD, R, L, M
            // Load value to top of stack from the stack location at offset M
			// from L lexicographical levels down
            REG[R] = stack[base(L, BP) + M];
            break;
        case 04: 
            // STO, R, L, M
            // Store value at top of stack in the stack location at offset M
			// from L lexi levels down
        	stack[base(L, BP) + M] = REG[R];
            break;
        case 05:
            // CAL, 0, L, M
            // Call procedure at code index M
            // Generate new activation record and pc <- M
            
            // notate pipe
            pipes[SP + 1] = 1;
            
            stack[SP + 1] = 0;
            stack[SP + 2] = base(L, BP);
            stack[SP + 3] = BP;
            stack[SP + 4] = PC;
            BP = SP + 1;
            PC = M;
            break;
        case 06:
            // INC 0, 0, M
            // Allocate M locals, increment SP by M. 
            // First 3 are Static Link (SL), Dynamic Link (DL), and return address (RA)
            SP += M;
            break;
        case 07:
            // JMP 0, 0, M
            // Jump to instruction M
            PC = M;
            break;
        case 8:
            // JPC, R, 0, M
            // Jump to instruction M if top stack element == 0
            if (REG[R] == 0) {
                PC = M;
            }
            break;
        case 9:
            // SIO, R, 0, 1
            // Write top of stack to screen
            // pop? peek? who knows

            printf("%d", REG[SP]); // @TODO what ???
            // stack[SP] = REG[R];
            
            // SP += 1;
            break;
        case 10:
            // SIO, R, 0, 2
            // Read in input from user and store at top of stack
            SP -= 1;
            // read user input into stack
            printf("Enter integer to push to stack: ");
            scanf("%d", &REG[SP]);
            break;
        case 11: 
            // SIO, R, 0, 3
            // End of program: halt condition
            halt = 1;
            break;
        default:
            if (operation >= 12) {
                MATH(operation, R, L, M);
            }
            break;
    }
	update_output_two();
}

int read_in(FILE * fp, instruction * text) {
	int lines_of_text = 0;
	while (fscanf(fp, "%d %d %d %d", &text[lines_of_text].op, &text[lines_of_text].R, &text[lines_of_text].L, &text[lines_of_text].M) != EOF) {
		lines_of_text++;
	}
	
	return lines_of_text;
}

void update_output_one(char * OP, int R, int L, int M, int i) {
	char tmp[3];
	output_one[i] = dynamic_strcat(output_one[i], OP);
	
	sprintf(tmp, "\t%d", R);
	output_one[i] = dynamic_strcat(output_one[i], tmp);
	
	sprintf(tmp, " %d", L);
	output_one[i] = dynamic_strcat(output_one[i], tmp);

	sprintf(tmp, " %d", M);
	output_one[i] = dynamic_strcat(output_one[i], tmp);

	return;
}

void update_output_two() {
	char tmp[6];
	int i;

	// update the registers
	sprintf(tmp, "%d\t", PC);
	output_two = dynamic_strcat(output_two, tmp);

	sprintf(tmp, "%d\t", BP);
	output_two = dynamic_strcat(output_two, tmp);

	sprintf(tmp, "%d\t", SP);
	output_two = dynamic_strcat(output_two, tmp);

	for (i = 0; i < REG_FILE_LENGTH - 1; i++) {
		sprintf(tmp, "%d ", REG[i]);
		output_two = dynamic_strcat(output_two, tmp);
	}
	
	sprintf(tmp, "%d\n", REG[REG_FILE_LENGTH - 1]);
	output_two = dynamic_strcat(output_two, tmp);

	// update the stack
	output_two = dynamic_strcat(output_two, "Stack:");
	for (i = 1; i < SP+1; i++) {
	    if (pipes[i] == 1) {
	        sprintf(tmp, " | %d", stack[i]);
	    } else {
		    sprintf(tmp, " %d", stack[i]);
	    }
		output_two = dynamic_strcat(output_two, tmp);		
	}
	output_two = dynamic_strcat(output_two, "\n\n");		

	return;
}

char * ins_to_string(instruction ins, int i) {
	char * tmp = (char *)calloc(4, sizeof(char));

	// determine the OP
	switch(ins.op) {
		case 1:
			tmp = "lit";
			break;
		case 2:
			tmp = "rtn";
			break;
		case 3:
			tmp = "lod";
			break;
		case 4:
			tmp = "sto";
			break;
		case 5:
			tmp = "cal";
			break;
		case 6:
			tmp = "inc";
			break;
		case 7:
			tmp = "jmp";
			break;
		case 8:
			tmp = "jpc";
			break;
		case 9:
		case 10:
		case 11:
			tmp = "sio";
			break;
		case 12:
			tmp = "neg";
			break;
		case 13:
			tmp = "add";
			break;
		case 14:
			tmp = "sub";
			break;
		case 15:
			tmp = "mul";
			break;
		case 16:
			tmp = "div";
			break;
		case 17:
			tmp = "odd";
			break;
		case 18:
			tmp = "mod";
			break;
		case 19:
			tmp = "eql";
			break;
		case 20:
			tmp = "neq";
			break;
		case 21:
			tmp = "lss";
			break;
		case 22:
			tmp = "leq";
			break;
		case 23:
			tmp = "gtr";
			break;
		case 24:
			tmp = "geq";
			break;
		default:
			printf("literally not possible");
	}
	// function the rest
	update_output_one(tmp, ins.R, ins.L, ins.M, i);

	return output_one[i];
}

char * convert_tabs_to_spaces(char * str) {
	int i;
	int length = strlen(str);
	char * cpy = (char *)calloc(length, sizeof(char *));
	cpy = strcpy(cpy, str);
	for (i = 0; i < length; i++)
		if (cpy[i] == '\t')
			cpy[i] = ' ';
	return cpy;
}

int vm_main(void) {
	char line_index[4];
	int lines_of_text;
	int i;
	instruction * text = (instruction *)calloc(MAX_CODE_LENGTH, sizeof(instruction));

    // init stack
    for (int i = 0; i < MAX_DATA_STACK_HEIGHT; i++) {
        stack[i] = 0;
    }
    
    // init REG
    for (int i = 0; i < REG_FILE_LENGTH; i++) {
        REG[i] = 0;
    }
    
    // init pipes
    for (int i = 0; i < MAX_DATA_STACK_HEIGHT; i++) {
        pipes[i] = 0;
    }

	FILE *fp = fopen("vm_input.txt", "r");
	if (fp == NULL) {
		printf("Error: Could not locate file.\n");
		exit(-1);
	}
	
	// record number of operations while reading from input.txt
	lines_of_text = read_in(fp, text);

	// initialize output one double array
	output_one = (char **)calloc(lines_of_text, sizeof(char *));
	
	// loop to create output one
	for (i = 0; i < lines_of_text; i++) {
		output_one[i] = (char *)calloc(1, sizeof(char));
		sprintf(line_index, "%d\t", i);
		output_one[i] = dynamic_strcat(output_one[i], line_index);
		output_one[i] = ins_to_string(text[i], i);
	}

	// instruction fetch and execute
	update_output_two();
	while(!halt) {
		if (halt)
			break;
		do_operation(text[PC]);
	}

	// write the strings to the output file		
	fclose(fp);
	fp = fopen("output.txt", "w");
	fprintf(fp, "Line\tOP\tR L M\n");
	for (i = 0; i < lines_of_text; i++) {
		fprintf(fp, "%s\n", output_one[i]);
	}
	fprintf(fp, "%s\n", output_two);

	fclose(fp);
	free(text);
	free(output_one);
    
	return 0;
}


/* ================================================================================== */

char * get_symbolic_representation(int token_type) {
    char * symbols[] = { "nulsym", "identsym", "numbersym", "plussym", "minussym", 
                    "multsym", " slashsym", "oddsym", "eqlsym", "neqsym", 
                    "lessym", "leqsym", "gtrsym", "geqsym", "lparentsym", 
                    "rparentsym", "commasym", "semicolonsym", "periodsym", "becomessym", 
                    "beginsym", "endsym", "ifsym", "thensym", "whilesym", 
                    "dosym", "callsym", "constsym", "varsym", "procsym", 
                    "writesym", "readsym ", "elsesym" };
                    
    return symbols[token_type-1];
}

int main(int argc, char* argv[]) {

    // accept input filename from command line
    char * file_name = argv[1];
    INPUT_FILE = file_name;
    
    // printf("Lexer\n");
    lex_main();
    
    // printf("Parser\n");
    parser();

    /* 
	int i;
	for(i = 0; i < cx; i++)
		printf("%d %d %d %d\n", assembly_array[i].op, assembly_array[i].R, assembly_array[i].L, assembly_array[i].M);
    */
    
    // printf("Codegen\n");
    codegen();
    
    // printf("VM\n");
    
    /* ==== PRINTING CLI ARGS ==== */
    
    char * lexer_output_arg = "-l";
    char * parser_output_arg = "-a";
    char * vm_execution_trace_arg = "-v";
    
    int print_lexer_output=0, print_parser_output=0, print_vm_trace = 0;
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], lexer_output_arg) == 0) {
            print_lexer_output = 1;
        }
        if (strcmp(argv[i], parser_output_arg) == 0) {
            print_parser_output = 1;
        }
        if (strcmp(argv[i], vm_execution_trace_arg) == 0) {
            print_vm_trace = 1;
        }
    }
    
    // === If program has error, don't continue w/ execution
    
    if (HAS_ERROR == 1) {
        print_lexer_output = 1;
        print_parser_output = 0;
        print_vm_trace = 0;
    } else {
        // print generated code
        print_parser_output = 1;
    }
    
    if (print_lexer_output) {
        printf("\n\nLexer Output: \n");
        printf("======================================\n");
        /*
        // print lexer output - lexer_output.txt
        FILE * f = fopen("lexer_output.txt", "r");
        char s;
        while((s = fgetc(f)) != EOF) {
          printf("%c", s);
        }
        fclose(f);
        */
        
        for (int i = 0; i < word_count; i++) {
            printf("%d ", word_list[i].token_type);
            if (word_list[i].token_type == identsym || word_list[i].token_type == numbersym) {
			    printf("%s ", word_list[i].lexeme);
		    }
        }
        printf("\n");
        for (int i = 0; i < word_count; i++) {
            printf("%s ", get_symbolic_representation(word_list[i].token_type));
            if (word_list[i].token_type == identsym || word_list[i].token_type == numbersym) {
			    printf("%s ", word_list[i].lexeme);
		    }
        }
        printf("\n");
    }
    
    if (print_parser_output) {
        printf("\n\nParser Output: \n");
        printf("======================================\n");
        // print parser output - vm_input.txt
        FILE * f = fopen("vm_input.txt", "r");
        char s;
        while((s = fgetc(f)) != EOF) {
          printf("%c", s);
        }
        fclose(f);
        
        printf("\n");
    }
    
    // run on VM
    if (HAS_ERROR == 0) {
        vm_main();
    }
    
    if (print_vm_trace) {
        printf("\n\nVM Execution Trace: \n");
        printf("======================================\n");
        // print vm trace - output.txt
        FILE * f = fopen("output.txt", "r");
        char s;
        while((s = fgetc(f)) != EOF) {
          printf("%c", s);
        }
        fclose(f);
        
        printf("\n");
    }
    
    printf("\n");
    
    free(word_list);
}
