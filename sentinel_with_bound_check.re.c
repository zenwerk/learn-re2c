// re2c $INPUT -o $OUTPUT 
#include <assert.h>

// expect a null-terminated string
static int lex(const char *str, unsigned int len)
{
    const char *YYCURSOR = str, *YYLIMIT = str + len, *YYMARKER;
    int count = 0;

    // http://re2c.org/manual/manual_c.html#sentinel-character-with-bounds-checks
loop:
    /*!re2c
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0;
    re2c:eof = 0;

    *        { return -1; }
    $        { return count; }
    [a-z]+   { ++count; goto loop; }
    [ ]+     { goto loop; }

    */
}

#define TEST(s, r) assert(lex(s, sizeof(s) - 1) == r)
int main()
{
    TEST("", 0);
    TEST("one two three", 3);
    TEST("f0ur", -1);
    return 0;
}