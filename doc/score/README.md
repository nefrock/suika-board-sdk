# 打ち込み方法

1.  score.xlsx を Excel で開く
1.  音名、長さの列を修正し曲を作る。音名については[こちら](https://tomari.org/main/java/oto.html)を参考にしてください。注意点として#は S で表します。例えば D#1 は DS1 のように記述します。また休符は N を用いて表します。
1.  J1 セルで BPM を、J4 セルで曲名(英数字のみ)を設定
1.  K4 のセルにある arr=[(....の値をコピーし soundtest.py の 7 行目に貼り付ける
1.  python3 soundtest.py で実行し曲を確認する(実行には pip3 install synthesizer が必要)
1.  K5 セルにある int ...の値をコピーし src/pitches.h の当該箇所に貼り付ける
1.  [こちら](../installation/)を参考にして Suika-Board に書き込む
