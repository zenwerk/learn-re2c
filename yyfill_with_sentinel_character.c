/* Generated by re2c 2.0.3 on Mon Nov 16 03:20:55 2020 */
// re2c $INPUT -o $OUTPUT 
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SIZE 4096

typedef struct {
    FILE *file;
    char buf[SIZE + 1], *lim, *cur, *mar, *tok;
    int eof;
} Input;

static int fill(Input *in)
{
    if (in->eof) {
        return 1;
    }
    const size_t free = in->tok - in->buf;
    if (free < 1) {
        return 2;
    }
    memmove(in->buf, in->tok, in->lim - in->tok);
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    in->lim += fread(in->lim, 1, free, in->file);
    in->lim[0] = 0;
    in->eof |= in->lim < in->buf + SIZE;
    return 0;
}

static void init(Input *in, FILE *file)
{
    in->file = file;
    in->cur = in->mar = in->tok = in->lim = in->buf + SIZE;
    in->eof = 0;
    fill(in);
}

static int lex(Input *in)
{
    int count = 0;
loop:
    in->tok = in->cur;

    /* \0 が途中で出現しても大丈夫な EOF=0 な単語カウント */

    /*!re2c
    re2c:eof = 0;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = in->cur;
    re2c:define:YYMARKER = in->mar;
    re2c:define:YYLIMIT  = in->lim;
    re2c:define:YYFILL   = "fill(in) == 0";

    *                           { return -1; }
    $                           { return count; }
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
    [ ]+                        { goto loop; }

    */

{
	char yych;
yyFillLabel0:
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy4;
	case '\'':	goto yy7;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			/* yyfill が成功 = 新しい文字が充填されたので再度チェックするため戻る */
			if (fill(in) == 0) goto yyFillLabel0;
			/* 初期状態でのEOFルール適用 */
			goto yyeof1;
		}
		goto yy2;
	}
yy2:
	++in->cur;
yy3:
	{ return -1; }
yy4:
	++in->cur;
yyFillLabel1:
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy4;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel1;
		}
		goto yy6;
	}
yy6:
	{ goto loop; }
yy7:
	in->mar = ++in->cur;
yyFillLabel2:
	yych = *in->cur;
	if (yych >= 0x01) goto yy9;
	/* 受諾できる文字以外のものが来た場合に yyfill チェック */
	if (in->lim <= in->cur) {
		if (fill(in) == 0) goto yyFillLabel2;
		goto yy3;
	}
yy8:
	++in->cur;
yyFillLabel3:
	yych = *in->cur;
yy9:
	switch (yych) {
	case '\'':	goto yy10;
	case '\\':	goto yy12;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel3;
			goto yy13;
		}
		goto yy8;
	}
yy10:
	++in->cur;
	{ ++count; goto loop; }
yy12:
	++in->cur;
yyFillLabel4:
	yych = *in->cur;
	if (yych <= 0x00) {
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel4;
			goto yy13;
		}
		goto yy8;
	}
	goto yy8;
yy13:
	in->cur = in->mar;
	goto yy3;
yyeof1:
	{ return count; }
}

}

int main()
{
    const char *fname = "input";
    const char str[] = "'qu\0tes' 'are' 'fine: \\'' ";
    FILE *f;
    Input in;

    // prepare input file: a few times the size of the buffer,
    // containing strings with zeroes and escaped quotes
    f = fopen(fname, "w");
    for (int i = 0; i < SIZE; ++i) {
        fwrite(str, 1, sizeof(str) - 1, f);
    }
    fclose(f);

    f = fopen(fname, "r");
    init(&in, f);
    assert(lex(&in) == SIZE * 3);
    fclose(f);

    remove(fname);
    return 0;
}