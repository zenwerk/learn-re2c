# バッファ充填処理・パディング付き境界チェックの場合


EOFルール未使用（`re2c:eof=-1`）の場合、`YYFILL`は引数１つ取り返り値`void`の関数ライクなプリミティブとなる。
- C言語で例えるなら `void YYFILL(int n)`
- 引数`n`は「最低限充填されるべき文字数」
  - 最低`n`文字充填しないとルールに沿ったLexingができなくなる。
  - なので`n`文字の読み込んでバッファに再充填できなかったら、呼び出し元に戻らずレキサーを終了させる
    - 失敗したときに呼び出し元の関数から戻るマクロとして実装するのがお勧め

`YYFILL`成功時
- `YYLIMIT`の位置を更新する
  - バッファ内の最後の入力位置の後
  - `YYMAXFILL`のパディング終了位置（引数分の文字を読み込んだがバッファ全体を埋めるほどの入力がない場合）


`YYFILL`の呼び出しトリガーは以下

- デフォルトAPI　→　`(YYLIMIT - YYCURSOR) < n`
- 汎用API　　　　→　`YYLESSTHAN(n)` が trueを返す

## 動作図解
`#` がパディング文字
`need` が YYFILL に渡される引数の値で、その数だけの文字が必要

```
YYFILL前
               <-- shift -->                 <-- need -->
             >-A------------B---------C-----D-------E---F--------G->
             buffer       token    marker cursor  limit

YYFILL後

シフトした分だけtoken,marker,cursorが移動する。
G地点までの文字でバッファ全体が埋まったのでパディングなし

>-A------------B---------C-----D-------E---F--------G->
             buffer,  marker cursor               limit
```

ファイル終端に差し掛かった場合

```
YYFILL前
               <-- shift -->                 <-- need -->
             >-A------------B---------C-----D-------E-F (EOF)
             buffer       token    marker cursor  limit

YYFILL後

F地点で入力が終わるためYYMAXFILLの分だけ末尾にパディングを追加する

>-A------------B---------C-----D-------E-F###############
             buffer,  marker cursor                   limit
             token                        <- YYMAXFILL ->
```

## サンプルコード
 # パディング付き境界チェックYYFILLのサンプルコード

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*!max:re2c*/
#define SIZE 4096

typedef struct {
    FILE *file;
    // YYMAXFILL分のパディング分だけバッファを広くとる
    char buf[SIZE + YYMAXFILL], *lim, *cur, *mar, *tok;
    int eof;
} Input;

static int fill(Input *in, size_t need)
{
    // ファイル終端に到達済み
    if (in->eof) {
        return 1;
    }
    // free = (現在解析している箇所 - バッファの開始地点) = 解析済みの領域
    const size_t free = in->tok - in->buf;
    if (free < need) {
        return 2;
    }
    // free分だけバッファをシフトする
    memmove(in->buf, in->tok, in->lim - in->tok);
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    // シフトした分だけアドレス位置を更新する
    in->lim += fread(in->lim, 1, free, in->file);
    // ファイル終端まで読み込んだかチェックしてフラグをセットしておく
    if (in->lim < in->buf + SIZE) {
        // フラグをセットする
        in->eof = 1;
        // ヌル文字をパディングする
        memset(in->lim, 0, YYMAXFILL);
        in->lim += YYMAXFILL;
    }
    return 0;
}

static void init(Input *in, FILE *file)
{
    in->file = file;
    in->cur = in->mar = in->tok = in->lim = in->buf + SIZE;
    in->eof = 0;
    fill(in, 1);
}

static int lex(Input *in)
{
    int count = 0;
loop:
    in->tok = in->cur;
    /*!re2c
    re2c:api:style = free-form;
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = in->cur;
    re2c:define:YYMARKER = in->mar;
    re2c:define:YYLIMIT  = in->lim;
    re2c:define:YYFILL   = "if (fill(in, @@) != 0) return -1;"; // YYFILL失敗時はレキサーを終了する

    *                           { return -1; }
    // 停止ルール
    [\x00]                      { return (in->lim - in->cur == YYMAXFILL - 1) ? count : -1; }
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

#define YYMAXFILL 1

#define SIZE 4096

typedef struct {
    FILE *file;
    char buf[SIZE + YYMAXFILL], *lim, *cur, *mar, *tok;
    int eof;
} Input;

static int fill(Input *in, size_t need)
{
    if (in->eof) {
        return 1;
    }
    const size_t free = in->tok - in->buf;
    if (free < need) {
        return 2;
    }
    memmove(in->buf, in->tok, in->lim - in->tok);
    in->lim -= free;
    in->cur -= free;
    in->mar -= free;
    in->tok -= free;
    in->lim += fread(in->lim, 1, free, in->file);
    if (in->lim < in->buf + SIZE) {
        in->eof = 1;
        memset(in->lim, 0, YYMAXFILL);
        in->lim += YYMAXFILL;
    }
    return 0;
}

static void init(Input *in, FILE *file)
{
    in->file = file;
    in->cur = in->mar = in->tok = in->lim = in->buf + SIZE;
    in->eof = 0;
    fill(in, 1);
}

static int lex(Input *in)
{
    int count = 0;
loop:
    in->tok = in->cur;
    
{
	char yych;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return -1; // YYFILL
	yych = *in->cur;
	switch (yych) {
	case 0x00:	goto yy2; // センチネル文字
	case ' ':	goto yy6;
	case '\'':	goto yy9;
	default:	goto yy4;
	}
yy2:
	++in->cur;
	{ return (in->lim - in->cur == YYMAXFILL - 1) ? count : -1; }
yy4: // *      { return -1; }
	++in->cur;
	{ return -1; }
yy6: // [ ]+   { goto loop; }
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return -1; // YYFILL
	yych = *in->cur;
	switch (yych) {
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ goto loop; }
yy9:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return -1; // YYFILL
	yych = *in->cur;
	switch (yych) {
	case '\'':	goto yy11;
	case '\\':	goto yy13;
	default:	goto yy9;
	}
yy11:
	++in->cur;
	{ ++count; goto loop; }
yy13:
	++in->cur;
	if (in->lim <= in->cur) if (fill(in, 1) != 0) return -1; // YYFILL
	yych = *in->cur;
	goto yy9;
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

YYFILL未定義の「パディング付き境界チェック」とほぼ同じコードが生成される。
`re2c:eof` と違うのは、オートマトンが移動してカーソルを移動した後に`YYFILL`チェックが挿入されること。
YYFILLでのバッファ容量確認後に、字句解析に入る。

`re2c:eof` の場合は、まず文字を読み込んで字句解析を行った後、失敗した場合にYYFILLでのバッファ容量確認処理を行う。