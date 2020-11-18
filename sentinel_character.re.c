#include <assert.h>

static int lex(const char *YYCURSOR)
{
	int count = 0;
	// http://re2c.org/manual/manual_c.html#sentinel-character
loop:
	/*!re2c
	re2c:define:YYCTYPE = char;
	re2c:yyfill:enable = 0;

	*      { return -1; }
	[\x00] { return count; }
	//[a-z]+ { ++count; goto loop; }
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
	[ ]+   { goto loop; }

	*/
}

int main()
{
	assert(lex("") == 0);
	assert(lex("'one' 'two' 'three'") == 3);
	assert(lex("'f0ur'") == 1);
	assert(lex("'qu\0tes' 'are' 'fine: \\'' ") == 3);
	return 0;
}
