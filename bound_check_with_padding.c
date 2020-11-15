/* Generated by re2c 2.0.3 on Sun Nov 15 01:41:56 2020 */
// re2c $INPUT -o $OUTPUT 
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define YYMAXFILL 1


// expect YYMAXFILL-padded string
static int lex(const char *str, unsigned int len)
{
    const char *YYCURSOR = str, *YYLIMIT = str + len + YYMAXFILL;
    int count = 0;

loop:
    
{
	char yych;
	if (YYLIMIT <= YYCURSOR) return -1;
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy2;
	case ' ':	goto yy6;
	case '\'':	goto yy9;
	default:	goto yy4;
	}
yy2:
	++YYCURSOR;
	{ return YYCURSOR + YYMAXFILL - 1 == YYLIMIT ? count : -1; }
yy4:
	++YYCURSOR;
	{ return -1; }
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1;
	yych = *YYCURSOR;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ goto loop; }
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1;
	yych = *YYCURSOR;
	switch (yych) {
	case '\'':	goto yy11;
	case '\\':	goto yy13;
	default:	goto yy9;
	}
yy11:
	++YYCURSOR;
	{ ++count; goto loop; }
yy13:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1;
	yych = *YYCURSOR;
	goto yy9;
}

}

// make a copy of the string with YYMAXFILL zeroes at the end
static void test(const char *str, unsigned int len, int res)
{
    char *s = (char*) malloc(len + YYMAXFILL);
    memcpy(s, str, len);
    memset(s + len, 0, YYMAXFILL); // YYMAXFILL分\0を末尾にパディングセットしている
    int r = lex(s, len);
    free(s);
    assert(r == res);
}

#define TEST(s, r) test(s, sizeof(s) - 1, r)
int main()
{
    TEST("", 0);
    TEST("'qu\0tes' 'are' 'fine: \\'' ", 3);
    TEST("'unterminated\\'", -1);
    return 0;
}