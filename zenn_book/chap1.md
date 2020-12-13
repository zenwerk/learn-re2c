# はじめに

:::message info
この本は [言語実装 Advent Calendar 2020](https://qiita.com/advent-calendar/2020/lang_dev) の14日目の記事として書かれました。
:::


re2c はC言語・Go言語向けの字句解析器生成ソフトウェアです。同種のソフトウェアでは[Lex](https://ja.wikipedia.org/wiki/Lex)などが有名です。

https://re2c.org/index.html

Lexと比べるとあまり知名度はありませんが、PHPやビルドシステムのNinjaなどに採用されており実績のあるソフトウェアです。
しかしながら、情報の少なさや公式ドキュメントの難しさもあり日本語での情報が少ないのが現状です。

そこで、この書籍では公式ドキュメントをなぞる形式で補足を記入しなら re2c の使い方を解説していきます。

https://re2c.org/manual/manual_c.html

なおC言語向けの使用方法を解説し、Go言語向けの使用方法は解説しません。

:::message
この書籍では re2c のバージョン `2.0.3` を対象としています。
:::

:::message alert
筆者自身が re2c に熟達しているわけではないため、間違いなどがあるかもしれません。
その場合はコメントや twitter などで指摘いただければ幸いです。
:::