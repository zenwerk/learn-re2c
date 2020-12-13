# Storable State

http://re2c.org/manual/manual_c.html#storable-state

Storable State オプションはプッシュ型のパーサーを作成するための方法です。

## プル型とプッシュ型

### プル型のレキサー
`re2c`のデフォルト
- 必要なときに続々と次の字句を読み込もうと動作する

**入力が少しずつ利用可能**なシチュエーションではプル型は不便
- パーサからレキサーが呼び出された場合
- レキサーが次のメッセージを送信する前に、レキサーからの応答を待たなければならない他のプログラムと、ソケット等を介して通信している場合
- その他いろいろ…


### プッシュ型のレキサー
プッシュ型　→　新しい字句が必要になったら、現在の状態を保存して呼び出し元に `return` するような動作
- その後、新しい字句が入力可能になったら保存した状態から再開する。

`-f` / `--storable-state` オプションを指定　→　現在の状態を保存して呼び出し元へ return できるようになる
- 戻ったときに保存した状態から再開できる。

## Storable State で必要な設定
-  `YYSETSTATE()` / `YYGETSTATE(state)`　の定義
-  `yych`, `yyaccept`, `state` 変数をレキサーの状態として定義する。
   - `state` 変数は `-1` で初期化すること。
-  `YYFILL` は新たな入力を取得せず、外部プログラムへ `return` しなければならない
   - 戻り値がレキサーがさらに入力が必要かどうかを示す。
-  外部プログラムは、レキサーがさらに入力を必要としているのか確認でき、適切に応答すること
-  レキサーに入る前にコードを実行する必要がある場合は、`/*!getstate:re2c*/`ディレクティブを使う
-  生成コードをさらにいじるなら `state:abort`, `state:nextlabel` の設定をいじる

## サンプルコード
- `stdin` から読み取る

```c
// re2c $INPUT -o $OUTPUT -f
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DEBUG    1
#define LOG(...) if (DEBUG) fprintf(stderr, __VA_ARGS__);
#define BUFSIZE  10

typedef struct {
    FILE *file;
    char buf[BUFSIZE + 1], *lim, *cur, *mar, *tok;
    unsigned yyaccept;
    int state;
} Input;

static void init(Input *in, FILE *f)
{
    in->file = f;
    in->cur = in->mar = in->tok = in->lim = in->buf + BUFSIZE;
    in->lim[0] = 0; // append sentinel symbol
    in->yyaccept = 0; // storable-state での必須変数
    in->state = -1; // storable-state での必須変数
}

typedef enum {END, READY, WAITING, BAD_PACKET, BIG_PACKET} Status;

static Status fill(Input *in)
{
    const size_t shift = in->tok - in->buf;
    const size_t free = BUFSIZE - (in->lim - in->tok);

    if (free < 1) return BIG_PACKET;

    // 処理した分だけ各ポインタの指すアドレスを -shift
    memmove(in->buf, in->tok, BUFSIZE - shift);
    in->lim -= shift;
    in->cur -= shift;
    in->mar -= shift;
    in->tok -= shift;

    // limを起点に最大free分だけfileから読み取り
    const size_t read = fread(in->lim, 1, free, in->file);
    // \0 が番兵文字で来るので、read 0 となり in->lim と in->cur が重なる
	LOG("read %ld bytes\n", read);
	if (read == 0) {
		LOG("in->lim = %p ; in->cur = %p\n", in->lim, in->cur);
	}
    in->lim += read; // 読み込み分だけ末尾を後ろへ
    in->lim[0] = 0; // append sentinel symbol

    return READY;
}

static Status lex(Input *in, unsigned int *recv)
{
    char yych; // storable-state での必須変数
    /*!getstate:re2c*/
loop:
    in->tok = in->cur;
    /*!re2c
        re2c:eof = 0;
        re2c:api:style = free-form;
        re2c:define:YYCTYPE    = "char";
        re2c:define:YYCURSOR   = "in->cur";
        re2c:define:YYMARKER   = "in->mar";
        re2c:define:YYLIMIT    = "in->lim";
        re2c:define:YYGETSTATE = "in->state";
        re2c:define:YYSETSTATE = "in->state = @@;";
        re2c:define:YYFILL     = "return WAITING;"; // storable-state の YYFILL は呼び出し元へ現在の状態を return しなければならない。さらなる入力は呼び出し元から供給され、レジュームする。

        packet = [a-z]+[;];

        *      { return BAD_PACKET; }
        $      { return END; }
        packet { *recv = *recv + 1; goto loop; }
    */
}

void test(const char **packets, Status status)
{
    const char *fname = "pipe";
    FILE *fw = fopen(fname, "w");
    FILE *fr = fopen(fname, "r");
    setvbuf(fw, NULL, _IONBF, 0);
    setvbuf(fr, NULL, _IONBF, 0);

    Input in;
    init(&in, fr);
    Status st;
    unsigned int send = 0, recv = 0;

    // 無限ループ
    for (;;) {
        // lex が字句を解析すると Status を返す
        st = lex(&in, &recv);
        if (st == END) {
            LOG("done: got %u packets\n", recv);
            break;
        } else if (st == WAITING) {
            LOG("waiting...\n");
            if (*packets) {
                LOG("sent packet %u\n", send);
                fprintf(fw, "%s", *packets++);
                ++send;
            }
            // バッファ充填は /*!re2c .. */ の外側で呼び出して管理する
            st = fill(&in);
            LOG("queue: '%s'\n", in.buf);
            // 異常終了
            if (st == BIG_PACKET) {
                LOG("error: packet too big\n");
                break;
            }
            assert(st == READY);
        } else {
            // 異常終了
            assert(st == BAD_PACKET);
            LOG("error: ill-formed packet\n");
            break;
        }
    }

    LOG("\n");
    assert(st == status);
    if (st == END) assert(recv == send);

    fclose(fw);
    fclose(fr);
    remove(fname);
}

int main()
{
    const char *packets1[] = {0};
    const char *packets2[] = {"zero;", "one;", "two;", "three;", "four;", 0};
    const char *packets3[] = {"zer0;", 0};
    const char *packets4[] = {"goooooooooogle;", 0};

    test(packets1, END);
    test(packets2, END);
    test(packets3, BAD_PACKET);
    test(packets4, BIG_PACKET);

    return 0;
}
```

自動生成されるコードは以下

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DEBUG    0
#define LOG(...) if (DEBUG) fprintf(stderr, __VA_ARGS__);
#define BUFSIZE  10

typedef struct {
    FILE *file;
    char buf[BUFSIZE + 1], *lim, *cur, *mar, *tok;
    unsigned yyaccept;
    int state;
} Input;

static void init(Input *in, FILE *f)
{
    in->file = f;
    in->cur = in->mar = in->tok = in->lim = in->buf + BUFSIZE;
    in->lim[0] = 0; // append sentinel symbol
    in->yyaccept = 0;
    in->state = -1;
}

typedef enum {END, READY, WAITING, BAD_PACKET, BIG_PACKET} Status;

static Status fill(Input *in)
{
    const size_t shift = in->tok - in->buf;
    const size_t free = BUFSIZE - (in->lim - in->tok);

    if (free < 1) return BIG_PACKET;

    memmove(in->buf, in->tok, BUFSIZE - shift);
    in->lim -= shift;
    in->cur -= shift;
    in->mar -= shift;
    in->tok -= shift;

    const size_t read = fread(in->lim, 1, free, in->file);
    in->lim += read;
    in->lim[0] = 0; // append sentinel symbol

    return READY;
}

static Status lex(Input *in, unsigned int *recv)
{
    char yych;
    /*!getstate:re2c で生成されるコードが以下 */
    switch (in->state) {
default:
	goto yy0; // 初期状態へ
case 0:
	if (in->lim <= in->cur) goto yyeof1;
	goto yyFillLabel0;
case 1:
	if (in->lim <= in->cur) goto yy4;
	goto yyFillLabel1;
case 2:
	if (in->lim <= in->cur) goto yy10;
	goto yyFillLabel2;
}

loop:
    in->tok = in->cur;
    

yy0:
yyFillLabel0:
	yych = *in->cur;
	switch (yych) {
	case 'a':
	case 'b':
	case 'c':
/* 省略 */
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy5;
	default:
		if (in->lim <= in->cur) {
			in->state = 0; // 状態を保存する
			return WAITING;
		}
		goto yy3;
	}
yy3:
	++in->cur;
yy4:
	{ return BAD_PACKET; }
yy5:
	in->mar = ++in->cur;
yyFillLabel1:
	yych = *in->cur;
	switch (yych) {
	case ';':	goto yy6;
	case 'a':
	case 'b':
	case 'c':
/* 省略 */
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy8;
	default:
		if (in->lim <= in->cur) {
			in->state = 1;  // 状態を保存する
			return WAITING; // YYFILL の置換結果
		}
		goto yy4;
	}
yy6:
	++in->cur;
	{ *recv = *recv + 1; goto loop; }
yy8:
	++in->cur;
yyFillLabel2:
	yych = *in->cur;
	switch (yych) {
	case ';':	goto yy6;
	case 'a':
	case 'b':
	case 'c':
/* 省略 */
	case 'x':
	case 'y':
	case 'z':	goto yy8;
	default:
		if (in->lim <= in->cur) {
			in->state = 2;  // 状態を保存する
			return WAITING;
		}
		goto yy10;
	}
yy10:
	in->cur = in->mar;
	goto yy4;
yyeof1:
	{ return END; }

}

void test(const char **packets, Status status)
{
    const char *fname = "pipe";
    FILE *fw = fopen(fname, "w");
    FILE *fr = fopen(fname, "r");
    setvbuf(fw, NULL, _IONBF, 0);
    setvbuf(fr, NULL, _IONBF, 0);

    Input in;
    init(&in, fr);
    Status st;
    unsigned int send = 0, recv = 0;

    for (;;) {
        st = lex(&in, &recv);
        if (st == END) {
            LOG("done: got %u packets\n", recv);
            break;
        } else if (st == WAITING) {
            LOG("waiting...\n");
            if (*packets) {
                LOG("sent packet %u\n", send);
                fprintf(fw, "%s", *packets++);
                ++send;
            }
            st = fill(&in);
            LOG("queue: '%s'\n", in.buf);
            if (st == BIG_PACKET) {
                LOG("error: packet too big\n");
                break;
            }
            assert(st == READY);
        } else {
            assert(st == BAD_PACKET);
            LOG("error: ill-formed packet\n");
            break;
        }
    }

    LOG("\n");
    assert(st == status);
    if (st == END) assert(recv == send);

    fclose(fw);
    fclose(fr);
    remove(fname);
}

int main()
{
    const char *packets1[] = {0};
    const char *packets2[] = {"zero;", "one;", "two;", "three;", "four;", 0};
    const char *packets3[] = {"zer0;", 0};
    const char *packets4[] = {"goooooooooogle;", 0};

    test(packets1, END);
    test(packets2, END);
    test(packets3, BAD_PACKET);
    test(packets4, BIG_PACKET);

    return 0;
}
```

`YYFILL` や `return` するときに以下のようなコードが生成される。

```c
	default:
		if (in->lim <= in->cur) {
			in->state = 1;  // YYSETSTATE (状態を保存する)
			return WAITING; // YYFILL の置換結果がこうなる
		}
		goto yy4;
	}
```

`return` 前に `YYSETSTATE` で定義されたコードで、どこの箇所で `return` されたのかを構造体や変数などに保存する。

`/*!getstate:re2c*/` の箇所が以下のように置換される
- `re2c:define:YYGETSTATE` から取得できる状態変数からステートマシンの状態へ `goto` する

```c
    switch (in->state) {
default:
	goto yy0; // 初期状態へ
case 0:
	if (in->lim <= in->cur) goto yyeof1;
	goto yyFillLabel0;
case 1:
	if (in->lim <= in->cur) goto yy4;
	goto yyFillLabel1;
case 2:
	if (in->lim <= in->cur) goto yy10;
	goto yyFillLabel2;
}
```

これにより呼び出し前に保存した状態へ復帰します。