# ルールブロックの再利用

http://re2c.org/manual/manual_c.html#reusable-blocks

`-r` / `--reusable` オプションで有効になる機能で、`/*!rules:re2c ... */` で指定されたブロックを `/*use:re2c .. */` で再利用できるようになります （re2c ver1.2 から使用可能）

## サンプルコード
UNICODE を受け取るルールブロックを `UTF-8` と `UTF-32` で解析するレキサーをそれぞれ定義する例。

`--input-encoding utf8` オプションを指定しないとUNICODEをちゃんと解析できない。
- 指定しない場合、re2cはユニコード文字をプレーンなアスキーバイトシーケンスとして解釈する。
  - 16進数のエスケープシーケンスを使用する必要でるらしい（？）

```c
// re2c $INPUT -o $OUTPUT -r --input-encoding utf8
#include <assert.h>
#include <stdint.h>

// 共通利用するルールブロック
/*!rules:re2c
    re2c:yyfill:enable = 0;

    "∀x ∃y: p(x, y)" { return 0; }
    *                { return 1; }
*/

static int lex_utf8(const uint8_t *YYCURSOR)
{
    const uint8_t *YYMARKER;
    // 再利用ブロック1 (UTF-8)
    /*!use:re2c
    re2c:define:YYCTYPE = uint8_t;
    re2c:flags:8 = 1;
    */
}

static int lex_utf32(const uint32_t *YYCURSOR)
{
    const uint32_t *YYMARKER;
    // 再利用ブロック2 (UTF-32)
    /*!use:re2c
    re2c:define:YYCTYPE = uint32_t;
    re2c:flags:8 = 0;
    re2c:flags:u = 1;
    */
}

int main()
{
    static const uint8_t s8[] = // UTF-8
        { 0xe2, 0x88, 0x80, 0x78, 0x20, 0xe2, 0x88, 0x83, 0x79
        , 0x3a, 0x20, 0x70, 0x28, 0x78, 0x2c, 0x20, 0x79, 0x29 };

    static const uint32_t s32[] = // UTF32
        { 0x00002200, 0x00000078, 0x00000020, 0x00002203
        , 0x00000079, 0x0000003a, 0x00000020, 0x00000070
        , 0x00000028, 0x00000078, 0x0000002c, 0x00000020
        , 0x00000079, 0x00000029 };

    assert(lex_utf8(s8) == 0);
    assert(lex_utf32(s32) == 0);
    return 0;
}
```

自動生成されるコードは以下

```c
#include <assert.h>
#include <stdint.h>



static int lex_utf8(const uint8_t *YYCURSOR)
{
    const uint8_t *YYMARKER;
    
{
	uint8_t yych;
	yych = *YYCURSOR;
	switch (yych) {
	case 0xE2:	goto yy4;
	default:	goto yy2;
	}
yy2:
	++YYCURSOR;
yy3:
	{ return 1; }
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
	case 0x88:	goto yy5;
	default:	goto yy3;
	}
yy5:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x80:	goto yy7;
	default:	goto yy6;
	}
yy6:
	YYCURSOR = YYMARKER;
	goto yy3;
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':	goto yy8;
	default:	goto yy6;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy9;
	default:	goto yy6;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0xE2:	goto yy10;
	default:	goto yy6;
	}
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x88:	goto yy11;
	default:	goto yy6;
	}
yy11:
	yych = *++YYCURSOR;
	switch (yych) {
	case 0x83:	goto yy12;
	default:	goto yy6;
	}
yy12:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'y':	goto yy13;
	default:	goto yy6;
	}
yy13:
	yych = *++YYCURSOR;
	switch (yych) {
	case ':':	goto yy14;
	default:	goto yy6;
	}
yy14:
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy15;
	default:	goto yy6;
	}
yy15:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'p':	goto yy16;
	default:	goto yy6;
	}
yy16:
	yych = *++YYCURSOR;
	switch (yych) {
	case '(':	goto yy17;
	default:	goto yy6;
	}
yy17:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'x':	goto yy18;
	default:	goto yy6;
	}
yy18:
	yych = *++YYCURSOR;
	switch (yych) {
	case ',':	goto yy19;
	default:	goto yy6;
	}
yy19:
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy20;
	default:	goto yy6;
	}
yy20:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'y':	goto yy21;
	default:	goto yy6;
	}
yy21:
	yych = *++YYCURSOR;
	switch (yych) {
	case ')':	goto yy22;
	default:	goto yy6;
	}
yy22:
	++YYCURSOR;
	{ return 0; }
}

}

static int lex_utf32(const uint32_t *YYCURSOR)
{
    const uint32_t *YYMARKER;
    
{
	uint32_t yych;
	yych = *YYCURSOR;
	if (yych == 0x00002200) goto yy28;
	++YYCURSOR;
yy27:
	{ return 1; }
yy28:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych != 'x') goto yy27;
	yych = *++YYCURSOR;
	if (yych == ' ') goto yy31;
yy30:
	YYCURSOR = YYMARKER;
	goto yy27;
yy31:
	yych = *++YYCURSOR;
	if (yych != 0x00002203) goto yy30;
	yych = *++YYCURSOR;
	if (yych != 'y') goto yy30;
	yych = *++YYCURSOR;
	if (yych != ':') goto yy30;
	yych = *++YYCURSOR;
	if (yych != ' ') goto yy30;
	yych = *++YYCURSOR;
	if (yych != 'p') goto yy30;
	yych = *++YYCURSOR;
	if (yych != '(') goto yy30;
	yych = *++YYCURSOR;
	if (yych != 'x') goto yy30;
	yych = *++YYCURSOR;
	if (yych != ',') goto yy30;
	yych = *++YYCURSOR;
	if (yych != ' ') goto yy30;
	yych = *++YYCURSOR;
	if (yych != 'y') goto yy30;
	yych = *++YYCURSOR;
	if (yych != ')') goto yy30;
	++YYCURSOR;
	{ return 0; }
}

}

int main()
{
    static const uint8_t s8[] = // UTF-8
        { 0xe2, 0x88, 0x80, 0x78, 0x20, 0xe2, 0x88, 0x83, 0x79
        , 0x3a, 0x20, 0x70, 0x28, 0x78, 0x2c, 0x20, 0x79, 0x29 };

    static const uint32_t s32[] = // UTF32
        { 0x00002200, 0x00000078, 0x00000020, 0x00002203
        , 0x00000079, 0x0000003a, 0x00000020, 0x00000070
        , 0x00000028, 0x00000078, 0x0000002c, 0x00000020
        , 0x00000079, 0x00000029 };

    assert(lex_utf8(s8) == 0);
    assert(lex_utf32(s32) == 0);
    return 0;
}
```

指定されたエンコーディングによって、再利用されたルールブロックが適切にコード生成されていることが分かります。