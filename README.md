
## 依存関係
```
$ sudo apt install libboost1.65-dev libboost-system1.65-dev cmake
```
## コンパイル方法 
```
$ ./make.sh
```

## 入出力形式
AIの入出力は標準入出力で行う。


#### 入力
```
(自分の駒のx座標) (自分の駒のy座標) (相手の駒のx座標) (相手の駒のy座標) (自分の残り壁枚数) (相手の残り壁枚数)
(8x8の壁を表す整数列)
```
座標は全て左上隅が(0,0)、右上隅が(8,0)、左下隅が(0,8)となるような座標で表す。自分は常に下から上に向かい、相手は上から下に向かう。
壁を表す整数列は、
1: そのマスの下&右下にまたがる横の壁がある
2: そのマスの右&右下にまたがる縦の壁がある
0: どちらもない

#### 出力

```
MOVE (移動先のx座標) (移動先のy座標) # 動く
または
WALL H (置く位置のx座標) (置く位置のy座標) # 横の壁を置く
または
WALL V (置く位置のx座標) (置く位置のy座標) # 縦の壁を置く
```

壁を置く時の座標は、置きたい壁の位置の上下(水平な壁の場合)あるいは左右(垂直な壁の場合)の4マスのうち、左上のマスの座標とする。


## 実行方法
### ローカルでjudgeを使う
```
$ ./judge.sh (先手のAIプログラム) (後手のAIプログラム)
```

例えば、"negascout"同士で戦わせるには、
```
$ ./judge.sh negascout negascout
```

それぞれのAIとjudgeの標準エラー出力を別ターミナルに分けたい場合は、次のようなコマンドを別のターミナルで実行する。
```
$ cd build
$ ./judge output-pipe1 input-pipe1 output-pipe2 input-pipe2
```
```
$ cd build
$ (先手のAIプログラム) < input-pipe1 > output-pipe1
```
```
$ cd build
$ (後手のAIプログラム) < input-pipe2 > output-pipe2
```

### CUIで人間と戦う
"interactive"というプログラムを使う。例えば人間が後手にしたいときは、別々のターミナルで
```
$ cd build
$ ./judge output-pipe1 input-pipe1 output-pipe2 input-pipe2 -s
```
```
$ cd build
$ ./(先手のAIプログラム) < input-pipe1 > output-pipe1
```
```
$ cd build
$ ./interactive input-pipe2 output-pipe2
```
というようにすればいい。ただし、"interactive"は標準入力からユーザーのコマンドを受けるので注意。パイプは代わりにコマンドライン引数で指定する。
interactiveからはUNDOコマンドで二手戻すことができる。
judgeに-sを渡すと、勝敗が決まってもjudgeが終了せず、UNDOを待つ。

### ジャッジサーバーに接続する
AIをジャッジサーバーに接続するには、次のようにする。
```
$ ./connect.sh (ジャッジサーバーのIPアドレス) (AIプログラム)
```
もしくは、明示的に次のように実行する。
```
$ cd build
$ ./tcp-pipe (ジャッジサーバーのIPアドレス) > input-pipe1 < output-pipe1 & 
$ ./(あなたのAIプログラム) < input-pipe1 > output-pipe1
```

ジャッジサーバーでの入出力形式はここで使われているものと若干異なるので、tcp-pipe内でその変換を行っている。

## 付属AIの種類
* negascout: 愚直なnegascout。計算時間は1sとしている。同じ評価値の手はランダムで打つ。
* negascout-deterministic: negascoutとほぼ同じだが、同じ評価値の手は初めて見た手以外は無視することで枝刈りの条件をわずかに厳しくして高速化している。
* negascout-fast: 工夫して速くしたnegascout。同じ評価値の手はランダムで打つが、評価関数が荒いのか、ランダムにすると極めて弱くなる。
* negascout-fast-deterministic: negascout-fastと同じだが、初めて見立て以外は無視する。なぜかdetreministicじゃないやつより強い。
* interactive: 人間がAIになりきる。





