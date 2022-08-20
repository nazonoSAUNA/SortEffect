# エフェクト並び替えプラグイン
## 動作要件
- Visual C++ 再頒布可能パッケージの2022 x86(32bit)対応の物がインストールされている必要があります【Microsoft Visual C++ 2015-2022 Redistributable(x86)】
- マイクロソフト公式:< https://docs.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist >
- AviUtl解説サイト:< https://scrapbox.io/aviutl/Visual_C++_再頒布可能パッケージ >

- 拡張編集バージョン0.92のみ対応

## 使い方
- AviUtl起動後、プラグインと同じ場所に生成されるSortEffect_Setting.txtで編集します。
- メニューグループを示す制御文字は / 
- グループ化は次のグループが提示されるまで有効
- / のみを書けばグループに属しない状態になります
```
ぼかし
/ぐるーぷ１
縁取り
ライト
/ぐるーぷ２
スクリプト制御
アニメーション効果
/
クロマキー

↓

ぼかし
ぐるーぷ１　＞　縁取り
　　　　　　＞　ライト
ぐるーぷ２　＞　スクリプト制御
　　　　　　＞　アニメーション効果
クロマキー
```

- 設定ファイルの字数制限：1行につき256バイトまで
- 設定ファイルの文字コード：ANSI

- エイリアスが表示されるのは従来と同じ条件です（exedit.aufがあるフォルダと1階層次のフォルダまで）
- エフェクトのエイリアス作成方法↓

![exa条件](https://user-images.githubusercontent.com/99536641/185729389-1ced839f-b82e-4041-8b9a-31be7c785245.png)


- 中身が空のグループは表示されません
- 存在しないエフェクトやエイリアスは表示されません
- 存在するけど設定ファイルに書かれていないエフェクトやエイリアスは自動的にその他グループに追加されます

- 同じグループは1つにまとめられます


## バグ報告・連絡先
Twitter：@nazono22