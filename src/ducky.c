#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#include "hid_keycodes.h"

char example_script[] = "\
STRING cooltest\n\
WINDOWS r\n\
REM A slightly more advanced Hello, World! for Windows\n\
DELAY 3000\n\
REM Open the Run dialog\n\
WINDOWS r\n\
DELAY 1000\n\
REM Open powershell with our message\n\
STRING powershell \"echo 'Hello, World!'; pause\"\n\
ENTER\n\
";

void asciiToKeyCode(char c, uint8_t *keycode, uint8_t *mod) {
    *keycode = (uint8_t)c;
    *mod = 0;
}

enum TOKEN {
    STRING,
    WINDOWS,
    LITERAL,
    CHAR,
    REM,
    COMMENT,
    DELAY,
    INT,
    ENTER,
};

int tokenizeScript(char *script, char **outTokens, enum TOKEN *outTokenTypes) {
    char *token = strtok(script, "\n ");
    int numTokens = 0;
    int i=0;
    while (token != NULL) {
        printf("%s: ", token);
        outTokens[i] = token;

        if(strcmp(token, "STRING") == 0) {
            outTokenTypes[i] = STRING;
            printf("STRING\n");
            // Capture the entire line after STRING
            token = strtok(NULL, "\n");
            if (token != NULL) {
                i++;
                outTokens[i] = token;
                outTokenTypes[i] = LITERAL;
                printf("LITERAL\n");
                numTokens++;
            }
        } else if (strcmp(token, "REM") == 0) {
            outTokenTypes[i] = REM;
            printf("REM\n");
            // Capture the entire line after REM
            token = strtok(NULL, "\n");
            if (token != NULL) {
                i++;
                outTokens[i] = token;
                outTokenTypes[i] = COMMENT;
                printf("COMMENT\n");
                numTokens++;
            }
        } else if(strcmp(token, "WINDOWS") == 0) {
            outTokenTypes[i] = WINDOWS;
            printf("WINDOWS\n");
        } else if(strlen(token) == 1 && token[0] < 127 && token[0] > 31) {
            outTokenTypes[i] = CHAR;
            printf("CHAR\n");
        } else if(strcmp(token, "DELAY") == 0) {
            outTokenTypes[i] = DELAY;
            printf("DELAY\n");
            // Capture the entire line after DELAY
            token = strtok(NULL, "\n");
            if (token != NULL) {
                i++;
                outTokens[i] = token;
                outTokenTypes[i] = INT;
                printf("INT\n");
                numTokens++;
            }
        } else if(strcmp(token, "ENTER") == 0) {
            outTokenTypes[i] = ENTER;
            printf("ENTER\n");
        } else {
            outTokenTypes[i] = LITERAL;
            printf("LITERAL\n");
        }

        token = strtok(NULL, "\n ");
        i++;
        numTokens++;
    }

    return numTokens;
}

bool parseTokens(char **tokens, enum TOKEN *token_types, int numTokens) {
    bool syntaxError = false;
    printf("\nPARSING!\n");
    int keyIdx = 0; // Index into output keycodes
    int i = 0; // Index into tokens
    while(i < numTokens) {
        printf("Token: %s\n", tokens[i]);
        switch(token_types[i]) {
            case STRING:
                i++;
                if(i >= numTokens) {
                    printf("SYNTAX ERROR: UNEXPECTED EOF. EXPECTED LITERAL AFTER STRING\n");
                    syntaxError = true;
                    break;
                }

                // If next token is a literal, add it to the keycodes
                if(token_types[i] == LITERAL) {
                    char *token = tokens[i];
                    printf("Keycode literal added: %s\n", token);
                    keyIdx++;
                } else {
                    printf("SYNTAX ERROR: EXPECTED LITERAL AFTER STRING\n");
                    syntaxError = true;
                }

                break;

            case WINDOWS:
                // ADD WINDOWS KEY
                i++;
                if(i >= numTokens) {
                    printf("SYNTAX ERROR: UNEXPECTED EOF. EXPECTED CHAR AFTER WINDOWS\n");
                    syntaxError = true;
                    break;
                }

                if(token_types[i] == CHAR) {
                    char *token = tokens[i];
                    printf("Keycode added: WIN+%c\n", token[0]);
                    keyIdx++;
                } else {
                    printf("SYNTAX ERROR: EXPECTED LITERAL AFTER WINDOWS\n");
                    syntaxError = true;
                }

                keyIdx++;

                break;

            case LITERAL:
                printf("SYNTAX ERROR: UNEXPECTED LITERAL\n");
                syntaxError = true;
                break;

            case CHAR:
                printf("SYNTAX ERROR: UNEXPECTED CHAR\n");
                syntaxError = true;
                break;

            case REM:
                i++;
                if(i >= numTokens) {
                    printf("SYNTAX ERROR: UNEXPECTED EOF. EXPECTED COMMENT AFTER REM\n");
                    syntaxError = true;
                    break;
                }

                if(token_types[i] == COMMENT) {
                    char *token = tokens[i];
                    printf("Comment: %s\n", token);
                } else {
                    printf("SYNTAX ERROR: EXPECTED COMMENT AFTER REM\n");
                    syntaxError = true;
                }

                break;

            // TODO: Add delay when running script somehow!
            case DELAY:
                i++;
                if(i >= numTokens) {
                    printf("SYNTAX ERROR: UNEXPECTED EOF. EXPECTED INT AFTER DELAY\n");
                    syntaxError = true;
                    break;
                }

                if(token_types[i] == INT) {
                    char *token = tokens[i];
                    int delay = atoi(token);
                    printf("Delay: %d\n", delay);
                } else {
                    printf("SYNTAX ERROR: EXPECTED INT AFTER DELAY\n");
                    syntaxError = true;
                }

                break;

            case ENTER:
                printf("Keycode added: ENTER\n");
                keyIdx++;
                break;

            default:
                printf("SYNTAX ERROR: UNEXPECTED TOKEN TYPE IN TOKEN %s\n", tokens[i]);
                syntaxError = true;
                break;
        }

        i++;
    }
    return !syntaxError;
}

/*
Convert string containing a ducky script to 
array of HID keycodes.
*/
bool scriptToKeyPresses(char *script, uint8_t **keycodes) {
    size_t len = strlen(script);
    *keycodes = malloc(sizeof(uint8_t) * len);
    if (*keycodes == NULL) {
        return false;
    }

    // Tokenize the script
    printf("\nTokenizing!\n");
    enum TOKEN *token_types = malloc(sizeof(enum TOKEN) * len);
    char **tokens = malloc(sizeof(char) * len);
    int numTokens = tokenizeScript(script, tokens, token_types);
    
    // Parse tokens to generate keycodes
    bool parseSuccess = parseTokens(tokens, token_types, numTokens);


    return parseSuccess;
}

int main() {
    printf("%s", example_script);

    uint8_t *keycodes = NULL;
    if (scriptToKeyPresses(example_script, &keycodes)) {
        for (size_t i = 0; i < strlen(example_script); i++) {
            printf("%d ", keycodes[i]);
        }
        printf("\n");
        free(keycodes);
    } else {
        printf("Error parsing script\n");
    }

    return 0;
}