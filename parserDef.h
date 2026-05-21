#ifndef PARSERDEF_H
#define PARSERDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerDef.h"

/* ============================
   CONSTANTS
============================ */

#define MAX_RULES 200
#define MAX_RHS 20
#define MAX_SYMBOL_LENGTH 50
#define MAX_CHILDREN 20

/* additional constants for parser implementation */
#define MAX_NONTERMINALS 100
#define MAX_TERMINALS 100
#define STACK_SIZE 1000


/* ============================
   SYMBOL TYPE
============================ */

typedef enum {
    TERMINAL,
    NON_TERMINAL
} SymbolType;

/* compatibility macros used in parser.c */
#define SYMBOL_TERMINAL TERMINAL
#define SYMBOL_NON_TERMINAL NON_TERMINAL

/* ============================
   NON-TERMINALS ENUM
   (Modify according to your grammar)
============================ */

typedef enum {

    NT_PROGRAM,
    NT_MAINFUNCTION,
    NT_OTHERFUNCTIONS,
    NT_FUNCTION,
    NT_INPUT_PAR,
    NT_OUTPUT_PAR,
    NT_PARAMETER_LIST,
    NT_DATA_TYPE,
    NT_PRIMITIVE_DATATYPE,
    NT_CONSTRUCTED_DATATYPE,
    NT_REMAINING_LIST,
    NT_STMTS,
    NT_TYPEDEFINITIONS,
    NT_TYPEDEFINITION,
    NT_DECLARATIONS,
    NT_DECLARATION,
    NT_GLOBAL_OR_NOT,
    NT_OTHERSTMTS,
    NT_STMT,
    NT_ASSIGNMENTSTMT,
    NT_SINGLEORRECID,
    NT_FUNCCALLSTMT,
    NT_OUTPUTPARAMETERS,
    NT_INPUTPARAMETERS,
    NT_ITERATIVESTMT,
    NT_CONDITIONALSTMT,
    NT_ELSEPART,
    NT_IOSTMT,
    NT_ARITHMETICEXPR,
    NT_ARITHMETICEXPR_PRIME,
    NT_TERM,
    NT_TERM_PRIME,
    NT_FACTOR,
    NT_BOOLEANEXPR,
    NT_BOOLEANEXPR_PRIME,
    NT_LOGICALOP,
    NT_RELATIONALOP,
    NT_RETURNSTMT,
    NT_OPTIONALRETURN,
    NT_IDLIST,
    NT_MOREIDS,
    NT_DEFINETYPESTMT,
    NT_FIELDDEFINITIONS,
    NT_FIELDDEFINITION,
    NT_MOREFIELDS,
    NT_RECTAIL,
    NT_VAR,
    NT_BOOLTERM,
    NT_RELEXPR,
    NT_RECORD_OR_UNION

} NonTerminal;


/* ============================
   GRAMMAR SYMBOL
============================ */

typedef struct {
    SymbolType type;
    int symbol;   // token enum (if terminal) or NonTerminal enum
} GrammarSymbol;


/* ============================
   PRODUCTION RULE
============================ */

typedef struct {
    NonTerminal lhs;
    int rhs_count;
    GrammarSymbol rhs[MAX_RHS];
} Production;


/* ============================
   FIRST AND FOLLOW SET
============================ */

typedef struct {
    int first[100][100];     // Adjust sizes if needed
    int follow[100][100];
} FirstAndFollow;


/* ============================
   PARSE TABLE
============================ */

typedef struct {
    int table[100][100];     // stores production number
} ParseTable;


/* ============================
   PARSE TREE NODE
============================ */

typedef struct parseTreeNode {

    char lexeme[MAX_LEXEME_LEN];
    tokenType token;
    int lineNo;

    union {
        int intVal;
        float realVal;
    } value;

    int isLeaf;

    char symbolName[MAX_SYMBOL_LENGTH];

    struct parseTreeNode *parent;
    struct parseTreeNode *child;      // leftmost child
    struct parseTreeNode *sibling;    // next sibling

} ParseTreeNode;


/* ============================
   PARSE TREE STRUCTURE
============================ */

typedef struct {
    ParseTreeNode *root;
} ParseTree;


/* ====== Debugging helper names ====== */

/* arrays defined in parser.c for mapping enums to strings */
extern const char *tokenNames[];
extern const char *nonTerminalNames[];

#endif