# Mappingcon
周囲の障害物をマッピングするラジコンです。Bluetoothを用いた無線通信により、ラジコン→PCへセンサデータを、PC→ラジコンへモータ制御用のデータを送信します。

mappingcon/mappingcon.inoをArduinoへ書き込み、main_pc/main_pc.pdeをPC側で起動することで実行可能です。

使用しているセンサは6軸センサ(加速度+ジャイロ)、距離センサ(超音波式)、温度・湿度センサであり、6軸センサの情報からラジコンの位置と方向を、距離センサと温度・湿度センサの情報から障害物までの距離を推定し、
PC側クライアントソフトウェアに障害物の位置を描画します。

PC側クライアントソフトウェアは障害物及びラジコンの位置描画と、キーボードからの入力(操作)情報の送信、ラジコンからのセンサデータの受信をおこないます。ラジコンの操作はWASDキーを用い、WとSで前後進、ADキーのどちらかを押しながらWSキーで左(右)前(後)方向へ移動、
ADキーを同時押ししながらWSキーでゆっくりと前後進することができます。ただしラジコンの起動後しばらくは6軸センサのオフセット処理のためPC側は何も操作しないでください。

またデバッグのためラジコン側にはドットマトリクスLEDを搭載しており、現在PC側から受信している命令を表示することができます。

![discription](https://github.com/saikiRA1011/Mappingcon/blob/main/image/description.png "説明")

## 回路図等
![diagram_top](https://github.com/saikiRA1011/Mappingcon/blob/main/image/diagram_top.png "回路図上側")  
回路図上側

![diagram_bottom](https://github.com/saikiRA1011/Mappingcon/blob/main/image/diagram_top.png "回路図下側")  
回路図下側

![case](https://github.com/saikiRA1011/Mappingcon/blob/main/image/case.jpg "ケース実装例")  
ケース実装例

![circuit](https://github.com/saikiRA1011/Mappingcon/blob/main/image/circuit.jpg "表面実装例")
表面実装の例
