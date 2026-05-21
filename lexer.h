#ifndef LEXER_H
#define LEXER_H

#include "lexerDef.h"

void initializeLexer(char* filename);
tokenInfo getNextToken();
void removeComments(char* filename);


#endif