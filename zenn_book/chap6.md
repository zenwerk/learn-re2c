# 入力終端処理その１・番兵文字

http://re2c.org/manual/manual_c.html#sentinel-character

番兵とはWikipediaによれば以下の通りです。
- https://ja.wikipedia.org/wiki/%E7%95%AA%E5%85%B5

```
データの終了を示すために配置される特殊なデータを指す。

実際にはこの用語は、微妙に異なる以下の2つの意味で使われる。

- 実データには出現しない、データの終了を表すための専用の値
- 入力データを処理するループの終了条件が複数ある場合に、条件判定の数を削減するために置くダミーのデータ
```

C言語では文字列の `\0` ことヌル文字がそれです。
つまり、番兵に指定された文字はレキサーの解析ルール（正規表現）に使われてはいけないし、
レキサーに入力される文字列の最後には必ず番兵文字が付与されていることが条件です。

## 設定

「番兵文字」ルールを使うためには以下のように設定します。

- `re2c:yyfill:enable = 0` （バッファ再充填関数が不要）
- `re2c:eof = -1` （EOFルールを設定しない）
- `re2c:sentinel` を設定するとレキサーの生成時に正しく設定されているかチェックできる
    - あくまでも設定チェックのための追加機能のようなもの
    - `-Wsentinel-in-midrule` を指定して警告出力すること


## 番兵文字ルールのサンプルコード

```c
#include <assert.h>

// '\0'で終わる文字列が来ることを想定している（番兵文字）。
// 文字列中のスペースで区切られた単語数をカウントする例。
// 入力が十分小さく、明確なセンチネル文字があるパターン。
static int lex(const char *YYCURSOR)
{
    int count = 0;
loop:
    /*!re2c
    re2c:define:YYCTYPE = char;
    // EOFチェックとYYFILLは不要になる
    re2c:yyfill:enable = 0;

    *      { return -1; }
    // re2c:eof = 0 として [\x00] を $ に置き換えても等価だが re2c:sentinel は eof = -1 じゃないとだめ。
    [\x00] { return count; } // センチネル文字
    [a-z]+ { ++count; goto loop; }
    [ ]+   { goto loop; }

    */
}

int main()
{
    assert(lex("") == 0);
    assert(lex("one two three") == 3);
    assert(lex("f0ur") == -1);
    return 0;
}
```

自動生成されるコードは以下

```c
#include <assert.h>

static int lex(const char *YYCURSOR)
{
	int count = 0;
loop:
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy2; // センチネル文字
	case ' ':	goto yy6; // 区切り文字
	case 'a':
	case 'b':
	/* 省略 */
	case 'x':
	case 'y':
	case 'z':	goto yy9; // [a-z]+ { ++count; goto loop; } の継続
	default:	goto yy4; // [a-z]+ { ++count; goto loop; } の失敗
	}
yy2: // [\x00] { return count; }
	++YYCURSOR;
	{ return count; }
yy4: // *      { return -1; }
	++YYCURSOR;
	{ return -1; }
yy6: // [ ]+   { goto loop; }
	yych = *++YYCURSOR;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ goto loop; }
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':
	case 'b':
	case 'c':
/* 省略 */
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy9;
	default:	goto yy11;
	}
yy11: // [a-z]+ { ++count; goto loop; } の成功
	{ ++count; goto loop; }
}

}

int main()
{
	assert(lex("") == 0);
	assert(lex("one two three") == 3);
	assert(lex("f0ur") == -1);
	return 0;
}
```

## 動作解説

一つの字句を解析し `goto loop;` で先頭(初期状態）に戻ったあと、一番最初の `switch` に番兵文字判定がくる。
- 番兵文字の判断はレキサーの初期状態のみに行われる。
    - 番兵文字はルールの途中に現れてはいけないという条件があるため
    - 字句を読み込みレキサーのステートマシンが確定したあと、次の字句解析が始まるのか、それとも番兵文字が現れるのか？
      - 番兵文字だった場合は終了処理に `goto` してレキサーが終了する


## 使いどころ

番兵文字ルールは `re2c:yyfill:enable = 0` を設定する必要があるため、バッファ再充填関数を使用できません。
よって、入力される容量があらかじめ予測できる、もしくは固定長など場合に使えそうです。

ファイル全体の読み込みの責務がレキサー以外の場合などにも使えるかもしれません。