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