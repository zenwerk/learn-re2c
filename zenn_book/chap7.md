# 入力終端処理その2・境界チェック付き番兵文字

http://re2c.org/manual/manual_c.html#sentinel-character-with-bounds-checks

汎用的で、使用できる正規表現ルールの制限なしだが、ある文字が「番兵」として入力の最後に付与される点では同じです。

単純な番兵文字ルールの場合は「番兵」に使用する文字は正規表現ルールで使用することはできませんが「境界チェック付き番兵文字」では番兵文字も正規表現ルール中に使用することができます。
つまり入力の途中に「番兵文字」が出現しても意図せずレキサーが止まることがありません。

## 境界チェック
「境界チェック」とはレキサーが現在読み込んでいる位置がバッファ領域を超えていないかを確認することです。
API的に表すと以下のようなコードです。

```c
YYLIMIT <= YYCURSOR 
```

## 動作の概要

レキサーが番兵文字を読み取ると、追加で「境界チェック」が実行される。
- 現在のカーソルが範囲内にある場合、番兵を通常の文字として処理し動作を継続する。
- カーソルが範囲外のとき、`YYFILL` で定義されたバッファ充填関数を実行し追加の文字列を読み込む（`re2c:yyfill:enable = 1`の場合）。
  - `YYFILL` が成功した場合、「番兵文字」を通常の処理として処理しレキサーが継続する
  - `YYFILL` が失敗した場合、「番兵文字」として扱い、レキサーが終了する。

## 設定方法

1. 「番兵文字」を `re2c:eof = ...` で設定する。
2. 使用の有無に関わらず、`re2c:yyfill:enable = ...` でバッファ充填関数の設定を明示する
3. 「番兵文字」の動作を `$` で設定する。

## サンプルコード

`'foo bar buzz'` のような引用符で囲まれた文字列中の単語数を数えるサンプル。

番兵文字は`\0`（`re2c:eof = 0;`）。
NULL文字は入力の最後を検出するために不可欠なシンボル。

単純な「番兵文字ルール」と違い、NULL文字も他の文字と同様にルールの **途中で出現可能**。

- 有効なルール　→　`'aaa\0aa'\0'`
- エラー入力　→　`'aaa\0`

境界チェックの条件は

- デフォルトAPI　→　`YYLIMIT <= YYCURSOR`
- 汎用API　→　`YYLESSTHAN(1)`

境界チェックが

- 成功　→　入力文字の解析を継続する
- 失敗　→　レクサーは入力の終わりに達しており、停止する


この例では `YYFILL`は無効なので追加の文字読み込み処理はありません。
よって入力の終わりは以下の3つの可能性があります。

- レクサーが初期状態の場合は、終端ルール `$` にマッチしている
- それ以外の場合
   - 前にマッチしていたルールにフォールバックする（`*`も含む）
   - もしくはデフォルト状態に戻る

```c
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
```

自動生成されるコードは以下

```c
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
```

特徴として、レキサーの初期状態（開始状態）に以下のチェックコードが挿入されます。

- `YYLIMIT` がバッファの末尾アドレス、`YYCURSOR`が現在のカーソル位置として
  - カーソルがバッファの末尾を超えたときに初期状態が受け付けない文字がきたかどうか

```c
default:
	// 初期状態での境界チェック
	if (YYLIMIT <= YYCURSOR) goto yyeof1;
	// 境界チェックに失敗している場合は不正な文字が来たと判断する
	goto yy2;
}
```

この例では `YYFILL` がないため、より柔軟なルールを扱える「番兵文字ルール」という趣きですが、`YYFILL` が有効な場合は可変長な長さの入力を処理できる汎用的な方法です。
`YYFILL` が有効な場合の例は後ほど登場します。