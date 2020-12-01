/* Generated by re2c 2.0.3 on Sun Nov 15 03:22:57 2020 */
// re2c $INPUT -o $OUTPUT 
#include <assert.h>

// expect a null-terminated string
static int lex(const char *str, unsigned int len)
{
    const char *YYCURSOR = str, *YYLIMIT = str + len, *YYMARKER;
    int count = 0;

loop:
    
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
	case ' ':	goto yy4;
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
	case 'z':	goto yy7;
	default:
		// 初期状態での境界チェック
		if (YYLIMIT <= YYCURSOR) goto yyeof1;
		// 境界チェックに失敗している場合は不正な文字が来たと判断する
		goto yy2;
	}
yy2: // *        { return -1; }
	++YYCURSOR;
	{ return -1; }
yy4: // [ ]+     { goto loop; }
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy4;
	default:	goto yy6;
	}
yy6:
	{ goto loop; }
yy7: // [a-z]+   { ++count; goto loop; }
	yych = *++YYCURSOR;
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
	case 'z':	goto yy7;
	default:	goto yy9;
	}
yy9:
	{ ++count; goto loop; }
yyeof1:
	{ return count; }
}

}

#define TEST(s, r) assert(lex(s, sizeof(s) - 1) == r)
int main()
{
    TEST("", 0);
    TEST("one two three", 3);
    TEST("f0ur", -1);
    return 0;
}