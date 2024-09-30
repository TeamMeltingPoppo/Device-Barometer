# tweliteファームウェア

## 構成

- `.vscode/`: Visual Studio Codeの設定ファイル
- `build/`: ビルドファイル
- `TweliteLibrary/` : Twelite用共有ライブラリ
- `twelite-app.cpp` : Tweliteアプリケーションのエントリーポイント

## ライブラリのimport

1. `TweliteLibrary`を最新版に更新する
```shell
cd TweliteLibrary
git checkout main
git pull
```
2. `build/Makefile`を書き換える
例えば、dps310を使う場合は38行目を以下のように書き換える
```makefile
include $(CURDIR)/../TweliteLibrary/dps310/build.mk
```
3. このディレクトリをVisual Studio Codeで開く

## ライブラリ開発

`feature/xxx`ブランチを切って作業してくれると助かります。
ライブラリが安定してきたら、`main`ブランチに取り込んで、基板用リポジトリのほうもsubmoduleの更新したcommitを取り込んでください。