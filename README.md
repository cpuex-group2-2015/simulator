# Powerless PC Simulator

CPU実験2015

## ビルド
```sh
> make
```

bin/ 以下に生成されます

## 実行

```sh
> ./bin/sim target.bin
```

## テストファイル(test/\*.txt)

test/ 以下にテストファイル(\*.txt)があります。makeすることで同じディレクトリに実行用のバイナリ\*.binが生成されるので、それらをシュミレータで実行することができます。
テストファイルは0, 1で書かれた命令列です。任意の空白と行中のセミコロン以降は無視されます。

## 実行オプション

```
  ___  __ \_  __ \_ |     / /__  ____/__  __ \__  /___  ____/_  ___/_  ___/    ___  __ \_  ____/
  __  /_/ /  / / /_ | /| / /__  __/  __  /_/ /_  / __  __/  _____ \_____ \     __  /_/ /  /
  _  ____// /_/ /__ |/ |/ / _  /___  _  _, _/_  /___  /___  ____/ /____/ /     _  ____// /___
  /_/     \____/ ____/|__/  /_____/  /_/ |_| /_____/_____/  /____/ /____/      /_/     \____/


usage: sim [OPTIONS] FILE.bin
options:

-i               interactive mode
-e <entry_point> set entry_point (default=0)
-d <file_name>   inital data file (default: FILE.data)
-a               disassemble file and exit
-o <file_name>   output file (default: stdout)
-s               report statistics
-l               report f-p instruction log
-f               use x87 f-p instructions instead of PowerlessPC FPU
```

+ -i
	- 対話モード(後述)
+ -e
	- エントリポイントの指定
+ -a
	- ターゲットファイルを逆アセンブルして終了する
+ -o
	- 出力先ファイルを指定(デフォルトで標準出力に出力されます)
+ -s
	- 実行された命令をカウントして表示
+ -l
	- 浮動小数点命令のオペランド・計算結果をファイルにログ(ターゲットファイル名 + 日付.log)
+ -f
	- PowerlessPC FPUの代わりにX87の浮動小数点命令を使用する

recvで読み込む入力は標準入力から与えられます。入力をファイルから読み込みたい場合はcatなどで出力してパイプしてください。

## 対話モード(interactive mode)
```
0x000000> h
interactive mode commands
 p  -- print (ex. p r4 / p ir)
 P  -- print always
 s  -- step into next instruction
 so -- step over into next instruction
 r  -- run
 d  -- disassemble current instruction
 D  -- disassemble always
 b  -- set breakpoint (b 00a123)
 bl -- show breakpoint list
 br -- remove breakpoint
 w  -- watch (same as 'P')
 v  -- view memory data (ex. v r3)
 m  -- dump memory (m [addr] [length])
 q  -- quit
 h  -- help

```

-i オプションをつけることでシミュレータは対話モードになります。
対話モードで使用できるコマンドは以下です:

### p - print 出力

レジスタに格納されたデータ、命令などを出力する。

#### 汎用レジスタ
```
0x012300> p r3
(R3) = 10
```

#### 浮動小数点レジスタ
浮動小数点レジスタはfNまたはfrNで指定する。
```
0x012300> p fr3
(FR3) = 1.700000 (0x3fd9999a)
0x012300> p f3
(FR3) = 1.700000 (0x3fd9999a)
```

#### 命令(16進)
```
0x012300> p ir
(R3) = 01234567
```

#### 条件レジスタ(NGZS)
```
0x012300> p cr
(CR) = 0000
```

#### リンクレジスタ(16進)
```
0x012300> p lr
(LR) = 0x000000
```

#### カウントレジスタ(16進)
```
0x012300> p ctr
(CTR) = 0x000000
```

### P - 出力(常時)
対話モードに移行する度に指定したレジスタの値を出力する。

### s - step ステップ実行
現在のアドレスの命令を実行し、プログラムカウンタを増減させた後再び対話モードに移行する。
```
0x012300> s
0x012304>
```

### so - step over ステップオーバー実行
現在のアドレスの命令を実行する。分岐命令の場合、分岐先の命令群は停止せずに実行し続け、プログラムカウンタが現在のアドレス+1になったところで対話モードに移行する。関数呼び出しをスキップしたいときに使う。

### r - run 実行
ブレークポイントに入るか終了するまで続けて命令を実行する
```
0x012300> r
simulation completed at 0x000090
317418 instructions executed
```

### d - disassemble 逆アセンブル
命令を逆アセンブルして表示する
```
0x012300> d
   0x012300: li    r2, 20
=> 0x012304: mflr  r31
   0x012308: st    r31, 4(r3)
   0x01230c: addi  r3, r3, 8
   0x012310: bl    0x000004
   0x012304: addi  r3, r3, -8
   0x012308: ld    r31, 4(r3)
```

### D - 常時逆アセンブル
対話モードに入る度に逆アセンブルした結果を表示する

### b - breakpoint ブレークポイント
ブレークポイントを設定する
```
0x012300> b
set breakpoint 1 at 0x012300
0x012300> b 12308
set breakpoint 2 at 0x012308
0x012300> b 0x1230a
set breakpoint 3 at 0x01230a
0x012300>
```

### bl - list ブレークポイントの一覧
ブレークポイントの一覧を表示する
```
0x012300> bl
ID TARGET
3  0x01230a
2  0x012308
1  0x012300
```

### br - remove breakpoint ブレークポイントの削除
```
0x012300> br 2
remove breakpoint 2
0x012300> bl
ID TARGET
3  0x01230a
1  0x012300
```

### w - watch
Pと同様

### v - view メモリ上の値を出力(レジスタ指定)
```
0x012300> v r3
MEM[R3] = 0x00001234
```

### m - memory メモリ上の値を出力(アドレス、個数指定)
```
0x012300> m 4 3
0x000004: 4e6e6b28
0x000008: 437f0000
0x00000c: 00000000
```

### q - quit 終了
- q : quit        終了
```
0x012300> q
simulation aborted at 0x012300
4 instructions executed
```

### h - help ヘルプ
- h : help        ヘルプ
