/* Generated by re2c 2.0.3 on Sun Nov 15 02:52:12 2020 */
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
    
{
	char yych;
	if (in->lim <= in->cur) fill(in) == 0;
	yych = *in->cur;
	switch (yych) {
	case 0x00:	goto yy2;
	case ' ':	goto yy6;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy9;
	default:	goto yy4;
	}
yy2:
	++in->cur;
	{ return count; }
yy4:
	++in->cur;
	{ return -1; }
yy6:
	++in->cur;
	if (in->lim <= in->cur) fill(in) == 0;
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ goto loop; }
yy9:
	++in->cur;
	if (in->lim <= in->cur) fill(in) == 0;
	yych = *in->cur;
	switch (yych) {
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy9;
	default:	goto yy11;
	}
yy11:
	{ ++count; goto loop; }
}

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