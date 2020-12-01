// re2c $INPUT -o $OUTPUT -f
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
    in->yyaccept = 0; // storable-state での必須変数
    in->state = -1; // storable-state での必須変数
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