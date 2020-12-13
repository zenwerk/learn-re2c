# サンプルコード・Bisonとの連携

最後に実際にパーサーと連携して動作するサンプルコードを示したいと思います。
re2c は Bison/Flex のように特定のツールと連携することを前提していないため、使用するパーサーとの連携部分は自分で調整する必要があります。

今回は Bison を利用しますが、他にも[Lemon](http://www.hwaci.com/sw/lemon/)や[PackCC](https://en.wikipedia.org/wiki/PackCC)などとも連携できると思います。

## Bison との連携
Bisonはデフォルト設定ではプル型パーサーを生成します。
プル型なのでパーサーが内部でレキサーを呼び出します。このとき呼び出す関数は `yylex()` という Lex/Flex が生成する関数が前提となっています。

この `yylex` の仕様に沿った関数を re2c で生成するのは至難の業なので、今回は re2c が Bison を呼び出す、プッシュ型パーサーを作成したいと思います。

Bison でプッシュ型パーサーを作成する情報は以下を参考にしました。

https://www.gnu.org/software/bison/manual/html_node/Push-Decl.html#Push-Decl

https://stackoverflow.com/questions/42434603/how-can-flex-return-multiple-terminals-at-one-time

## サンプルコード

作成するのは非常に簡単な整数電卓です。
急作りのためエラーメッセージやリカバリの考慮がほとんどありませんが、最低限動作します。

- パーサー部
```y:parse.y
%{
#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#define SIZE 4096

void yyerror(const char* s);
%}

%define api.pure full /* 再入可能パーサー */
%define api.push-pull push /* プッシュパーサー */
%define parse.error verbose

%token NL
%token NUMBER
%token LP
%token RP
%left ADDOP SUBOP
%left MULOP DIVOP
%right UMINUS

%%

s    : list
     ;

list : /* empty */
     | list expr NL { printf("%d\n", $2); }
     ;

expr : expr ADDOP expr { $$ = $1 + $3; }
     | expr SUBOP expr { $$ = $1 - $3; }
     | expr MULOP expr { $$ = $1 * $3; }
     | expr DIVOP expr 
       {
           if ($3 == 0) {
               fprintf(stderr, "Zero Division Error\n");
               YYERROR;
           }
           $$ = $1 / $3; 
       }
     | SUBOP expr %prec UMINUS { $$ = -$2; }
     | LP expr RP              { $$ = $2; }
     | NUMBER                  { $$ = $1; }
     ;
%%

#include "lex.c"

int interpret()
{
    int n;
    char buffer[SIZE];
    yypstate *ps = yypstate_new();

    do {
        fgets(buffer, SIZE, stdin);
        n = yylex(buffer, ps);
    } while (n == 0);

    yypstate_delete(ps);
    return n;
}

void yyerror(const char* s)
{
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}

int main()
{
    int n;
    n = interpret();
    if (n > 0) return 1;

    return 0;
}
```

- レキサー部
```c:lex.re.c
#include <stdlib.h>
#include <string.h>


#define PUSH_TOKEN(token, yylval) do {               \
  int status = yypush_parse(pstate, token, yylval);  \
  if (status != YYPUSH_MORE) {                       \
    yypstate_delete(pstate);                         \
    return status;                                   \
  }                                                  \
} while (0)

int yylex(char *cursor, yypstate *pstate)
{
    int num;
    char *marker;
    char *token;

loop:
    token = cursor;
/*!re2c
    re2c:api:style = free-form;
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = cursor;
    re2c:define:YYMARKER = marker;
    re2c:yyfill:enable   = 0;

    D = [0-9];
    N = [1-9];

    *      { return -1; }
    [\x00] { return 0; }
    [ \t]  { goto loop; }

    "+"  { PUSH_TOKEN(ADDOP, NULL); goto loop; }
    "-"  { PUSH_TOKEN(SUBOP, NULL); goto loop; }
    "*"  { PUSH_TOKEN(MULOP, NULL); goto loop; }
    "/"  { PUSH_TOKEN(DIVOP, NULL); goto loop; }
    "("  { PUSH_TOKEN(LP, NULL); goto loop; }
    ")"  { PUSH_TOKEN(RP, NULL); goto loop; }
    "\n" { PUSH_TOKEN(NL, NULL); goto loop; }

    "0" {
        num = 0;
        PUSH_TOKEN(NUMBER, &num);
        goto loop;
    }

    N{1}D* {
        size_t size = cursor - token;
        char *yytext = (char *)calloc(size, sizeof(char));
        memcpy(yytext, token, size);
        num = atoi(yytext);
        PUSH_TOKEN(NUMBER, &num);
        free(yytext);
        goto loop;
    }
*/
}
```

- Makefile
```Makefile
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
```

## 動作例

```bash
$ make
bison -y -v -o y.tab.c parse.y
re2c -i -o lex.c lex.re.c
cc -g -c y.tab.c -o parse.o
cc -g parse.o -o calc

$ ./calc
1 + 1
2
(-10 / 2) + 3 * 4
7
1 + + 2
Parse error: syntax error, unexpected ADDOP, expecting NUMBER or LP or SUBOP
```