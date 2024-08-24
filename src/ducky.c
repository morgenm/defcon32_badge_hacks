#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ducky.h"
#include "hid_keycodes.h"
#include "tusb.h" // Temporary while fixing HID_SPACE

char example_script[] = "\
STRING cooltestechohello echo hello\n\
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

bool addKeycode(uint8_t *keycodes, uint8_t *mods, int *keyIdx, uint8_t keycode, uint8_t mod) {
    if(*keyIdx >= MAX_SCRIPT_KEYCODES) {
        return false;
    }
    
    keycodes[*keyIdx] = keycode;
    mods[*keyIdx] = mod;
    *keyIdx += 1;
    return true;
}

bool parseTokens(char **tokens, enum TOKEN *token_types, int numTokens, uint8_t *keycodes, uint8_t *mods, int *numKeyCodes) {
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
                    for(int j=0; j<strlen(token); j++) {
                        uint8_t keycode;
                        uint8_t mod;
                        asciiToKeyCode(token[j], &keycode, &mod);

                        if(token[j] == ' ') {
                            keycode = HID_KEY_SPACE;
                            mod = 0;
                        }

                        if(!addKeycode(keycodes, mods, &keyIdx, keycode, mod)) {
                            printf("ERROR: MAX SCRIPT KEYCODES EXCEEDED\n");
                            syntaxError = true;
                            break;
                        }
                    }
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
                    //keyIdx++;
                } else {
                    printf("SYNTAX ERROR: EXPECTED LITERAL AFTER WINDOWS\n");
                    syntaxError = true;
                }

                //keyIdx++;

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
                //keyIdx++;
                break;

            default:
                printf("SYNTAX ERROR: UNEXPECTED TOKEN TYPE IN TOKEN %s\n", tokens[i]);
                syntaxError = true;
                break;
        }

        i++;
    }
    *numKeyCodes = keyIdx;
    return !syntaxError;
}

/*
Convert string containing a ducky script to 
array of HID keycodes.
*/
int scriptToKeyPresses(char *script, uint8_t **keycodes, uint8_t **mods) {
    size_t len = strlen(script);

    // Tokenize the script
    printf("\nTokenizing!\n");
    enum TOKEN *token_types = malloc(sizeof(enum TOKEN) * len);
    char **tokens = malloc(sizeof(char) * len);
    int numTokens = tokenizeScript(script, tokens, token_types);
    
    // Parse tokens to generate keycodes
    int numKeyCodes = -1;
    bool parseSuccess = parseTokens(tokens, token_types, numTokens, *keycodes, *mods, &numKeyCodes);

    if(!parseSuccess) {
        return -1;
    }

    return numKeyCodes;
}