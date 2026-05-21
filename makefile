CC = gcc
CFLAGS = -Wall -g -std=c11

# Target executable
TARGET = stage1exe

# Source files
SRC = driver.c lexer.c parser.c

# Object files
OBJ = driver.o lexer.o parser.o

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compilation rules
driver.o: driver.c lexer.h parser.h parserDef.h lexerDef.h
	$(CC) $(CFLAGS) -c driver.c

lexer.o: lexer.c lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h parserDef.h lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c parser.c

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET)

# Rebuild everything
rebuild: clean all

# ============================================================
# Helpful test shortcuts (Optional but useful)
# ============================================================

# Lexer testing
test_lexer_t1:
	./$(TARGET) t1.txt out.txt

test_lexer_t2:
	./$(TARGET) t2.txt out.txt

# Parser correct cases
test_parser_t4:
	./$(TARGET) t4.txt parseTree_t4.txt

test_parser_t5:
	./$(TARGET) t5.txt parseTree_t5.txt

# Parser error case
test_parser_t6:
	./$(TARGET) t6.txt parseTree_t6.txt