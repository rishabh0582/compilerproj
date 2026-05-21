#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"
#include "lexer.h"
#include "lexerDef.h"

/* ============================================================
   FIRST AND FOLLOW SET COMPUTATION
   ============================================================ */

/*
 * Computes FIRST and FOLLOW sets for the grammar G.
 * If FIRST/FOLLOW are manually populated, this function
 * initializes the structure appropriately.
 */
FirstAndFollow ComputeFirstAndFollowSets(Production grammar[], int ruleCount);


/* ============================================================
   PARSE TABLE CREATION
   ============================================================ */

/*
 * Creates LL(1) parse table using FIRST and FOLLOW sets.
 */
void createParseTable(FirstAndFollow F,
                      Production grammar[],
                      int ruleCount,
                      ParseTable *T);


/* ============================================================
   PARSING FUNCTION
   ============================================================ */

/*
 * Parses the source code using predictive parsing.
 * Builds and returns the parse tree.
 */
ParseTree parseInputSourceCode(char *testcaseFile,
                               ParseTable T,
                               FirstAndFollow F,
                               Production grammar[],
                               int ruleCount);


/* ============================================================
   PARSE TREE PRINTING
   ============================================================ */

/*
 * Prints the parse tree in inorder traversal
 * in the required format to output file.
 */
void printParseTree(ParseTree PT, char *outfile);

/* Initializes grammar rules from LL(1) compatible grammar */
int initializeGrammar(Production grammar[]);

/* helpers for debugging and printing */
const char *getTokenName(tokenType tk);
const char *getNonTerminalName(NonTerminal nt);
void printParseTreeToTerminal(ParseTree PT);

#endif