// re2c $INPUT -o $OUTPUT 
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