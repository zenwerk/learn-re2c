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
    /* 英単語をカウントするだけの簡単な例
     * センチネル文字を使う場合の YYFILL ルールは正しいコードを生成しない
     */
    /*!re2c
    re2c:eof = -1;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = in->cur;
    re2c:define:YYMARKER = in->mar;
    re2c:define:YYLIMIT  = in->lim;
    re2c:define:YYFILL   = "fill(in) == 0";

	*        { return -1; }
	[\x00]   { return count; }
	[a-z]+   { ++count; goto loop; }
	[ ]+     { goto loop; }

    */
}

int main()
{
    const char *fname = "input";
    const char str[] = "one two three";
    FILE *f;
    Input in;
    int result;

    // prepare input file: a few times the size of the buffer,
    // containing strings with zeroes and escaped quotes
    f = fopen(fname, "w");
    fwrite(str, 1, sizeof(str) - 1, f);
    // for (int i = 0; i < SIZE; ++i) {
    //     fwrite(str, 1, sizeof(str) - 1, f);
    // }
    fclose(f);

    f = fopen(fname, "r");
    init(&in, f);
    result = lex(&in);
    printf("result = %d\n", result);
    fclose(f);
    remove(fname);

    assert(result == 3);
    return 0;
}