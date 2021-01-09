BME280 温度センサー C/C++ 実装コード

本プログラムは BME280 温度・気圧センサーを C/C++ コードで読み取るものです。
このソフトウェアは、 MIT ライセンスのもとで公開します。

## 背景

WritingPi が開発終了になったため、 C/C++でのセンサー読み取りを行うために作成しました。
本プログラムは pigpio ライブラリを使用し、非 root で動作するようにしています。


## 依存情報

pigpio ライブラリを使用しています。
- http://abyz.me.uk/rpi/pigpio/

```bash
$ sudo apt install pigpio
```

## 使用例

サンプルプログラムを用意しています(sample_bme280.cpp)。
こちらを参考にしてください。
また事前に pigpiod サービスの稼働が必要です。

```bash
$ sudo pigpiod
```

## 動作環境

- Raspberry Pi OS (2020/12/02版)
- Raspberry Pi 4 (4GB/8GB版)

## 参考情報

- https://github.com/SWITCHSCIENCE/BME280
- https://akizukidenshi.com/download/ds/bosch/BST-BME280_DS001-10.pdf