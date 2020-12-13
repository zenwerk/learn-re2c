# 入力終端処理その4・汎用APIを使ったカスタムメソッド

http://re2c.org/manual/manual_c.html#custom-methods-with-generic-api

汎用APIは、文字の読み込みなどの基本的な動作を上書きできます。
- 自動生成されるコードの各部分を自分でカスタムできる。
- 当然、入力終端処理の部分もカスタムできる。

:::message alert
この方法はバグを生みやすいので、他の方法が使えない場合に検討するべき。
:::

## 設定
- `--input custom` オプションか `re2c:flags:input = custom` を指定する
- `re2c:yyfill:enable = 0` を指定する

## サンプルコード

「境界チェック付き番兵文字」のサンプルを汎用APIで実装する例で以下の点が異なります。
- 入力文字列がNULL文字（`\0`）で終了しない
  - この方法は、単一の番兵文字でさえも、パディングをまったく行うことができない場合に使う
- 入力の最後に番兵文字がこないことをカバーするために、`YYPEEK` が次の文字の入力前に境界チェックをするように再定義している。
  - 読み込みのたびにチェック処理を行うので非効率である点に注意

```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// expect a string without terminating null
static int lex(const char *str, unsigned int len)
{
    const char *cur = str, *lim = str + len, *mar;
    int count = 0;

loop:
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:eof = 0;
    re2c:flags:input = custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE    = char;
    re2c:define:YYLESSTHAN = "cur >= lim";
    re2c:define:YYPEEK     = "cur < lim ? *cur : 0";  // fake null
    re2c:define:YYSKIP     = "++cur;";
    re2c:define:YYBACKUP   = "mar = cur;";
    re2c:define:YYRESTORE  = "cur = mar;";

    *                           { return -1; }
    $                           { return count; }
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
    [ ]+                        { goto loop; }

    */
}

// make a copy of the string without terminating null
// C言語の文字列の末尾ヌル文字を削除してレキサーに渡す関数
static void test(const char *str, unsigned int len, int res)
{
    char *s = (char*)malloc(len);
    memcpy(s, str, len);
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
```

自動生成されるコードは以下

```c
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// expect a string without terminating null
static int lex(const char *str, unsigned int len)
{
    const char *cur = str, *lim = str + len, *mar;
    int count = 0;

loop:
    
{
	char yych;
	yych = cur < lim ? *cur : 0;
	switch (yych) {
	case ' ':	goto yy4;
	case '\'':	goto yy7;
	default:
		if (cur >= lim) goto yyeof1;
		goto yy2;
	}
yy2:
	++cur;
yy3:
	{ return -1; }
yy4:
	++cur;
	yych = cur < lim ? *cur : 0;
	switch (yych) {
	case ' ':	goto yy4;
	default:	goto yy6;
	}
yy6:
	{ goto loop; }
yy7:
	++cur;
	mar = cur;
	yych = cur < lim ? *cur : 0;
	if (yych >= 0x01) goto yy9;
	if (cur >= lim) goto yy3;
yy8:
	++cur;
	yych = cur < lim ? *cur : 0;
yy9:
	switch (yych) {
	case '\'':	goto yy10;
	case '\\':	goto yy12;
	default:
		if (cur >= lim) goto yy13;
		goto yy8;
	}
yy10:
	++cur;
	{ ++count; goto loop; }
yy12:
	++cur;
	yych = cur < lim ? *cur : 0;
	if (yych <= 0x00) {
		if (cur >= lim) goto yy13;
		goto yy8;
	}
	goto yy8;
yy13:
	cur = mar;
	goto yy3;
yyeof1:
	{ return count; }
}

}

// make a copy of the string without terminating null
static void test(const char *str, unsigned int len, int res)
{
    char *s = (char*) malloc(len);
    memcpy(s, str, len);
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
```

生成されるC言語のコードがそれぞれ自分で定義した処理に置き換わっているのがわかると思います。
どの部分をカスタマイズしたいのかを比較すれば、どのように変更したいかが把握しやすくなるでしょう。