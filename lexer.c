
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "lexerDef.h"

int lineNo = 1;
FILE *sourceFile = NULL;

typedef struct
{
    char *lexeme;
    tokenType token;
} keyword;

keyword keywords[] = {
    {"with", TK_WITH},
    {"parameters", TK_PARAMETERS},
    {"end", TK_END},
    {"while", TK_WHILE},
    {"union", TK_UNION},
    {"endunion", TK_ENDUNION},
    {"definetype", TK_DEFINETYPE},
    {"as", TK_AS},
    {"type", TK_TYPE},
    {"_main", TK_MAIN},
    {"global", TK_GLOBAL},
    {"parameter", TK_PARAMETER},
    {"list", TK_LIST},
    {"input", TK_INPUT},
    {"output", TK_OUTPUT},
    {"int", TK_INT},
    {"real", TK_REAL},
    {"endwhile", TK_ENDWHILE},
    {"if", TK_IF},
    {"then", TK_THEN},
    {"endif", TK_ENDIF},
    {"read", TK_READ},
    {"write", TK_WRITE},
    {"return", TK_RETURN},
    {"call", TK_CALL},
    {"record", TK_RECORD},
    {"endrecord", TK_ENDRECORD},
    {"else", TK_ELSE}};

int totalKeywords = sizeof(keywords) / sizeof(keyword);

tokenType checkKeyword(char *str)
{
    for (int i = 0; i < totalKeywords; i++)
        if (strcmp(str, keywords[i].lexeme) == 0)
            return keywords[i].token;
    return TK_ERROR;
}

int isValidID(char *str)
{

    int len = strlen(str);

    if (len < 2)
        return 0;

    if (str[0] < 'b' || str[0] > 'd')
        return 0;

    if (str[1] < '2' || str[1] > '7')
        return 0;

    if (len > 20)
    {
        printf("Line No %d: Error : Variable Identifier is longer than the prescribed length of 20 characters.\n", lineNo);
        return 0;
    }

    int i = 2;

    while (i < len && str[i] >= 'b' && str[i] <= 'd')
        i++;

    while (i < len && str[i] >= '2' && str[i] <= '7')
        i++;

    return (i == len);
}

int isValidFUNID(char *str)
{

    int len = strlen(str);

    if (len < 2 || len > 30)
        return 0;

    if (str[0] != '_')
        return 0;

    int i = 1;

    if (!isalpha(str[i]))
        return 0;

    while (isalpha(str[i]))
        i++;
    while (isdigit(str[i]))
        i++;

    return (str[i] == '\0');
}

int isValidFIELDID(char *str)
{
    int i = 0;
    while (str[i] >= 'a' && str[i] <= 'z')
        i++;
    return (str[i] == '\0');
}

tokenInfo makeToken(tokenType tk, char *lexeme)
{

    tokenInfo t;
    t.token = tk;
    strcpy(t.lexeme, lexeme);
    t.lineNo = lineNo;

    if (tk == TK_NUM)
        t.value.intVal = atoi(lexeme);

    if (tk == TK_RNUM)
        t.value.realVal = atof(lexeme);

    return t;
}

void initializeLexer(char *filename)
{

    sourceFile = fopen(filename, "r");
    if (!sourceFile)
    {
        printf("Error opening source file\n");
        exit(1);
    }
    lineNo = 1;
}

void removeComments(char *filename)
{

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return;

    char ch;

    while ((ch = fgetc(fp)) != EOF)
    {

        if (ch == '%')
        {
            while (ch != '\n' && ch != EOF)
                ch = fgetc(fp);
        }

        if (ch == '\n')
            lineNo++;

        if (ch != EOF)
            putchar(ch);
    }

    fclose(fp);
}

tokenInfo getNextToken()
{

    char ch;

    while ((ch = fgetc(sourceFile)) != EOF)
    {

        if (ch == '\n')
        {
            lineNo++;
            continue;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r')
            continue;

        if (ch == '%')
        {

            while ((ch = fgetc(sourceFile)) != '\n' && ch != EOF)
                ;

            if (ch == '\n')
                ungetc(ch, sourceFile);

            return makeToken(TK_COMMENT, "%");
        }

        if (ch == '#')
        {

            char buffer[MAX_LEXEME_LEN];
            int i = 0;

            buffer[i++] = ch;

            ch = fgetc(sourceFile);

            if (!(ch >= 'a' && ch <= 'z'))
            {
                if (ch != EOF)
                    ungetc(ch, sourceFile);

                buffer[i] = '\0';
                return makeToken(TK_ERROR, buffer);
            }

            while (ch >= 'a' && ch <= 'z')
            {
                buffer[i++] = ch;
                ch = fgetc(sourceFile);
            }

            buffer[i] = '\0';

            if (ch != EOF)
                ungetc(ch, sourceFile);

            return makeToken(TK_RUID, buffer);
        }
        if (isalpha(ch) || ch == '_')
        {

            char buffer[MAX_LEXEME_LEN];
            int i = 0;

            buffer[i++] = ch;

            if (ch == '_')
            {

                while (isalpha(ch = fgetc(sourceFile)))
                    buffer[i++] = ch;

                while (isdigit(ch))
                {
                    buffer[i++] = ch;
                    ch = fgetc(sourceFile);
                }

                buffer[i] = '\0';
                ungetc(ch, sourceFile);

                tokenType kw = checkKeyword(buffer);
                if (kw != TK_ERROR)
                {
                    return makeToken(kw, buffer);
                }

                if (isValidFUNID(buffer))
                    return makeToken(TK_FUNID, buffer);

                return makeToken(TK_ERROR, buffer);
            }


            if (ch >= 'b' && ch <= 'd')
            {

                char next = fgetc(sourceFile);

                if (next >= '2' && next <= '7')
                {

                    buffer[i++] = next;

                    while ((ch = fgetc(sourceFile)) >= 'b' && ch <= 'd')
                        buffer[i++] = ch;

                    while (ch >= '2' && ch <= '7')
                    {
                        buffer[i++] = ch;
                        ch = fgetc(sourceFile);
                    }

                    buffer[i] = '\0';
                    ungetc(ch, sourceFile);

                    if (strlen(buffer) > 20)
                    {
                        printf("Line No %d: Error : Variable Identifier is longer than the prescribed length of 20 characters.\n", lineNo);
                        return makeToken(TK_ERROR, buffer);
                    }

                    return makeToken(TK_ID, buffer);
                }

                ungetc(next, sourceFile);
            }


            while ((ch = fgetc(sourceFile)) >= 'a' && ch <= 'z')
                buffer[i++] = ch;

            buffer[i] = '\0';
            ungetc(ch, sourceFile);

            tokenType kw = checkKeyword(buffer);
            if (kw != TK_ERROR)
                return makeToken(kw, buffer);

            return makeToken(TK_FIELDID, buffer);
        }

        if (isdigit(ch))
        {

            char buffer[MAX_LEXEME_LEN];
            int i = 0;

            buffer[i++] = ch;

            while (isdigit(ch = fgetc(sourceFile)))
                buffer[i++] = ch;

            if (ch == '.')
            {

                buffer[i++] = ch;

                int digitCount = 0;

                ch = fgetc(sourceFile);

                while (isdigit(ch))
                {
                    buffer[i++] = ch;
                    digitCount++;
                    ch = fgetc(sourceFile);
                }

                if (digitCount == 2)
                {

                    if (ch == 'E')
                    {

                        buffer[i++] = ch;

                        char next = fgetc(sourceFile);

                        char sign = '\0';

                        if (next == '+' || next == '-')
                        {
                            sign = next;
                            buffer[i++] = sign;
                            next = fgetc(sourceFile);
                        }

                        char d1 = next;
                        char d2 = fgetc(sourceFile);

                        if (isdigit(d1) && isdigit(d2))
                        {

                            buffer[i++] = d1;
                            buffer[i++] = d2;
                            buffer[i] = '\0';

                            return makeToken(TK_RNUM, buffer);
                        }

                        if (d2 != EOF)
                            ungetc(d2, sourceFile);
                        if (d1 != EOF)
                            ungetc(d1, sourceFile);
                        if (sign != '\0')
                            ungetc(sign, sourceFile);

                        buffer[i] = '\0';
                        return makeToken(TK_ERROR, buffer);
                    }
                    if (ch != EOF)
                        ungetc(ch, sourceFile);

                    buffer[i] = '\0';
                    return makeToken(TK_RNUM, buffer);
                }

                if (ch != EOF)
                    ungetc(ch, sourceFile);

                buffer[i] = '\0';
                return makeToken(TK_ERROR, buffer);
            }

            if (ch != EOF)
                ungetc(ch, sourceFile);

            buffer[i] = '\0';
            return makeToken(TK_NUM, buffer);
        }

        if (ch == '<')
        {

            char c1 = fgetc(sourceFile);

            if (c1 == '-')
            {

                char c2 = fgetc(sourceFile);
                char c3 = fgetc(sourceFile);

                if (c2 == '-' && c3 == '-')
                    return makeToken(TK_ASSIGNOP, "<---");

                if (c3 != EOF)
                    ungetc(c3, sourceFile);
                if (c2 != EOF)
                    ungetc(c2, sourceFile);
                if (c1 != EOF)
                    ungetc(c1, sourceFile);

                return makeToken(TK_LT, "<");
            }

            if (c1 == '=')
            {
                return makeToken(TK_LE, "<=");
            }

            if (c1 != EOF)
                ungetc(c1, sourceFile);

            return makeToken(TK_LT, "<");
        }
        if (ch == '>')
        {
            char c1 = fgetc(sourceFile);
            if (c1 == '=')
                return makeToken(TK_GE, ">=");
            ungetc(c1, sourceFile);
            return makeToken(TK_GT, ">");
        }
        if (ch == '=')
        {

            char c1 = fgetc(sourceFile);

            if (c1 == '=')
                return makeToken(TK_EQ, "==");

            if (c1 != EOF)
                ungetc(c1, sourceFile);

            return makeToken(TK_ERROR, "=");
        }
        if (ch == '!')
        {
            if (fgetc(sourceFile) == '=')
                return makeToken(TK_NE, "!=");
            return makeToken(TK_ERROR, "!");
        }

        if (ch == '&')
        {

            char c1 = fgetc(sourceFile);
            char c2 = fgetc(sourceFile);

            if (c1 == '&' && c2 == '&')
                return makeToken(TK_AND, "&&&");

            if (c1 == '&')
            {
                ungetc(c2, sourceFile);
                return makeToken(TK_ERROR, "&&");
            }

            ungetc(c2, sourceFile);
            ungetc(c1, sourceFile);
            return makeToken(TK_ERROR, "&");
        }

        if (ch == '@')
        {

            char c1 = fgetc(sourceFile);
            char c2 = fgetc(sourceFile);

            if (c1 == '@' && c2 == '@')
                return makeToken(TK_OR, "@@@");

            if (c1 == '@')
            {
                ungetc(c2, sourceFile);
                return makeToken(TK_ERROR, "@@");
            }

            ungetc(c2, sourceFile);
            ungetc(c1, sourceFile);
            return makeToken(TK_ERROR, "@");
        }

        if (ch == '~')
            return makeToken(TK_NOT, "~");
        if (ch == '+')
            return makeToken(TK_PLUS, "+");
        if (ch == '-')
            return makeToken(TK_MINUS, "-");
        if (ch == '*')
            return makeToken(TK_MUL, "*");
        if (ch == '/')
            return makeToken(TK_DIV, "/");
        if (ch == '[')
            return makeToken(TK_SQL, "[");
        if (ch == ']')
            return makeToken(TK_SQR, "]");
        if (ch == '(')
            return makeToken(TK_OP, "(");
        if (ch == ')')
            return makeToken(TK_CL, ")");
        if (ch == ',')
            return makeToken(TK_COMMA, ",");
        if (ch == ';')
            return makeToken(TK_SEM, ";");
        if (ch == ':')
            return makeToken(TK_COLON, ":");
        if (ch == '.')
            return makeToken(TK_DOT, ".");

        char unknown[2] = {ch, '\0'};
        return makeToken(TK_ERROR, unknown);
    }

    return makeToken(TK_EOF, "EOF");
}