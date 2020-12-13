# 入力終端処理その3・パディング付き境界チェック

文字パディングを利用した境界チェックを使用して、入力の終わりを処理する方法。

https://re2c.org/manual/manual_c.html#bounds-checks-with-padding

言語で利用する場合のre2cのデフォルト動作(無設定のこと)となります。
- `re2c:yyfill:enable` はデフォルト値が `1` で有効設定
    - よってバッファ充填関数（`YYFILL`）の定義が必要
- `re2c:eof` はデフォルト値が `-1` で無効設定

汎用的かつ「境界チェック付き番兵文字」より（一般的には）早く動作するとのことです。
しかし「境界チェック付き番兵文字」よりは使用方法が複雑になります。

## 基本的な動作

このルールの「境界チェック」はオートマトンの強く接続された構成要素（SCC）に依存するいくつかの状態で以下のように生成されます。
- デフォルトAPI　→　`(YYLIMIT - YYCURSOR) < n`
- 汎用API　→　`YYLESSTHAN(n)`

`n` は、レキサーが処理を進めるのに必要な文字数の最小値です。
- レキサーを動かし続けるには、少なくとも`n`文字が追加で供給されなければならない、ということ。
- 次の境界チェックが最大`n`文字で行われることも意味します。

境界チェックが成功した場合、レキサーは動作を続行します。

境界チェックが失敗した場合は以下のように動作します。
- レキサーは入力の終わりに到達し、`YYFILL(n)`を呼び出し、以下のパターンのいずれかとなります。
  - 少なくとも`n`個の入力文字を供給する → 動作が継続する
  - なにも返さない → レキサーを終了する


## 設定方法
1. `/*!max:re2c*/` を書く
    - これは最低限必要なパディング文字数`YYMAXFILL`を自動生成してくれます
2. `re2c:eof = -1` （デフォルト設定なので記載する必要なし）
3. `re2c:yyfill:enable = 1` （デフォルト設定なので記載する必要なし）
4. `re2c:define:YYFILL` を定義する

## サンプルコード
`YYMAXFILL`の分だけNULL文字をパディングします。（`/*!max:re2c*/`）

- パディングに使う文字は、構文エラーにならない文字以外なら制限はありません。
  - 今回のサンプルでは `'` のような一重引用符はNG！

この方法には、パディング文字にマッチする「停止ルール」が必要です。

- 今回の例ではNULL文字
  - 停止ルールは、パディングの開始位置でマッチしたときのみ成功する。
  - それ以外での（迷子の）NULL文字マッチは構文エラーとなる。

```c
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
```

自動生成されるコードは以下

```c
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
	if (YYLIMIT <= YYCURSOR) return -1; // YYFILL の処理
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy2; // パディング文字
	case ' ':	goto yy6;
	case '\'':	goto yy9;
	default:	goto yy4;
	}
yy2:
	++YYCURSOR;
	// 停止ルールのチェック
	{ return YYCURSOR + YYMAXFILL - 1 == YYLIMIT ? count : -1; }
yy4:
	++YYCURSOR;
	{ return -1; }
yy6: // [ ]+  { goto loop; }
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1; // YYFILL
	yych = *YYCURSOR;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ goto loop; }
yy9:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1; // YYFILL
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
	if (YYLIMIT <= YYCURSOR) return -1; // YYFILL
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
```

生成されるコードは、レキサーのオートマトンが遷移するごとに以下のようにYYFILLコードが挿入される

```c
yy6: // [ ]+  { goto loop; }
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) return -1; // YYFILL
	yych = *YYCURSOR;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
```

つまり、遷移するごとに現在位置が`YYLIMIT`に達しているかチェックしています。

最後、パディング文字での「停止ルール」で、以下のように確認していいます

```c
// (現在のカーソル位置+YYMAXFILL-1) が YYLIMIT と等しいか?
{ return YYCURSOR + YYMAXFILL - 1 == YYLIMIT ? count : -1; }
```

これらのルールは以下のような終端文字をパディングするユーティリティ関数の存在が前提となっています。

```c
// make a copy of the string with YYMAXFILL zeroes at the end
// 入力文字列の最後にYYMAXFILLの分だけ\0をパディングした文字列のコピーを作成する
static void test(const char *str, unsigned int len, int res)
{
    char *s = (char*) malloc(len + YYMAXFILL);
    memcpy(s, str, len);
    memset(s + len, 0, YYMAXFILL); // YYMAXFILL分\0を末尾にパディングセットしている
    int r = lex(s, len);
    free(s);
    assert(r == res);
}
```

## まとめ
「パディング付き境界チェック」は、ようは「パディング」が「番兵文字」の扱いです。
「番兵文字」は単に出現したら終わり！という単純さがありますが、この方法場合は別途「停止ルール」を定義する必要があります。
その点を作者は「複雑」だと言っているのでしょうか（？）

このサンプルでは `YYFILL` は `-1` を返すのみ、かつ固定長の文字列の末尾に `YYMAXFILL` の分だけ `\0` を付与するので、単に「番兵文字」を長くしただけのように感じられ利点が薄く感じますが、バッファ充填関数を使う前提の方法なので、可変長な長さの入力に使用できる利点はあります。

（ただ、`YYFILL` が必要になるなら個人的には `re2c:eof` を使うルールの方がよさそうに感じました。）