// re2c $INPUT -o $OUTPUT 
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