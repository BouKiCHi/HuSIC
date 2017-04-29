# HuSIC開発環境構築

このドキュメントはWindows環境向けの開発環境の構築を解説します。

## MSYS2をインストール

[MSYS2公式サイト](http://www.msys2.org/)
より実行ファイルをダウンロード、手順に従って最新の環境にする。
開発環境が32bit OSであればi686を、64bit OSであればx86_64を選ぶ。

### MSYS環境のアップデートと開発ツールのインストール

MSYS2 MSYSを起動し、

```shell
pacman -Syu
```

MSYS環境そのものをアップデート、完了後にウインドウを閉じる。

再び、MSYS2 MSYSを起動し、

```shell
pacman -Su
```

でその他のソフトウェアを変更。
exitなどでウインドウを閉じる。

MSYS2 MinGW32を起動し、

```shell
pacman -S make
pacman -S mingw-w64-i686-gcc
で、開発ツール一式をインストールする
```

### 開発ツールのパスを通す

「環境変数を編集」より、Pathに

```shell
C:\msys64\mingw32\bin
C:\msys64\usr\bin
```

を追加する。優先順位を上にするように、順番は...\mingw32\...の方が上になるようにする。

## HuCの作成

プロジェクトルートで

```shell
make huc
```

を実行する

## ソースコードのコンパイル

プロジェクトルートで

```shell
make husic
```

を実行する
