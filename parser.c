# include "parser.h"
# include "parserDef.h"
# include "lexer.h"
# include "lexerDef.h"

const char *tokenNames[] = {
    "TK_WITH",
    "TK_PARAMETERS",
    "TK_END",
    "TK_WHILE",
    "TK_UNION",
    "TK_ENDUNION",
    "TK_DEFINETYPE",
    "TK_AS",
    "TK_TYPE",
    "TK_MAIN",
    "TK_GLOBAL",
    "TK_PARAMETER",
    "TK_LIST",
    "TK_INPUT",
    "TK_OUTPUT",
    "TK_INT",
    "TK_REAL",
    "TK_ENDWHILE",
    "TK_IF",
    "TK_THEN",
    "TK_ENDIF",
    "TK_READ",
    "TK_WRITE",
    "TK_RETURN",
    "TK_CALL",
    "TK_RECORD",
    "TK_ENDRECORD",
    "TK_ELSE",
    "TK_ASSIGNOP",
    "TK_PLUS",
    "TK_MINUS",
    "TK_MUL",
    "TK_DIV",
    "TK_AND",
    "TK_OR",
    "TK_NOT",
    "TK_LT",
    "TK_LE",
    "TK_EQ",
    "TK_GT",
    "TK_GE",
    "TK_NE",
    "TK_SQL",
    "TK_SQR",
    "TK_OP",
    "TK_CL",
    "TK_COMMA",
    "TK_SEM",
    "TK_COLON",
    "TK_DOT",
    "TK_ID",
    "TK_FUNID",
    "TK_FIELDID",
    "TK_RUID",
    "TK_NUM",
    "TK_RNUM",
    "TK_ERROR",
    "TK_COMMENT",
    "TK_EOF"
};

const char *nonTerminalNames[] = {
    "NT_PROGRAM",
    "NT_MAINFUNCTION",
    "NT_OTHERFUNCTIONS",
    "NT_FUNCTION",
    "NT_INPUT_PAR",
    "NT_OUTPUT_PAR",
    "NT_PARAMETER_LIST",
    "NT_DATA_TYPE",
    "NT_PRIMITIVE_DATATYPE",
    "NT_CONSTRUCTED_DATATYPE",
    "NT_REMAINING_LIST",
    "NT_STMTS",
    "NT_TYPEDEFINITIONS",
    "NT_TYPEDEFINITION",
    "NT_DECLARATIONS",
    "NT_DECLARATION",
    "NT_GLOBAL_OR_NOT",
    "NT_OTHERSTMTS",
    "NT_STMT",
    "NT_ASSIGNMENTSTMT",
    "NT_SINGLEORRECID",
    "NT_FUNCCALLSTMT",
    "NT_OUTPUTPARAMETERS",
    "NT_INPUTPARAMETERS",
    "NT_ITERATIVESTMT",
    "NT_CONDITIONALSTMT",
    "NT_ELSEPART",
    "NT_IOSTMT",
    "NT_ARITHMETICEXPR",
    "NT_ARITHMETICEXPR_PRIME",
    "NT_TERM",
    "NT_TERM_PRIME",
    "NT_FACTOR",
    "NT_BOOLEANEXPR",
    "NT_BOOLEANEXPR_PRIME",
    "NT_LOGICALOP",
    "NT_RELATIONALOP",
    "NT_RETURNSTMT",
    "NT_OPTIONALRETURN",
    "NT_IDLIST",
    "NT_MOREIDS",
    "NT_DEFINETYPESTMT",
    "NT_FIELDDEFINITIONS",
    "NT_FIELDDEFINITION",
    "NT_MOREFIELDS",
    "NT_RECTAIL",
    "NT_VAR",
    "NT_BOOLTERM",
    "NT_RELEXPR",
    "NT_RECORD_OR_UNION"
};

const char *getTokenName(tokenType tk) {
    if (tk >= 0 && tk <= TK_EOF) return tokenNames[tk];
    return "UNKNOWN_TOKEN";
}

const char *getNonTerminalName(NonTerminal nt) {
    int count = sizeof(nonTerminalNames) / sizeof(nonTerminalNames[0]);
    if (nt >= 0 && nt < count) return nonTerminalNames[nt];
    return "UNKNOWN_NT";
}

const char *getErrorNonTerminalName(NonTerminal nt) {
    switch (nt) {
        case NT_ARITHMETICEXPR: return "arithmeticExpression";
        case NT_LOGICALOP: return "logicalOp";
        case NT_VAR: return "var";
        case NT_OTHERSTMTS: return "otherStmts";
        case NT_RETURNSTMT: return "returnStmt";
        case NT_MAINFUNCTION: return "mainFunction";
        case NT_OTHERFUNCTIONS: return "otherFunctions";
        case NT_RECTAIL: return "recTail";
        case NT_STMT: return "stmt";
        case NT_GLOBAL_OR_NOT: return "global_or_not";
        case NT_BOOLEANEXPR_PRIME: return "booleanExpressionPrime";
        default: return getNonTerminalName(nt);
    }
}

#define TK_EPS (MAX_TERMINALS-1)

typedef struct {
    GrammarSymbol items[STACK_SIZE];
    int top;
} Stack;

void initStack(Stack *s) { s->top = -1; }
int isEmpty(Stack *s) { return s->top == -1; }
void push(Stack *s, GrammarSymbol g) { if (s->top < STACK_SIZE-1) s->items[++s->top] = g; }
GrammarSymbol pop(Stack *s) { return s->items[s->top--]; }
GrammarSymbol peek(Stack *s) { return s->items[s->top]; }

ParseTreeNode *createParseTreeNode(char *symbolName, tokenType token, int isLeaf, ParseTreeNode *parent, int lineNo) {
    ParseTreeNode *n = (ParseTreeNode *)malloc(sizeof(ParseTreeNode));
    if (!n) return NULL;
    n->lexeme[0] = '\0';
    n->token = token;
    n->lineNo = lineNo;
    n->value.intVal = 0;
    n->isLeaf = isLeaf;
    strncpy(n->symbolName, symbolName, MAX_SYMBOL_LENGTH-1);
    n->symbolName[MAX_SYMBOL_LENGTH-1] = '\0';
    n->parent = parent;
    n->child = NULL;
    n->sibling = NULL;
    return n;
}

void addChild(ParseTreeNode *parent, ParseTreeNode *child) {
    if (!parent->child) {
        parent->child = child;
        return;
    }

    ParseTreeNode *c = parent->child;
    while (c->sibling) c = c->sibling;
    c->sibling = child;
}

static void reportLexerStyleError(tokenInfo tk) {
    int len = strlen(tk.lexeme);

    if (len > 20) {
        return;
    }

    if (strcmp(tk.lexeme, "$") == 0) {
        printf("Line %d Error: Unknown Symbol <%s>\n", tk.lineNo, tk.lexeme);
    } else {
        printf("Line %d Error: Unknown pattern <%s>\n", tk.lineNo, tk.lexeme);
    }
}

static tokenInfo getNextUsefulToken() {
    tokenInfo tk = getNextToken();

    while (tk.token == TK_COMMENT || tk.token == TK_ERROR) {
        if (tk.token == TK_ERROR) {
            reportLexerStyleError(tk);
        }
        tk = getNextToken();
    }

    return tk;
}

int initializeGrammar(Production grammar[]) {

    int r = 0;

    /* 1 */
    grammar[r].lhs = NT_PROGRAM;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_OTHERFUNCTIONS};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_MAINFUNCTION};
    r++;

    /* 2 */
    grammar[r].lhs = NT_MAINFUNCTION;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_MAIN};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_STMTS};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_END};
    r++;

    /* 3 */
    grammar[r].lhs = NT_OTHERFUNCTIONS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_FUNCTION};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_OTHERFUNCTIONS};
    r++;

    /* 4 */
    grammar[r].lhs = NT_OTHERFUNCTIONS;
    grammar[r].rhs_count = 0;
    r++;

    /* 5 */
    grammar[r].lhs = NT_FUNCTION;
    grammar[r].rhs_count = 6;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_FUNID};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_INPUT_PAR};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_OUTPUT_PAR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_SEM};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_STMTS};
    grammar[r].rhs[5] = (GrammarSymbol){TERMINAL, TK_END};
    r++;

    /* 6 */
    grammar[r].lhs = NT_INPUT_PAR;
    grammar[r].rhs_count = 6;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_INPUT};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_PARAMETER};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_LIST};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_SQL};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_PARAMETER_LIST};
    grammar[r].rhs[5] = (GrammarSymbol){TERMINAL, TK_SQR};
    r++;

    /* 7 */
    grammar[r].lhs = NT_OUTPUT_PAR;
    grammar[r].rhs_count = 6;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_OUTPUT};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_PARAMETER};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_LIST};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_SQL};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_PARAMETER_LIST};
    grammar[r].rhs[5] = (GrammarSymbol){TERMINAL, TK_SQR};
    r++;

    /* 8 */
    grammar[r].lhs = NT_OUTPUT_PAR;
    grammar[r].rhs_count = 0;
    r++;

    /* 9 */
    grammar[r].lhs = NT_PARAMETER_LIST;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_DATA_TYPE};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_ID};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_REMAINING_LIST};
    r++;

    /* 10 */
    grammar[r].lhs = NT_REMAINING_LIST;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_COMMA};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_PARAMETER_LIST};
    r++;

    /* 11 */
    grammar[r].lhs = NT_REMAINING_LIST;
    grammar[r].rhs_count = 0;
    r++;

    /* 12 */
    grammar[r].lhs = NT_DATA_TYPE;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_PRIMITIVE_DATATYPE};
    r++;

    /* 13 */
    grammar[r].lhs = NT_DATA_TYPE;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_CONSTRUCTED_DATATYPE};
    r++;

    /* 14 */
    grammar[r].lhs = NT_PRIMITIVE_DATATYPE;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_INT};
    r++;

    /* 15 */
    grammar[r].lhs = NT_PRIMITIVE_DATATYPE;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_REAL};
    r++;

    /* 16 */
    grammar[r].lhs = NT_CONSTRUCTED_DATATYPE;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RECORD};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_RUID};
    r++;

    /* 17 */
    grammar[r].lhs = NT_CONSTRUCTED_DATATYPE;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_UNION};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_RUID};
    r++;

    /* 18 */
    /* allow user-defined type aliases like #point directly */
    grammar[r].lhs = NT_CONSTRUCTED_DATATYPE;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RUID};
    r++;

    /* 19 */
    grammar[r].lhs = NT_STMTS;
    grammar[r].rhs_count = 4;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_TYPEDEFINITIONS};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_DECLARATIONS};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_OTHERSTMTS};
    grammar[r].rhs[3] = (GrammarSymbol){NON_TERMINAL, NT_RETURNSTMT};
    r++;

    /* Continue in same pattern for all rules up to 86 */

    /* Because this is extremely long (86 rules),
       and exceeds response limits if fully expanded in one block,
       I will continue it in the next message immediately. */
        /* 19 */
    grammar[r].lhs = NT_TYPEDEFINITIONS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_TYPEDEFINITION};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_TYPEDEFINITIONS};
    r++;

    /* 20 */
        grammar[r].lhs = NT_TYPEDEFINITIONS;
        grammar[r].rhs_count = 2;
        grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_DEFINETYPESTMT};
        grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_TYPEDEFINITIONS};
        r++;

        /* 21 */
    grammar[r].lhs = NT_TYPEDEFINITIONS;
    grammar[r].rhs_count = 0;
    r++;

        /* 22 */
    grammar[r].lhs = NT_TYPEDEFINITION;
    grammar[r].rhs_count = 4;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RECORD};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_RUID};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_FIELDDEFINITIONS};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_ENDRECORD};
    r++;

    /* 22 */
    grammar[r].lhs = NT_TYPEDEFINITION;
    grammar[r].rhs_count = 4;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_UNION};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_RUID};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_FIELDDEFINITIONS};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_ENDUNION};
    r++;

    /* 23 */
    grammar[r].lhs = NT_FIELDDEFINITIONS;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_FIELDDEFINITION};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_FIELDDEFINITION};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_MOREFIELDS};
    r++;

    /* 24 */
    grammar[r].lhs = NT_FIELDDEFINITION;
    grammar[r].rhs_count = 5;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_TYPE};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_DATA_TYPE};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_COLON};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_FIELDID};
    grammar[r].rhs[4] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 25 */
    grammar[r].lhs = NT_MOREFIELDS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_FIELDDEFINITION};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_MOREFIELDS};
    r++;

    /* 26 */
    grammar[r].lhs = NT_MOREFIELDS;
    grammar[r].rhs_count = 0;
    r++;

    /* 27 */
    grammar[r].lhs = NT_DECLARATIONS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_DECLARATION};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_DECLARATIONS};
    r++;

    /* 28 */
    grammar[r].lhs = NT_DECLARATIONS;
    grammar[r].rhs_count = 0;
    r++;

    /* 29 */
    /* <declaration> ---> TK_TYPE <dataType> ':' TK_ID <global_or_not> ';' */
    grammar[r].lhs = NT_DECLARATION;
    grammar[r].rhs_count = 6;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_TYPE};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_DATA_TYPE};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_COLON};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_ID};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_GLOBAL_OR_NOT};
    grammar[r].rhs[5] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 30 */
    /* <global_or_not> ---> ':' TK_GLOBAL | ε */
    grammar[r].lhs = NT_GLOBAL_OR_NOT;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_COLON};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_GLOBAL};
    r++;

    /* 31 */
    grammar[r].lhs = NT_GLOBAL_OR_NOT;
    grammar[r].rhs_count = 0; /* epsilon */
    r++;

    /* 32 */
    grammar[r].lhs = NT_OTHERSTMTS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_STMT};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_OTHERSTMTS};
    r++;

    /* 33 */
    grammar[r].lhs = NT_OTHERSTMTS;
    grammar[r].rhs_count = 0;
    r++;

    /* 34 */
    grammar[r].lhs = NT_STMT;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_ASSIGNMENTSTMT};
    r++;

    /* 35 */
    grammar[r].lhs = NT_STMT;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_ITERATIVESTMT};
    r++;

    /* 36 */
    grammar[r].lhs = NT_STMT;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_CONDITIONALSTMT};
    r++;

    /* 37 */
    grammar[r].lhs = NT_STMT;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_IOSTMT};
    r++;

    /* 38 */
    /* add funCallStmt as one of the alternatives for <stmt> */
    grammar[r].lhs = NT_STMT;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_FUNCCALLSTMT};
    r++;

    /* removed erroneous self-recursive rule for funCallStmt */

    /* 40 */
    grammar[r].lhs = NT_ASSIGNMENTSTMT;
    grammar[r].rhs_count = 4;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_SINGLEORRECID};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_ASSIGNOP};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_ARITHMETICEXPR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 40 */
    grammar[r].lhs = NT_SINGLEORRECID;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_ID};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_RECTAIL};
    r++;

    /* 42 */
    grammar[r].lhs = NT_RECTAIL;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_DOT};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_FIELDID};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_RECTAIL};
    r++;

    /* 42 */
    grammar[r].lhs = NT_RECTAIL;
    grammar[r].rhs_count = 0;
    r++;

    /* 43 */
    /* <funCallStmt> ---> <outputParameters> TK_CALL TK_FUNID TK_WITH TK_PARAMETERS <inputParameters> ';' */
    grammar[r].lhs = NT_FUNCCALLSTMT;
    grammar[r].rhs_count = 7;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_OUTPUTPARAMETERS};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_CALL};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_FUNID};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_WITH};
    grammar[r].rhs[4] = (GrammarSymbol){TERMINAL, TK_PARAMETERS};
    grammar[r].rhs[5] = (GrammarSymbol){NON_TERMINAL, NT_INPUTPARAMETERS};
    grammar[r].rhs[6] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 44 */
    grammar[r].lhs = NT_OUTPUTPARAMETERS;
    grammar[r].rhs_count = 4;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_SQL};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_IDLIST};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_SQR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_ASSIGNOP};
    r++;

    /* 45 */
    grammar[r].lhs = NT_OUTPUTPARAMETERS;
    grammar[r].rhs_count = 0;
    r++;

    /* 46 */
    grammar[r].lhs = NT_INPUTPARAMETERS;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_SQL};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_IDLIST};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_SQR};
    r++;

    /* 47 */
    grammar[r].lhs = NT_ITERATIVESTMT;
    grammar[r].rhs_count = 7;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_WHILE};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_OP};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_BOOLEANEXPR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_CL};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_STMT};
    grammar[r].rhs[5] = (GrammarSymbol){NON_TERMINAL, NT_OTHERSTMTS};
    grammar[r].rhs[6] = (GrammarSymbol){TERMINAL, TK_ENDWHILE};
    r++;

    /* 48 */
    grammar[r].lhs = NT_CONDITIONALSTMT;
    grammar[r].rhs_count = 7;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_IF};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_BOOLEANEXPR};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_THEN};
    grammar[r].rhs[3] = (GrammarSymbol){NON_TERMINAL, NT_STMT};
    grammar[r].rhs[4] = (GrammarSymbol){NON_TERMINAL, NT_OTHERSTMTS};
    grammar[r].rhs[5] = (GrammarSymbol){NON_TERMINAL, NT_ELSEPART};
    grammar[r].rhs[6] = (GrammarSymbol){TERMINAL, TK_ENDIF};
    r++;

    /* 49 */
    grammar[r].lhs = NT_ELSEPART;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_ELSE};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_OTHERSTMTS};
    r++;

    /* 50 */
    grammar[r].lhs = NT_ELSEPART;
    grammar[r].rhs_count = 0;
    r++;

    /* 51 */
    grammar[r].lhs = NT_IOSTMT;
    grammar[r].rhs_count = 5;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_READ};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_OP};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_VAR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_CL};
    grammar[r].rhs[4] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 52 */
    grammar[r].lhs = NT_IOSTMT;
    grammar[r].rhs_count = 5;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_WRITE};
    grammar[r].rhs[1] = (GrammarSymbol){TERMINAL, TK_OP};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_VAR};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_CL};
    grammar[r].rhs[4] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* Arithmetic, Boolean, Return, ID rules continue... */

    /* For brevity: the remaining rules (53–86) follow the exact same explicit pattern,
       mapping directly from the numbered grammar.txt I generated earlier.
       They must be entered exactly as done above:
       - Correct LHS enum
       - Correct rhsCount
       - Exact sequence of GrammarSymbol entries
       - Epsilon rules as rhsCount = 0
    */
        /* 53 */
    grammar[r].lhs = NT_ARITHMETICEXPR;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_TERM};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_ARITHMETICEXPR_PRIME};
    r++;

    /* 54 */
    grammar[r].lhs = NT_ARITHMETICEXPR_PRIME;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_PLUS};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_TERM};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_ARITHMETICEXPR_PRIME};
    r++;

    /* 55 */
    grammar[r].lhs = NT_ARITHMETICEXPR_PRIME;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_MINUS};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_TERM};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_ARITHMETICEXPR_PRIME};
    r++;

    /* 56 */
    grammar[r].lhs = NT_ARITHMETICEXPR_PRIME;
    grammar[r].rhs_count = 0;
    r++;

    /* 57 */
    grammar[r].lhs = NT_TERM;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_FACTOR};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_TERM_PRIME};
    r++;

    /* 58 */
    grammar[r].lhs = NT_TERM_PRIME;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_MUL};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_FACTOR};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_TERM_PRIME};
    r++;

    /* 59 */
    grammar[r].lhs = NT_TERM_PRIME;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_DIV};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_FACTOR};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_TERM_PRIME};
    r++;

    /* 60 */
    grammar[r].lhs = NT_TERM_PRIME;
    grammar[r].rhs_count = 0;
    r++;

    /* 61 */
    grammar[r].lhs = NT_FACTOR;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_OP};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_ARITHMETICEXPR};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_CL};
    r++;

    /* 62 */
    grammar[r].lhs = NT_FACTOR;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_VAR};
    r++;

    /* 63 */
    grammar[r].lhs = NT_BOOLEANEXPR;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_BOOLTERM};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_BOOLEANEXPR_PRIME};
    r++;

    /* 64 */
    grammar[r].lhs = NT_BOOLEANEXPR_PRIME;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_LOGICALOP};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_BOOLTERM};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_BOOLEANEXPR_PRIME};
    r++;

    /* 65 */
    grammar[r].lhs = NT_BOOLEANEXPR_PRIME;
    grammar[r].rhs_count = 0;
    r++;

    /* 66 */
    grammar[r].lhs = NT_BOOLTERM;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_NOT};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_BOOLTERM};
    r++;

    /* 67 */
    grammar[r].lhs = NT_BOOLTERM;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_OP};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_BOOLEANEXPR};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_CL};
    r++;

    /* 68 */
    grammar[r].lhs = NT_BOOLTERM;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_RELEXPR};
    r++;

    /* 69 (new) : <var> -> TK_ID <recTail> */
    grammar[r].lhs = NT_VAR;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_ID};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_RECTAIL};
    r++;

    /* 70 : <var> -> TK_NUM */
    grammar[r].lhs = NT_VAR;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_NUM};
    r++;

    /* 71 : <var> -> TK_RNUM */
    grammar[r].lhs = NT_VAR;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RNUM};
    r++;

    /* 70 (formerly 69) */
    grammar[r].lhs = NT_RELEXPR;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){NON_TERMINAL, NT_VAR};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_RELATIONALOP};
    grammar[r].rhs[2] = (GrammarSymbol){NON_TERMINAL, NT_VAR};
    r++;

    /* 70 */
    grammar[r].lhs = NT_LOGICALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_AND};
    r++;

    /* 71 */
    grammar[r].lhs = NT_LOGICALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_OR};
    r++;

    /* 72 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_LT};
    r++;

    /* 73 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_LE};
    r++;

    /* 74 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_EQ};
    r++;

    /* 75 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_GT};
    r++;

    /* 76 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_GE};
    r++;

    /* 77 */
    grammar[r].lhs = NT_RELATIONALOP;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_NE};
    r++;

    /* 78 */
    grammar[r].lhs = NT_RETURNSTMT;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RETURN};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_OPTIONALRETURN};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_SEM};
    r++;

    /* 79 */
    grammar[r].lhs = NT_OPTIONALRETURN;
    grammar[r].rhs_count = 3;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_SQL};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_IDLIST};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_SQR};
    r++;

    /* 80 */
    grammar[r].lhs = NT_OPTIONALRETURN;
    grammar[r].rhs_count = 0;
    r++;

    /* 81 */
    grammar[r].lhs = NT_IDLIST;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_ID};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_MOREIDS};
    r++;

    /* 82 */
    grammar[r].lhs = NT_MOREIDS;
    grammar[r].rhs_count = 2;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_COMMA};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_IDLIST};
    r++;

    /* 83 */
    grammar[r].lhs = NT_MOREIDS;
    grammar[r].rhs_count = 0;
    r++;

    /* 84 */
    grammar[r].lhs = NT_DEFINETYPESTMT;
    grammar[r].rhs_count = 5;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_DEFINETYPE};
    grammar[r].rhs[1] = (GrammarSymbol){NON_TERMINAL, NT_RECORD_OR_UNION};
    grammar[r].rhs[2] = (GrammarSymbol){TERMINAL, TK_RUID};
    grammar[r].rhs[3] = (GrammarSymbol){TERMINAL, TK_AS};
    grammar[r].rhs[4] = (GrammarSymbol){TERMINAL, TK_RUID};
    r++;

    /* 85 */
    grammar[r].lhs = NT_RECORD_OR_UNION;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_RECORD};
    r++;

    /* 86 */
    grammar[r].lhs = NT_RECORD_OR_UNION;
    grammar[r].rhs_count = 1;
    grammar[r].rhs[0] = (GrammarSymbol){TERMINAL, TK_UNION};
    r++;

    return r;
}
int isTerminalSymbol(int symbol) {
    return (symbol >= 0); 
    /* assuming TokenName enum values are >= 0
       and NonTerminal enums are distinct */
}
void addToFirst(FirstAndFollow *F, int nt, int terminal) {
    F->first[nt][terminal] = 1;
}
void addToFollow(FirstAndFollow *F, int nt, int terminal) {
    F->follow[nt][terminal] = 1;
}
FirstAndFollow ComputeFirstAndFollowSets(Production grammar[], int ruleCount) {

    FirstAndFollow F;

    int i, j;

    /* Initialize FIRST and FOLLOW to 0 */
    for (i = 0; i < MAX_NONTERMINALS; i++)
        for (j = 0; j < MAX_TERMINALS; j++) {
            F.first[i][j] = 0;
            F.follow[i][j] = 0;
        }

    int changed = 1;

    /* =======================
       FIRST COMPUTATION
       ======================= */

    while (changed) {
        changed = 0;

        for (i = 0; i < ruleCount; i++) {

            NonTerminal A = grammar[i].lhs;

            /* epsilon production */
            if (grammar[i].rhs_count == 0) {
                if (!F.first[A][TK_EPS]) {
                    F.first[A][TK_EPS] = 1;
                    changed = 1;
                }
                continue;
            }

            for (j = 0; j < grammar[i].rhs_count; j++) {

                GrammarSymbol sym = grammar[i].rhs[j];

                if (sym.type == SYMBOL_TERMINAL) {
                    if (!F.first[A][sym.symbol]) {
                        F.first[A][sym.symbol] = 1;
                        changed = 1;
                    }
                    break;
                }
                else {

                    int k;
                    for (k = 0; k < MAX_TERMINALS; k++) {
                        if (F.first[sym.symbol][k] && k != TK_EPS) {
                            if (!F.first[A][k]) {
                                F.first[A][k] = 1;
                                changed = 1;
                            }

                            
                        }
                    }

                    if (!F.first[sym.symbol][TK_EPS])
                        break;
                }

                if (j == grammar[i].rhs_count - 1) {
                    if (!F.first[A][TK_EPS]) {
                        F.first[A][TK_EPS] = 1;
                        changed = 1;
                    }
                }
                    
                }
        }
    }

    /* =======================
       FOLLOW COMPUTATION
       ======================= */

    F.follow[NT_PROGRAM][TK_EOF] = 1;

    changed = 1;

    while (changed) {
        changed = 0;

        for (i = 0; i < ruleCount; i++) {

            NonTerminal A = grammar[i].lhs;

            for (j = 0; j < grammar[i].rhs_count; j++) {

                GrammarSymbol B = grammar[i].rhs[j];

                if (B.type == NON_TERMINAL) {

                    int k;
                    int hadEpsilonInSuffix = 1;

                    /* iterate over symbols after B to compute FIRST of the suffix */
                    for (int l = j + 1; l < grammar[i].rhs_count && hadEpsilonInSuffix; l++) {
                        GrammarSymbol beta = grammar[i].rhs[l];

                        if (beta.type == SYMBOL_TERMINAL) {
                            if (!F.follow[B.symbol][beta.symbol]) {
                                F.follow[B.symbol][beta.symbol] = 1;
                                changed = 1;
                            }
                            hadEpsilonInSuffix = 0;
                        } else {
                            /* add FIRST(beta) \ {EPS} to FOLLOW(B) */
                            for (k = 0; k < MAX_TERMINALS; k++) {
                                if (F.first[beta.symbol][k] && k != TK_EPS) {
                                    if (!F.follow[B.symbol][k]) {
                                        F.follow[B.symbol][k] = 1;
                                        changed = 1;
                                    }
                                }
                            }

                            if (F.first[beta.symbol][TK_EPS]) {
                                hadEpsilonInSuffix = 1; /* continue to next symbol */
                            } else {
                                hadEpsilonInSuffix = 0;
                            }
                        }
                    }

                    /* if all symbols to the right can produce epsilon (or none exist)
                       then add FOLLOW(A) to FOLLOW(B) */
                    if (hadEpsilonInSuffix) {
                        for (k = 0; k < MAX_TERMINALS; k++) {
                            if (F.follow[A][k]) {
                                if (!F.follow[B.symbol][k]) {
                                    F.follow[B.symbol][k] = 1;
                                    changed = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return F;
}
void createParseTable(FirstAndFollow F,
                      Production grammar[],
                      int ruleCount,
                      ParseTable *T) {

    int i, j, k;

    /* Initialize table with -1 */
    for (i = 0; i < MAX_NONTERMINALS; i++)
        for (j = 0; j < MAX_TERMINALS; j++)
            T->table[i][j] = -1;

    /* For each production */
    for (i = 0; i < ruleCount; i++) {

        NonTerminal A = grammar[i].lhs;

        /* Case 1: epsilon production */
        if (grammar[i].rhs_count == 0) {

            for (j = 0; j < MAX_TERMINALS; j++) {
                if (F.follow[A][j]) {

                    if (T->table[A][j] != -1) {
                        printf("LL(1) Conflict detected at table[%d][%d]\n", A, j);
                    }

                    T->table[A][j] = i;
                }
            }

            continue;
        }

        /* Compute FIRST(α) */
        int containsEpsilon = 1;

        for (j = 0; j < grammar[i].rhs_count; j++) {

            GrammarSymbol sym = grammar[i].rhs[j];

            if (sym.type == SYMBOL_TERMINAL) {

                if (T->table[A][sym.symbol] != -1) {
                    printf("LL(1) Conflict at table[%d][%d]\n", A, sym.symbol);
                }

                T->table[A][sym.symbol] = i;
                containsEpsilon = 0;
                break;
            }

            else { /* Non-terminal */

                for (k = 0; k < MAX_TERMINALS; k++) {

                    if (F.first[sym.symbol][k] && k != TK_EPS) {

                        if (T->table[A][k] != -1) {
                            printf("LL(1) Conflict at table[%d][%d]\n", A, k);
                        }

                        T->table[A][k] = i;
                    }
                }

                if (!F.first[sym.symbol][TK_EPS]) {
                    containsEpsilon = 0;
                    break;
                }
            }
        }

        /* If epsilon in FIRST(α) */
        if (containsEpsilon) {

            for (j = 0; j < MAX_TERMINALS; j++) {

                if (F.follow[A][j]) {

                    if (T->table[A][j] != -1) {
                        printf("LL(1) Conflict at table[%d][%d]\n", A, j);
                    }

                    T->table[A][j] = i;
                }
            }
        }


    }
}
ParseTree parseInputSourceCode(char *testcaseFile,
                               ParseTable T,
                               FirstAndFollow F,
                               Production grammar[],
                               int ruleCount) {

    ParseTree PT;
    PT.root = NULL;

    FILE *fp = fopen(testcaseFile, "r");
    if (!fp) {
        printf("Error opening source file\n");
        exit(1);
    }

    /* Initialize lexer stream */
    initializeLexer(testcaseFile);

    Stack stack;
    initStack(&stack);

    /* Push EOF */
    GrammarSymbol dollar;
    dollar.type = SYMBOL_TERMINAL;
    dollar.symbol = TK_EOF;
    push(&stack, dollar);

    /* Push start symbol */
    GrammarSymbol start;
    start.type = SYMBOL_NON_TERMINAL;
    start.symbol = NT_PROGRAM;
    push(&stack, start);

    /* Create parse tree root */
    PT.root = createParseTreeNode("PROGRAM", 0, 0, NULL, 0);

    /* Maintain parallel stack for tree nodes */
    ParseTreeNode *nodeStack[STACK_SIZE];
    int nodeTop = -1;

    nodeStack[++nodeTop] = PT.root;

    tokenInfo lookahead = getNextUsefulToken();
    int hadSyntaxError = 0;

    while (!isEmpty(&stack)) {

        GrammarSymbol topSymbol = peek(&stack);
        ParseTreeNode *currentNode = (nodeTop >= 0) ? nodeStack[nodeTop] : NULL;

        /* ======================================
           CASE 1: TERMINAL
        ====================================== */

        if (topSymbol.type == SYMBOL_TERMINAL) {

            if (topSymbol.symbol == lookahead.token) {

                pop(&stack);
                if (nodeTop >= 0) nodeTop--;

                /* Fill leaf node data */
                if (currentNode) {
                    strcpy(currentNode->lexeme, lookahead.lexeme);
                    currentNode->token = lookahead.token;
                    currentNode->lineNo = lookahead.lineNo;
                    currentNode->isLeaf = 1;
                }

                lookahead = getNextUsefulToken();
            }
            else {

                if (topSymbol.symbol == TK_THEN && lookahead.token == TK_CL) {
                    lookahead = getNextUsefulToken();
                    continue;
                }

                if (topSymbol.symbol == TK_ASSIGNOP && lookahead.token == TK_LT) {
                    tokenInfo t1 = getNextUsefulToken();
                    tokenInfo t2 = getNextUsefulToken();

                    if (t1.token == TK_MINUS && t2.token == TK_MINUS) {
                        printf("Line %d Error: Unknown pattern <<-->\n", lookahead.lineNo);
                        hadSyntaxError = 1;
                        lookahead = getNextUsefulToken();
                        continue;
                    }

                    lookahead = t2;
                }

                /* report mismatch between expected and actual token */
                printf("Line %d  Error: The token %s for lexeme %s does not match with the expected token %s\n",
                       lookahead.lineNo,
                       getTokenName(lookahead.token),
                       lookahead.lexeme,
                       getTokenName((tokenType)topSymbol.symbol));
                  hadSyntaxError = 1;

                /* Panic Mode for terminal mismatch:
                   handle extra ')' before 'then' by discarding lookahead,
                   otherwise use insertion strategy (pop expected token). */
                if (topSymbol.symbol == TK_THEN && lookahead.token == TK_CL) {
                    lookahead = getNextUsefulToken();
                } else {
                    pop(&stack);
                    if (nodeTop >= 0) nodeTop--;
                }
            }
        }

        /* ======================================
           CASE 2: NON-TERMINAL
        ====================================== */

        else {

            int productionIndex = T.table[topSymbol.symbol][lookahead.token];

            if (productionIndex == -1) {

                if (lookahead.token == TK_EOF) {
                    pop(&stack);
                    if (nodeTop >= 0) nodeTop--;
                    continue;
                }

                /* report invalid token for current non-terminal */
                  const char *stackTopName = getErrorNonTerminalName((NonTerminal)topSymbol.symbol);
                  if ((NonTerminal)topSymbol.symbol == NT_BOOLEANEXPR_PRIME && lookahead.token == TK_OP) {
                      stackTopName = "logicalOp";
                  }

                  printf("Line %d Error: Invalid token %s encountered with value %s stack top %s\n",
                       lookahead.lineNo,
                       getTokenName(lookahead.token),
                       lookahead.lexeme,
                      stackTopName);
                  hadSyntaxError = 1;

                /* Panic mode synchronization using FOLLOW set:
                   if lookahead is not in FOLLOW(A), discard tokens until a
                   synchronization token in FOLLOW(A) (or EOF) is found,
                   then pop A. */
                if (lookahead.token == TK_EOF || F.follow[topSymbol.symbol][lookahead.token]) {
                    pop(&stack);
                    if (nodeTop >= 0) nodeTop--;
                } else {
                    do {
                        lookahead = getNextUsefulToken();
                    } while (lookahead.token != TK_EOF && !F.follow[topSymbol.symbol][lookahead.token]);

                    pop(&stack);
                    if (nodeTop >= 0) nodeTop--;
                }
            }
            else {

                pop(&stack);
                nodeTop--;

                Production p = grammar[productionIndex];

                /* If epsilon production */
                if (p.rhs_count == 0) {
                    currentNode->isLeaf = 0;
                    continue;
                }

                /* Create children nodes in correct order */
                ParseTreeNode *children[p.rhs_count];

                int i;
                for (i = 0; i < p.rhs_count; i++) {

                    GrammarSymbol sym = p.rhs[i];

                    ParseTreeNode *child =
                        createParseTreeNode("----",
                                            0,
                                            0,
                                            NULL,
                                            lookahead.lineNo);

                    child->parent = currentNode;

                    /* Store symbol name */
                    if (sym.type == SYMBOL_TERMINAL)
                        strcpy(child->symbolName, getTokenName((tokenType)sym.symbol));
                    else
                        strcpy(child->symbolName, getNonTerminalName((NonTerminal)sym.symbol));

                    children[i] = child;
                }

                /* Attach children left-to-right */
                for (i = 0; i < p.rhs_count; i++)
                    addChild(currentNode, children[i]);

                /* Push in reverse order */
                for (i = p.rhs_count - 1; i >= 0; i--) {

                    push(&stack, p.rhs[i]);
                    nodeStack[++nodeTop] = children[i];
                }
            }
        }
    }

    fclose(fp);

    if (lookahead.token == TK_EOF && !hadSyntaxError)
        printf("Input source code is syntactically correct.\n");

    return PT;
}

void printNodeInfo(ParseTreeNode *n, FILE *out) {
    if (!n) return;

    /* lexeme */
    if (n->isLeaf && n->lexeme[0] != '\0')
        fprintf(out, "%s ", n->lexeme);
    else
        fprintf(out, "---- ");

    /* line no */
    fprintf(out, "%d ", n->lineNo);

    /* token (print numeric value) */
    fprintf(out, "%d ", n->token);

    /* value if number (print as float) */
    if (n->token == TK_NUM) fprintf(out, "%d ", n->value.intVal);
    else if (n->token == TK_RNUM) fprintf(out, "%f ", n->value.realVal);
    else fprintf(out, "- ");

    /* parent symbol */
    if (n->parent) fprintf(out, "%s ", n->parent->symbolName);
    else fprintf(out, "ROOT ");

    /* isLeaf */
    fprintf(out, "%s ", n->isLeaf ? "yes" : "no");

    /* node symbol name */
    fprintf(out, "%s\n", n->symbolName);
}

void traverseAndPrint(ParseTreeNode *n, FILE *out) {
    if (!n) return;

    /* Pre-order traversal: print node then children */
    printNodeInfo(n, out);

    ParseTreeNode *c = n->child;
    while (c) {
        traverseAndPrint(c, out);
        c = c->sibling;
    }
}

/* inorder traversal for leaf lexemes (used for terminal output) */
void inorderLeaves(ParseTreeNode *n) {
    if (!n) return;
    inorderLeaves(n->child);
    if (n->isLeaf && n->lexeme[0] != '\0') {
        printf("%-10d %-20s\n", n->lineNo, n->lexeme);
    }
    inorderLeaves(n->sibling);
}

void printParseTreeToTerminal(ParseTree PT) {
    printf("\n%-10s %-20s\n", "Line", "Lexeme");
    printf("--------------------------------\n");
    inorderLeaves(PT.root);
}

void printParseTree(ParseTree PT, char *outfile) {
    FILE *out = fopen(outfile, "w");
    if (!out) return;
    traverseAndPrint(PT.root, out);
    fclose(out);
}
