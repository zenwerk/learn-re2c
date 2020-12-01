// re2c $INPUT -o $OUTPUT 
#include <assert.h>

// expect a null-terminated string
static int lex(const char *str, unsigned int len)
{
    const char *YYCURSOR = str, *YYLIMIT = str + len, *YYMARKER;
    int count = 0;

loop:
    /*!re2c
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0; // バッファ充填関数を使用しない
    re2c:eof = 0; // ヌル文字(\0)を番兵文字として指定

    *                           { return -1; } // 失敗
    $                           { return count; } // 番兵文字が出現したときのルール
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
    [ ]+                        { goto loop; }

    */
}

#define TEST(s, r) assert(lex(s, sizeof(s) - 1) == r)
int main()
{
    TEST("", 0);
    TEST("'qu\0tes' 'are' 'fine: \\'' ", 3);
    TEST("'unterminated\\'", -1);
    return 0;
}