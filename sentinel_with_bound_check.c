/* Generated by re2c 2.0.3 on Thu Nov 26 03:04:50 2020 */
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
	case '\'':	goto yy7;
	default:
		// 初期状態での番兵文字と境界チェック
		if (YYLIMIT <= YYCURSOR) goto yyeof1;
		// 境界チェックに失敗している場合は不正な文字が来たと判断する
		goto yy2;
	}
yy2:
	++YYCURSOR;
yy3: // *    { return -1; }
	{ return -1; }
yy4: // [ ]+     { goto loop; }
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy4;
	default:	goto yy6;
	}
yy6:
	{ goto loop; }
yy7:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych >= 0x01) goto yy9;
	// カーソルが境界に達してたら失敗
	if (YYLIMIT <= YYCURSOR) goto yy3;
yy8:
	yych = *++YYCURSOR;
yy9:
	switch (yych) {
	case '\'':	goto yy10;
	case '\\':	goto yy12;
	default:
		// カーソルが境界に達してたら失敗
		if (YYLIMIT <= YYCURSOR) goto yy13;
		goto yy8;
	}
yy10:
	++YYCURSOR;
	{ ++count; goto loop; }
yy12:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		// カーソルが境界に達してたら失敗
		if (YYLIMIT <= YYCURSOR) goto yy13;
		goto yy8;
	}
	goto yy8;
yy13:
	YYCURSOR = YYMARKER;
	goto yy3;
yyeof1:
	{ return count; }
}

}

#define TEST(s, r) assert(lex(s, sizeof(s) - 1) == r)
int main()
{
    TEST("", 0);
    TEST("'qu\0tes' 'are' 'fine: \\'' ", 3);
    TEST("'unterminated\\'", -1);
    return 0;
}