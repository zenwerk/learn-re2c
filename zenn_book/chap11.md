# バッファ充填処理・EOFルールの場合

http://re2c.org/manual/manual_c.html#yyfill-with-sentinel-character

`re2c:eof = ...` を設定している場合のバッファ再充填処理の実装は以下の方針となります。
- 引数なしの関数プリミティブとして動作する
  - 返り値が`0`なら成功、非ゼロ値なら失敗

`YYFILL`呼び出し条件は以下
- デフォルトAPI　→　`YYLIMIT <= YYCURSOR`
- 汎用API　　　　→　`YYLESSTHAN()`

以下のように動作させる
- 少なくとも１文字以上バッファに供給し、入力位置を調整する
- `limit`位置を、バッファ内の最後の入力文字の１文字後ろに常に設定する（**must**）
  - `limit` の位置には `re2c:eof` で指定した番兵文字を設置する

## 動作図解
`re2c:eof = #` とした場合
- `buffer`はバッファ開始位置
- `token` は現在読み込んでいる字句の開始位置
- `marker` は最後に一致した字句の位置
- `cursor` は現在読み込んでいる位置
- `limit` は番兵文字がある位置

```
YYFILL前
               <-- shift -->
             >-A------------B---------C-------------D#-----------E->
             buffer       token    marker         limit,
                                                  cursor

YYFILL後

buffer～tokenまではすでに解析済みの領域なので shift してバッファから消去する
shift した分だけ token,marker,cursor が後ろに移動する

>-A------------B---------C-------------D------------E#->
             buffer,  marker        cursor        limit
             token
```

バッファー全体を満たすのに十分な入力がない場合（入力ファイルの最後の方など）

```
YYFILL前
               <-- shift -->
             >-A------------B---------C-------------D#--E (EOF)
             buffer       token    marker         limit,
                                                  cursor

YYFILL後
shift 分だけtoken,marker,cursor がずれるが、
番兵文字は必ず入力文字の後ろに配置しなければらないので EndofFile の位置に従って limit 位置も移動する

>-A------------B---------C-------------D---E#........
             buffer,  marker       cursor limit
             token
```

## サンプルコード

# EOFルールのYYFILLのサンプルコード

入力ファイル`input.txt`を`4096バイト`のチャンクで読み取り、EOFルールを使用するプログラムの例

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SIZE 4096

typedef struct {
    FILE *file;
    // 最後の入力文字の1文字後ろ番兵文字を付与する必要があるので SIZE+1 のメモリを用意する
    char buf[SIZE + 1], *lim, *cur, *mar, *tok;
    int eof;
} Input;

static int fill(Input *in)
{
    // ファイル終端に到達済み
    if (in->eof) {
        return 1;
    }
    // free = (現在解析している箇所 - バッファの開始地点) = 解析済みの領域
    const size_t free = in->tok - in->buf;
    if (free < 1) { // shift する領域がない
        return 2;
    }
    // free分だけバッファをシフトする
    memmove(in->buf, in->tok, in->lim - in->tok);
    // シフトした分だけアドレス位置を更新する
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    // シフトした分だけファイルから読み込む
    in->lim += fread(in->lim, 1, free, in->file);
    // 番兵文字をセットする
    in->lim[0] = 0;
    // ファイル終端まで読み込んだかチェックしてフラグをセットしておく
    in->eof |= in->lim < in->buf + SIZE;
    return 0;
}

static void init(Input *in, FILE *file)
{
    in->file = file;
    in->cur = in->mar = in->tok = in->lim = in->buf + SIZE;
    in->eof = 0;
    fill(in);
}

static int lex(Input *in)
{
    int count = 0;
loop:
    in->tok = in->cur;

    /*!re2c
    re2c:eof = 0;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = in->cur;
    re2c:define:YYMARKER = in->mar;
    re2c:define:YYLIMIT  = in->lim;
    re2c:define:YYFILL   = "fill(in) == 0";

    *                           { return -1; }
    $                           { return count; }
    ['] ([^'\\] | [\\][^])* ['] { ++count; goto loop; }
    [ ]+                        { goto loop; }

    */
}

int main()
{
    const char *fname = "input";
    const char str[] = "'qu\0tes' 'are' 'fine: \\'' ";
    FILE *f;
    Input in;

    // prepare input file: a few times the size of the buffer,
    // containing strings with zeroes and escaped quotes
    f = fopen(fname, "w");
    for (int i = 0; i < SIZE; ++i) {
        fwrite(str, 1, sizeof(str) - 1, f);
    }
    fclose(f);

    f = fopen(fname, "r");
    init(&in, f);
    assert(lex(&in) == SIZE * 3);
    fclose(f);

    remove(fname);
    return 0;
}
```

自動生成されるコードは以下

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SIZE 4096

typedef struct {
    FILE *file;
    char buf[SIZE + 1], *lim, *cur, *mar, *tok;
    int eof;
} Input;

static int fill(Input *in)
{
    if (in->eof) {
        return 1;
    }
    const size_t free = in->tok - in->buf;
    if (free < 1) {
        return 2;
    }
    memmove(in->buf, in->tok, in->lim - in->tok);
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    in->lim += fread(in->lim, 1, free, in->file);
    in->lim[0] = 0;
    in->eof |= in->lim < in->buf + SIZE;
    return 0;
}

static void init(Input *in, FILE *file)
{
    in->file = file;
    in->cur = in->mar = in->tok = in->lim = in->buf + SIZE;
    in->eof = 0;
    fill(in);
}

static int lex(Input *in)
{
    int count = 0;
loop:
    in->tok = in->cur;
{
	char yych;
yyFillLabel0:
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy4;
	case '\'':	goto yy7;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			/* yyfill が成功 = 新しい文字が充填されたので再度チェックするため戻る */
			if (fill(in) == 0) goto yyFillLabel0;
			/* 初期状態でのEOFルール適用 */
			goto yyeof1;
		}
		goto yy2;
	}
yy2:
	++in->cur;
yy3: // *      { return -1; }
	{ return -1; }
yy4: // [ ]+   { goto loop; }
	++in->cur;
yyFillLabel1:
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy4;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel1;
		}
		goto yy6;
	}
yy6:
	{ goto loop; }
yy7:
	in->mar = ++in->cur;
yyFillLabel2:
	yych = *in->cur;
	if (yych >= 0x01) goto yy9;
	/* 受諾できる文字以外のものが来た場合に yyfill チェック */
	if (in->lim <= in->cur) {
		if (fill(in) == 0) goto yyFillLabel2;
		goto yy3;
	}
yy8:
	++in->cur;
yyFillLabel3:
	yych = *in->cur;
yy9:
	switch (yych) {
	case '\'':	goto yy10;
	case '\\':	goto yy12;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel3;
			goto yy13;
		}
		goto yy8;
	}
yy10:
	++in->cur;
	{ ++count; goto loop; }
yy12:
	++in->cur;
yyFillLabel4:
	yych = *in->cur;
	if (yych <= 0x00) {
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			if (fill(in) == 0) goto yyFillLabel4;
			goto yy13;
		}
		goto yy8;
	}
	goto yy8;
yy13:
	in->cur = in->mar;
	goto yy3;
yyeof1:
	{ return count; }
}

}

int main()
{
    const char *fname = "input";
    const char str[] = "'qu\0tes' 'are' 'fine: \\'' ";
    FILE *f;
    Input in;

    // prepare input file: a few times the size of the buffer,
    // containing strings with zeroes and escaped quotes
    f = fopen(fname, "w");
    for (int i = 0; i < SIZE; ++i) {
        fwrite(str, 1, sizeof(str) - 1, f);
    }
    fclose(f);

    f = fopen(fname, "r");
    init(&in, f);
    assert(lex(&in) == SIZE * 3);
    fclose(f);

    remove(fname);
    return 0;
}
```


YYFILL が入ると複雑に見えるが（実際複雑だけど…）自動生成されるコードは
「境界チェック番兵文字」に `goto` が追加されるだけ。

https://zenn.dev/zenwerk/scraps/3c9fd30757436c149ad1#comment-23e167bcbd375195500a

これが

```c
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
```

こう変化する

```c
{
	char yych;
yyFillLabel0:
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy4;
	case '\'':	goto yy7;
	default:
		/* 受諾できる文字以外のものが来た場合に yyfill チェック */
		if (in->lim <= in->cur) {
			/* yyfill が成功 = 新しい文字が充填されたので再度チェックするため戻る */
			if (fill(in) == 0) goto yyFillLabel0;
			/* 初期状態でのEOFルール適用 */
			goto yyeof1;
		}
		goto yy2;
	}
```

YYFILLの出現で `goto` の分岐が増えるが、番兵文字の基本的な考え方である以下は変わりません。

```
基本的に goto loop; で先頭に戻ったあと、一番最初の switch に番兵文字判定がくる。

- 番兵文字の判断はレキサーの初期状態のみに行われる。
    - 番兵文字はルールの途中に現れてはいけないという条件があるため
    - 字句を読み込みレキサーのステートマシンが確定したあと、次の字句解析が始まるのか、それとも番兵文字が現れるのか？
        - 番兵文字だった場合は終了処理に goto してレキサーが終了する
```