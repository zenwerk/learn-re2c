#include <assert.h>

int lex(const char *YYCURSOR)
{
/*!re2c
    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0;

    *       { return 1; }
    [a-z]+  { return 0; }
*/
}

int main()
{
    assert(lex("foo") == 0);
    assert(lex("012") == 1);
    return 0;
}