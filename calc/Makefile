YACC = bison -y
LEX = re2c
TARGET = calc

ifeq (Windows_NT,$(OS))
TARGET:=$(TARGET).exe
endif


all : $(TARGET)

.PHONY : all


y.tab.c : parse.y
	$(YACC) -v -o y.tab.c parse.y

lex.c : lex.re.c
	$(LEX) -i -o lex.c lex.re.c

parse.o : y.tab.c lex.c
	$(CC) -g -c y.tab.c -o parse.o

$(TARGET) : parse.o
	$(CC) -g parse.o -o $(TARGET)

clean :
	rm -f y.output y.tab.c
	rm -f lex.c
	rm -f *.o $(TARGET)
.PHONY : clean

rebuild: clean all