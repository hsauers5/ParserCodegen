/*
    Tiny-PL/0 Compiler
    Harry Sauers
    Alex Ogilbee
    COP 3402 Spring 2020
*/


/*
Structure:
    - LexicalAnalyzer (Scanner) | lex_main()
    - Parser & CodeGen @TODO  | parser()
    - VM (HW1) ** Fix factorial program issues ** | 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* LexicalAnalyzer */
/* ================================================================================== */

typedef struct {
	char * lexeme;
	int token_type;
} wordy;

typedef enum { 
    nulsym = 1, identsym, numbersym, plussym, minussym,
    multsym,  slashsym, oddsym, eqlsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym, 
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym 
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
    char * conjoined = (char *)malloc(strlen(base) + strlen(added) + 1);

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


wordy * word_list;
int lex_main(void) {
    // so we want to have an input reader
    FILE *fp = fopen("input.txt", "r");
    FILE *out = fopen("output.txt", "w");
    
    if (fp == NULL) {
        printf("Error: Could not locate file.\n");
        exit(-1);
    }
    word_list = (wordy *)calloc(1, sizeof(wordy));
	int word_count = 0;
	int temp_token;
	char * current_word;
	word_list[0].lexeme = "poopy";
	word_list[0].token_type = 22;
    
    // begin output formatting:
    fprintf(out, "Source Program: \n");
    
    // and copy to output file
    char temp_char = fgetc(fp);
    while (temp_char != EOF) {
        program_string = dynamic_strcat(program_string, &temp_char);
        
        fputc(temp_char, out);
        temp_char = fgetc(fp);
    }
    
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
                    } else {
                        token = lessym;
                    }
                    break;
                case '>':
                    if (program_string[i+1] == '=') {
                        current_word = ">=";
                        token = geqsym;
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
                    } else {
                        token = eqlsym;
                    }
                    break;
            }
            
        }
        // check if the string is a reserved word
        else if (is_reserved_word(current_word)) {
            // do stuff
            printf("Reserved word found\n");
           
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
                    printf("%s", current_word);
                    MalformedNumberError();
                }
                
                // check strlen <= 5: if true, we're good. else: NumberTooBigError(), return/exit
                if (is_word_too_big(current_word, 5)) {
                
                    printf("%s", current_word);
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
                
                    printf("%s", current_word);
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

// iterate through word_list
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
    
    if (is_valid_token(val)) {
        return val.token_type;
    }
    
    printf("\nAAAAA! Invalid token!\n");
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
int parser_statement();
int parser_condition();
int parser_expression();
int parser_term();
int parser_factor();


int TOKEN;
int parser(void) {
    // TOKEN = get_token();
    
    /*
    while (is_valid_token(this_token)) {
        printf("%d ", this_token.token_type);    
        parser_program(token);
        
        // next token
        this_token = get_token();
    }
    */
    
    parser_program();
}

int parser_program() {
    TOKEN = get_token();
    parser_block();
    
    if (TOKEN != periodsym) {
        // @TODO ERROR - Error number xxx, period expected
        printf("\n Error 001, period expected.\n");
    }
}

int parser_block() {
    if (TOKEN == constsym) {
        do {
            TOKEN = get_token();
            if (TOKEN != identsym) {
                // @TODO error
                printf("\nError identsym expected\n");
                return 0;
            }
            
            TOKEN = get_token();
            if (TOKEN != eqsym) {
                // @TODO error
                printf("\nError eqsym expected\n");
                return 0;
            }
            
            TOKEN = get_token();
            if (TOKEN != numbersym) {
                // @TODO error
                printf("\nError number expected\n");
                return 0;
            }
            
            TOKEN = get_token();
        } while (TOKEN == commasym);
        
        if (TOKEN != semicolonsym) {
            // @TODO error
            printf("\nError semicolonsym expected\n");
            return 0;
        }
        
        TOKEN = get_token();
    }
    
    if (TOKEN == numbersym) {
        do {
            TOKEN = get_token();
            if (TOKEN != identsym) {
                // @TODO error
                printf("Error expected identsym\n");
                return 0;
            }

            TOKEN = get_token();
        } while (TOKEN == commasym);

        if (TOKEN != semicolonsym) {
            // @TODO error
            printf("\nError semicolonsym expected\n");
            return 0;
        }
        TOKEN = get_token();
    }

    while (TOKEN == procsym) {
        TOKEN = get_token();
        if (TOKEN != identsym) {
            // @TODO error
            printf("\nError identsym expected\n");
            return 0;
        }

        TOKEN = get_token();
        if (TOKEN != semicolonsym) {
            // @TODO error
            printf("\nError semicolonsym expected\n");
            return 0;
        }

        TOKEN = get_token();

        parser_block();

        if (TOKEN != semicolonsym) {
            // @TODO error
            printf("\nError semicolonsym expected\n");
            return 0;
        }

        TOKEN = get_token();
    }
    parser_statement();
}

int parser_statement() {
    if (TOKEN == identsym) {
        TOKEN = get_token();
        if (TOKEN != becomessym) {
            // @TODO error
            printf("\nError: expected becomessym\n");
            return 0;
        }
        TOKEN = get_token();
        parser_expression();
    }
    else if (TOKEN == callsym) {
        TOKEN = get_token();
        if (TOKEN != identsym) {
            // @TODO error
            printf("\n Error identsym expected\n");
            return 0;
        }
        TOKEN = get_token();
    }
    else if (TOKEN == beginsym) {
        TOKEN = get_token();
        parser_statement();
        while (TOKEN == semicolonsym) {
            TOKEN = get_token();
            parser_statement();
        }
        if (TOKEN != endsym) {
            // @TODO error
            printf("\n Error endsym expected\n");
            return 0;
        }
    }
    else if (TOKEN == ifsym) {
        TOKEN = get_token();
        parser_condition();
        if (TOKEN != thensym) {
            // @TODO error
            printf("\n Error thensym expected");
            return 0;
        }
        TOKEN = get_token();
        parser_statement();
    }
    else if (TOKEN == whilesym) {
        TOKEN = get_token();
        parser_condition();
        if (TOKEN != dosym) {
            // @TODO error
            printf("\n Error dosym expected");
            return 0;
        }
        TOKEN = get_token();
        parser_statement();
    }
}

int parser_condition() {
    if (TOKEN == oddsym) {
        TOKEN = get_token();
        parser_expression();
    } else {
        parser_expression();
        if (! is_relation(TOKEN)) {
            // @ TODO relation error
            printf("\nError: relation expected");
            return 0;
        }
        TOKEN = get_token();
        parser_expression();
    }
}

int parser_expression() {
    if (TOKEN == plussym || TOKEN == minussym) {
        TOKEN = get_token();
    }
    parser_term();
    while (TOKEN == plussym || TOKEN == minussym) {
        TOKEN = get_token();
        parser_term();
    }
}

int parser_term() {
    parser_factor();
    while (TOKEN == multsym || TOKEN == slashsym) {
        TOKEN = get_token();
        parser_factor();
    }
}

int parser_factor() {
    if (TOKEN == identsym) {
        TOKEN = get_token();
    } else if (TOKEN == numbersym) {
        TOKEN = get_token();
    } else if (TOKEN == lparentsym) {
        TOKEN = get_token();
        parser_expression();

        if (TOKEN != rparentsym) {
            // @TODO paren error
            printf("\n Error: right paren expected");
            return 0;
        }

        TOKEN = get_token();
    } else {
        // @TODO factor error
        printf("\n Error: factor conditions not met.");
        return 0;
    }
}


/* ================================================================================== */



/* VM - pmachine */
/* ================================================================================== */



/* ================================================================================== */

int main(void) {
    // printf("Hello world. I work!\n");
    lex_main();
    parser();
    
    
    free(word_list);
}
