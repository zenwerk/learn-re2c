# バッファ充填処理（YYFILL）

http://re2c.org/manual/manual_c.html#buffer-refilling

`YYFILL` は入力文字が確保したメモリ領域に一度に全部マッピングできないときに必要になります。
- 入力が巨大だったり、Socketのようなストリーミング入力などの場合、etc...

なおバッファ再充填（`YYFILL`）関数はデフォルトで有効となっています（`re2c:yyfill:enable = 1`）

バッファ充填処理の基本的な方針は
- 固定サイズのバッファを`alloc`してバッファに収まるチャンク単位で入力文字を処理
- 現在のチャンクを処理したら、それを`move out`して新しいデータを`move in`

# バッファ再充填の動作要素

バッファ再充填関数は以下の各要素を使って状況に使用したい状況に合わせて自分で定義していきます。


`cursor`　→　次に読み取る文字。

- デフォルトAPI　→　`YYCURSOR`
- 汎用API　　　　→　`YYSKIP` と `YYPEEK`



`limit`　→　利用可能な最後の入力文字の **後の位置** 

- デフォルトAPI　→　`YYLIMIT`
- 汎用API　　　　→　`YYLESSTHAN` で暗黙的に処理する
  - つまり`YYLIMIT`のような明示的な定義は存在せず `YYLESSTHAN` のなかで最後の位置を操作する処理を入れてくださいということ（？）



`marker`　→　最後に一致した字句の位置

- デフォルトAPI　→　`YYMARKER`
- 汎用API　　　　→　`YYBACKUP` と `YYRESTORE`



`token`　→　現在マッチしているトークンの開始位置

- `re2c`側ではAPIの用意なし



`context marker`　→　`trailing context`(末尾コンテキスト)の位置

- デフォルトAPI　→　`YYCTXMARKER`
- 汎用API　　　　→　`YYBACKUPCTX` と `YYRESTORECTX`



`tag variables`　→　`/*!stags:re2c */` と `/*!mtags:re2c */`で定義した、サブマッチの位置。

- デフォルトAPI　→　なし
- 汎用API　　　　→　`YYSTAGP` と `YYSTAGN`、`YYMTAGP` と `YYMTAGN`

----

上記要素のどれを使うかは場合によりけりですが、使う場合は `YYFILL` での処理で更新する必要があります（**must**）

対象としてアクティブな領域は `token`と`cursor`の間の領域に含まれているため、バッファの先頭と`token`の間の領域は`free/delete`可能で、`token`から`limit`までの領域をバッファの先頭に移動し、バッファの最後に空き領域を移動して、新しいデータを入力する、というのが基本的な方針でしょう。

また一度に再充填する領域は大きいほうが`YYFILL`の回数が減って効率は良くなります。

次章から`YYFILL`の実装サンプルを見てきます。