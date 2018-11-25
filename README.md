# tanjunmeikAI - Quoridor Computer Player

## コンパイル方法 
```
$ ./make.sh
```

## 実行方法
### とりあえず戦わせる
```
$ ./judge negascout negascout
```
### とりあえず戦ってみる
```
$ ./judge.sh negascout interactive  # 自分は後手
または
$ ./judge.sh interactive negascout  # 自分は先手

```
人間は後述の出力形式に倣ったコマンドを打ち込むことで戦う。

### 入出力形式

#### 入力
```
(自分の駒のx座標) (自分の駒のy座標) (相手の駒のx座標) (相手の駒のy座標) (自分の残り壁枚数) (相手の残り壁枚数)
(8x8の壁を表す整数列)
```
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

### 単独で動かす
```
$ cd build
$ ./negascout (入力パイプ) (出力パイプ)
```
他のAIも同様

## AIの種類
* negascout: 愚直なnegascout。計算時間は1sとしている。同じ評価値の手はランダムで打つ。
* negascout-deterministic: negascoutとほぼ同じだが、同じ評価値の手は初めて見た手以外は無視することで枝刈りの条件をわずかに厳しくして高速化している。
* interactive: 人間がAIになりきる。


