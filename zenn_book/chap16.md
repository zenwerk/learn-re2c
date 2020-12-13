# カスタマイズ例・全二重通信的に動作するコマンド＆レスポンスプロトコル

re2c を用途に合わせてカスタマイズする例として以下の issue でのやり取りを見てみます。

https://github.com/skvadrik/re2c/issues/241

## issue の概要
issuer の要望を要約すると以下の通りです。
- re2c を全二重通信でコミュニケーションするプロトコル(ESMTP)で使いたい。
- クライアントはサーバーへコマンドを送信、サーバー側はre2cで字句解析した後パース処理を行う
    - コマンドは`CRLF`で区切られたテキスト。
- クライアントはレスポンスが返る前にサーバーへ複数のコマンドを送信してくる可能性あり
    - もしくはレスポンスを待ってから、時間をおいてコマンドを送ることもある。
- コマンドがTCPパケットを介して分割され、サーバーが非ブロッキングIOに基づいて構築されることがあるため、プッシュモデルレクサーが必要になる。

これについて以下のような点で困ったそうです。
- 全体の入力長が不明だが、ネットワーク越しにバラバラにコマンドが送られてくるため、`YYFILL` のタイミングによってはサーバーがデッドロックする
    - `YYFILL` が有効な場合、入力がバッファ全体に充填されるまで re2c は動作を止めるため
    - コマンド全体が送られて来た時点で、バッファの状態によらずレキサーから解析した語彙をパーサーへ送りたい

## 作者の返信（解決編）
`re2c:eof` を `--storable-state` と組み合わせて書く。
`re2c:eof` とは「レクサーがEOF文字を検出するたびに、それが本物の文字であるか、より多くの入力が必要な状況であるかを確認する必要」があることをre2cに伝える設定。

作者が最終的に示したPoCコードは以下の通り

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <assert.h>

/*!max:re2c*/
static const size_t SIZE = 4096;

struct input_t {
  char buf[SIZE + YYMAXFILL];
  char *lim;
  char *cur;
  char *tok;
  char *mark;
  int state;
  unsigned yyaccept;
  char yych;
  FILE *file;

  input_t(FILE * file)
    : buf()
    , lim(buf + SIZE)
    , cur(lim)
    , tok(lim)
    , mark(lim)
    , state(0)
    , yyaccept(0)
    , yych(0)
    , file(file)
  {}

  bool fill()
  {
    const size_t free = SIZE - (lim - tok); // バッファサイズ - (残りの解析しなければならない領域)
    if (free < 1) return false;
    const size_t shift = (tok - buf); // 解析済み領域

    printf("buf: %p\ntok: %p\ncur: %p\nlim: %p\n", buf, tok, cur, lim);
    printf("free:%ld | shift:%ld\n", free, shift);

    memmove(buf, tok, SIZE - shift);
    lim -= shift;
    cur -= shift;
    tok -= shift;
    mark -= shift;
    printf("> Reading input, can take up to %lu bytes\n", free);
    size_t bytes_read = fread(lim, 1, free, file);

    // YYLIMIT が YYCURSOR と一緒に伸長していく
    lim += bytes_read;
    // ノンバッファリング(nonblocking)な動作にするため番兵文字を末尾に追加する
    // cur と lim が同時に伸長するので同じアドレスになる -> (lim <= cur)が常に引っかかる -> 常に storable-state の return が動く
    lim[0] = 0;

    // quick make a copy of buffer with newlines replaced w/ _
    char b[SIZE+YYMAXFILL];
    int written;
    written = snprintf(b, SIZE+YYMAXFILL, "%s", buf);
    for(int i = 0; i < written; i++) {
      if ('\n' == b[i]) { b[i] = '_'; }
    }
    printf("> Read %lu bytes from input, current buffer: >%.40s<\n", bytes_read, b);

    return true;
  }
};

enum status_t {
  OK,
  FAIL,
  NEED_MORE_INPUT,
  WHITESPACE,
  WORD,
  THING
};
const char * STATUSES[] = {
  "OK",
  "FAIL",
  "NEED_MORE_INPUT",
  "WHITESPACE",
  "WORD",
  "THING"
};

#define YYGETSTATE()  in.state
#define YYSETSTATE(s) in.state = s
#define YYFILL() do { \
  printf("< Returning for more input\n"); \
  return NEED_MORE_INPUT; \
} while (0)

static status_t lex(input_t &in)
{
/*!getstate:re2c*/
    in.tok = in.cur;
/*!re2c
    re2c:define:YYCTYPE  = char;
    re2c:define:YYCURSOR = in.cur;
    re2c:define:YYLIMIT  = in.lim;
    re2c:define:YYMARKER = in.mark;
    re2c:variable:yych   = in.yych;
    re2c:eof             = 0;

    *                       { printf("< Unexpected character >%c<\n", in.yych); return FAIL; }
    $                       { printf("< EOF\n");                                return OK; }
    [\n ]+                  { printf("< whitespace\n");                         return WHITESPACE; }
    [a-zA-Z]+               { printf("< word\n");                               return WORD; }
    "THING\nWITH\nNEWLINES" { printf("< Thing w/ newlines\n");                  return THING; }
*/
}

int main()
{
  int fds[2];
  pipe(fds);
  fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK);
  FILE * write = fdopen(fds[1], "w");
  FILE * read = fdopen(fds[0], "r");
  input_t in(read);

  const char * packets[] = {
    "THING\n",
    "WITH\n",
    "NEWLINES\n",
    "H", "E", "L", "O", "\n",
    "HELO\n",
    "THING\nWITH\n",
    "NEWLINES"
  };
  size_t num_packets = sizeof(packets) / sizeof(char *);
  size_t current_packet = 0;

  enum status_t result = NEED_MORE_INPUT;

  while (OK != result) {
    switch (result) {
      case NEED_MORE_INPUT:
        if (current_packet == num_packets) {
          printf("Not enough input\n");
          goto end;
        }

        fwrite(packets[current_packet], strlen(packets[current_packet]), 1, write);
        fflush(write);
        current_packet++;
        printf("Packet %lu / %lu written\n", current_packet, num_packets);

        if (current_packet == num_packets) {
          printf("%lu / %lu packets sent, Closing down communication channel\n",
            current_packet, num_packets);
          fclose(write);
          write = NULL;
        }

        // NEED_MORE_INPUT 状態のたびに読み込む
        in.fill();
        break;

      case FAIL:
        goto end;

      default:
        // careful, need to reset state (re2c forgets to do it)
        // コマンドの入力が完了したら、レキサーを初期状態に戻して番兵文字チェックを動作させる
        // re2c:eof が有効なので lim[0] = 0 が動作しレキサーが終了する
        YYSETSTATE(0); 
        break;
    }

    result = lex(in);
    printf("Received response from lexer: %s\n", STATUSES[result]);
    printf("----------------------------------------------------------------------------------\n");
  }

end:

  // cleanup
  fclose(read);
  if (write) {
    fclose(write);
  }

  return result == OK ? 0 : 1;
}

#undef YYGETSTATE
#undef YYSETSTATE
#undef YYFILL
```

これにいくつかの `printf` を加えて実行した結果が以下の通りです。

```
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a46 <-- ここが常に同じアドレスになるように調整している
lim: 0x7fffa1266a46 <--
```

のように `cur` と `lim` の値が`buf` の開始位置から読み込んだバイト数ずつ伸長していていくのがポイントです。
このように動作すると常に `YYFILL` の条件につかまり `re2c:eof` のチェック対象となります。
（すぐにレキサーから抜けれるように `lim[0] = 0` を手動でセットしているため）

```
Packet 1 / 11 written
buf: 0x7fffa1266a40
tok: 0x7fffa1267a40
cur: 0x7fffa1267a40
lim: 0x7fffa1267a40
free:4096 | shift:4096
> Reading input, can take up to 4096 bytes
> Read 6 bytes from input, current buffer: >THING_<
< Returning for more input
Received response from lexer: NEED_MORE_INPUT
----------------------------------------------------------------------------------
Packet 2 / 11 written
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a46
lim: 0x7fffa1266a46
free:4090 | shift:0
> Reading input, can take up to 4090 bytes
> Read 5 bytes from input, current buffer: >THING_WITH_<
< Returning for more input
Received response from lexer: NEED_MORE_INPUT
----------------------------------------------------------------------------------
Packet 3 / 11 written
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a4b
lim: 0x7fffa1266a4b
free:4085 | shift:0
> Reading input, can take up to 4085 bytes
> Read 9 bytes from input, current buffer: >THING_WITH_NEWLINES_<
< Thing w/ newlines
Received response from lexer: THING
----------------------------------------------------------------------------------
< Returning for more input
Received response from lexer: NEED_MORE_INPUT
----------------------------------------------------------------------------------

省略

Packet 9 / 11 written
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a59
lim: 0x7fffa1266a59
free:4071 | shift:0
> Reading input, can take up to 4071 bytes
> Read 5 bytes from input, current buffer: >THING_WITH_NEWLINES_HELO_HELO_<
< whitespace
Received response from lexer: WHITESPACE
----------------------------------------------------------------------------------
< word
Received response from lexer: WORD
----------------------------------------------------------------------------------
< Returning for more input
Received response from lexer: NEED_MORE_INPUT
----------------------------------------------------------------------------------
Packet 10 / 11 written
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a5e
lim: 0x7fffa1266a5e
free:4066 | shift:0
> Reading input, can take up to 4066 bytes
> Read 11 bytes from input, current buffer: >THING_WITH_NEWLINES_HELO_HELO_THING_WITH<
< whitespace
Received response from lexer: WHITESPACE
----------------------------------------------------------------------------------
< Returning for more input
Received response from lexer: NEED_MORE_INPUT
----------------------------------------------------------------------------------
Packet 11 / 11 written
11 / 11 packets sent, Closing down communication channel
buf: 0x7fffa1266a40
tok: 0x7fffa1266a40
cur: 0x7fffa1266a69
lim: 0x7fffa1266a69
free:4055 | shift:0
> Reading input, can take up to 4055 bytes
> Read 8 bytes from input, current buffer: >THING_WITH_NEWLINES_HELO_HELO_THING_WITH<
< Thing w/ newlines
Received response from lexer: THING
----------------------------------------------------------------------------------
< EOF
Received response from lexer: OK
----------------------------------------------------------------------------------
```