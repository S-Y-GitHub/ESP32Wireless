# ESP32Wireless

Wi-Fiを使用して、esp32同士やesp32とPC間で通信を行うためのライブラリです。  
データ型とデータをセットで送受信することで、数値や文字列、配列などを簡単に扱うことができます。
PCではPythonまたはJavaを使用して通信を行うことができます。

# Usage

## include
スケッチと同じディレクトリに Wireless.hpp, Wireless.cpp, Data.hpp, Data.cpp を配置してください。
Wireless.hpp をインクルードすることで使用することができます。
```C++
#include "Wireless.hpp"
```

## Data
Dataクラスはデータ型とデータをセットにするためのものです。  
データ型はDataType列挙型で表され、typeメンバ関数を呼び出すことで取得できます。  
データ型には以下の種類があります。
| データ型 | Arduino言語でのデータ型 | データ              |
| -------- | ----------------------- | ------------------- |
| Null     | std::nullptr_t          | データなし          |
| Bool     | bool                    | 真理値              |
| String   | String                  | 文字列              |
| Array    | std::vector<Data>       | 配列                |
| Int8     | int8_t                  | 符号付き8bit整数値  |
| Int16    | int16_t                 | 符号付き16bit整数値 |
| Int32    | int32_t                 | 符号付き32bit整数値 |
| Int64    | int64_t                 | 符号付き64bit整数値 |
| UInt8    | uint8_t                 | 符号なし8bit整数値  |
| UInt16   | uint16_t                | 符号なし16bit整数値 |
| UInt32   | uint32_t                | 符号なし32bit整数値 |
| UInt64   | uint64_t                | 符号なし64bit整数値 |

データはキャスト演算によって取り出すことができます。  
変数に代入すると自動的にキャスト演算がおこなわれるため値を取り出すことができます。

## Wi-Fiアクセスポイントの作成
Wi-Fiアクセスポイントを作成するためには、wlAPBegin関数をSSIDとパスワードを渡して呼び出す必要があります。  
デフォルトではIPアドレスとデフォルトゲートウェイは 192.168.1.1 サブネットマスクは 255.255.255.0 に設定されます。  
パスワードに続いてこれらを引数として渡すことで設定できます。
```C++
static const char SSID[] = "yourssid";
static const char PASSWORD[] = "yourpassword";
// static const IPAddress GATEWAY(192, 168, 1, 1);
// static const IPAddress SUBNET(255, 255, 255, 0);
void setup() {
    wlAPBegin(SSID, PASSWORD);
    // wlAPBegin(SSID, PASSWORD, GATEWAY, SUBNET);
}
```

## Wi-Fiへの接続
Wi-Fiに接続するには、wlConnect関数をSSID、パスワード、IPアドレスを渡して呼び出す必要があります。  
IPアドレスは同じネットワークないで重複しないように設定する必要があります。（特にesp32を使用して作成したアクセスポイントではなく、既存のアクセスポイントを使用する場合は注意してください。）  
デフォルトではIPアドレスとデフォルトゲートウェイは 192.168.1.1 サブネットマスクは 255.255.255.0 に設定されます。  
IPアドレスに続いてこれらを引数として渡すことで設定できます。
（既存のアクセスポイントを使用する場合はこれらの値も適切に設定する必要があります。）
```C++
static const char SSID[] = "yourssid";
static const char PASSWORD[] = "yourpassword";
static const IPAddress IP(192, 168, 1, 2);
// static const IPAddress GATEWAY(192, 168, 1, 1);
// static const IPAddress SUBNET(255, 255, 255, 0);
void setup() {
    wlConnect(SSID, PASSWORD, IP);
    // wlConnect(SSID, PASSWORD, IP, GATEWAY, SUBNET);
}
```

## 送信チャンネルの設定
送信チャンネルを設定するには、wlTxAttach関数を送信先のIPアドレスとポート番号を渡して呼び出す必要があります。  
デフォルトではチャンネル0に設定されます。複数のチャンネルを使用する場合は、ポート番号に続いてチャンネルを引数として渡すことで設定できます。  
既存のアクセスポイントを使用する際は、ポート番号の重複を防ぐため、システムポート番号（0 ~ 1023）やユーザーポート番号（1024 ~ 49151）を避け動的・使用ポート（49152 ~ 65535）を使用することを推奨します。
```C++
void setup() {
    // TODO アクセスポイントの作成または接続

    wlTxAttach(IPAddress(192, 168, 1, 2), 50000);
    // wlTxAttach(IPAddress(192, 168, 1, 2), 50000, 0);
}
```

## 受信チャンネルの設定
受信チャンネルを設定するには、wlRxAttach関数を受信するポート番号を渡して呼び出す必要があります。  
デフォルトではチャンネル0に設定されます。複数のチャンネルを使用する場合は、ポート番号に続いてチャンネルを引数として渡すことで設定できます。  
既存のアクセスポイントを使用する際は、ポート番号の重複を防ぐため、システムポート番号（0 ~ 1023）やユーザーポート番号（1024 ~ 49151）を避け動的・使用ポート（49152 ~ 65535）を使用することを推奨します。
```C++
void setup() {
    // TODO アクセスポイントの作成または接続

    wlRxAttach(50000);
    // wlRxAttach(50000, 0);
}
```

## データの送信
データを送信する場合はwlTxWrite関数をデータを渡して呼び出す必要があります。  
デフォルトではチャンネル0に送信されます。データに続いてチャンネルを引数として渡すことで変更できます。
文字列や配列（`std::vector<Data>`）はそのまま渡すことができます。  
整数型はデータ型を明確にするため、キャスト演算を使用することを推奨します。
```C++
wlTxWrite("Hello");
// wlTxWrite("Hello", 0);
wlTxWrite(std::vector<Data>{"Hello", static_cast<uint8_t>(1), static_cast<int32_t>(2), static_cast<uint64_t>(3)});
// wlTxWrite(std::vector<Data>{"Hello", 1, 2, 3});
wlTxWrite(static_cast<uint32_t>(0));
// wlTxWrite(static_cast<uint32_t>(0), 0);
```

## データの受信
データを受信する場合は、まずwlRxAvailable関数を呼び出して取り出すことができるデータ数を確認します。  
デフォルトではチャンネル0のデータ数を取得します。チャンネルを引数として渡すことによって変更できます。  
データがある場合はwlRxRead関数を呼び出してデータを取り出します。  
デフォルトではチャンネル0のデータを取り出します。チャンネルを引数として渡すことで変更できます。
データ型を明確にするため変数に格納するか、キャスト演算を使用することを推奨します。
```C++
void loop() {
    if (wlRxAvailable() > 0) {
        String msg = wlRxRead();
    }
    /*
    if (wlRxAvailable(0) > 0) {
        String msg = wlRxRead(0);
    }
    */
}
```
