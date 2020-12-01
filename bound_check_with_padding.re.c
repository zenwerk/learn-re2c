// re2c $INPUT -o $OUTPUT 
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* YYMAXFILL の値を自動生成する */
/*!max:re2c*/

// expect YYMAXFILL-padded string
static int lex(const char *str, unsigned int len)
{
    const char *YYCURSOR = str, *YYLIMIT = str + len + YYMAXFILL;
    int count = 0;
    // http://re2c.org/manual/manual_c.html#bounds-checks-with-padding
loop:
    /*!re2c
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = char;
    re2c:define:YYFILL = "return -1;"; // 説明単純化のため常に失敗するように設定

    *                           { return -1; }
    // 停止ルール
    [\x00]                      { return YYCURSOR + YYMAXFILL - 1 == YYLIMIT ? count : -1; }
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
    [ ]+                        { goto loop; }

    */
}

// YYMAXFILL の分だけヌル文字を末尾にパディングしてレキサーに渡す
static void test(const char *str, unsigned int len, int res)
{
    char *s = (char*) malloc(len + YYMAXFILL);
    memcpy(s, str, len);
    memset(s + len, 0, YYMAXFILL); // YYMAXFILL分\0を末尾にパディングセットする
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