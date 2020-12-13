# エンコーディングサポート

re2c はデフォルトで複数の文字エンコーディングに対応しています。

ASCII以外の文字コードでレキサーのルールを定義するうえで以下の点に注意します。
- `[^]` は `any` ルールと呼ぶ　→　すべての「コードポイント」にマッチするルール
    - つまりすべての「文字」にマッチする、やや抽象的なルール
    - コードポイント ≠ 具体的にエンコーディングされた文字表現ではないので注意

エンコーディングに関わらずすべてのコード表現（具体的な文字表現のバイト列）にマッチするためのルールは、これまで同様 `*` を使います。

:::message info
`YYCTYPE`のサイズは、コード単位のサイズと同じにすること
:::

ユニコードならコード単位は 1byte なので`unsigned char`
- utf-8 なら1byteのコード単位を1～4個組み合わせて1文字を表現している。

## サポートするエンコーディング

- `ASCII`
  - デフォルト設定
  - 固定長エンコーディング
    - [0-255] の 1byte
- `EBCDIC`
  - `-e` / `--ecb` で有効の固定長エンコーディング
- `UCS2`
  - `-w` / `--wide-chars` で有効
  - 2バイトの固定長エンコーディング
- `UTF8`
  - `-8` / `--utf-8` / `re2c:flags:8 = 1`
  - `[0-0x10FFFF]` の間での１～４バイトの可変長エンコーディング
- `UTF16`
  - `-x` / `--utf-16` で有効
  - １～２バイトの間の可変長エンコーディング（？）
- `UTF32`
  - `-u` / `--unicode` で有効
  - 4バイト固定長エンコーディング

## 各種設定・オプション
`/*!include:re2c "unicode_categories.re" */` すると re2c が事前定義しているユニコードのカテゴリを取り込める。
- http://tool-support.renesas.com/autoupdate/support/onlinehelp/ja-JP/csp/V4.01.00/CS+.chm/Editor.chm/Output/ed_RegularExpressions4-nav-2.html

https://github.com/skvadrik/re2c/blob/master/include/unicode_categories.re

ユニコードリテラルを指定した正規表現ルールを定義するときは `--input-encoding utf8` を指定する。
UTF16のサロゲートペアの扱いを指定するときは `--encoding-policy <fail | substitute | ignore>` のオプションを指定する。

## サンプルコード

```c
// re2c $INPUT -o $OUTPUT -8 --case-ranges -i
//
// Simplified "Unicode Identifier and Pattern Syntax"
// (see https://unicode.org/reports/tr31)

#include <assert.h>
#include <stdint.h>

/*!include:re2c "unicode_categories.re" */

static int lex(const char *YYCURSOR)
{
    const char *YYMARKER;
    /*!re2c
    re2c:define:YYCTYPE = 'unsigned char';
    re2c:yyfill:enable  = 0;

    id_start    = L | Nl | [$_]; // ユニコードの一般カテゴリを指定している
    id_continue = id_start | Mn | Mc | Nd | Pc | [\u200D\u05F3];
    identifier  = id_start id_continue*;

    identifier { return 0; }
    *          { return 1; }
    */
}

int main()
{
    assert(lex("_Ыдентификатор") == 0);
    assert(lex("$_Ыдентификатор") == 0);
    assert(lex("asdfasdfЫдентификатор") == 0);
    assert(lex("あ_Ыдентификатор") == 0);
    assert(lex("0a1cЫдентификатор") == 1);
    return 0;
}
```

自動生成されるコードは以下は長大なので省略します。