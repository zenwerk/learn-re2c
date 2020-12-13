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