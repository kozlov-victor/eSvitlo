// #include "../src/app/libs/server/v_json_lite/v_json_lite.h"
//
// void setup() {
//
//     Serial.begin(115200);
//     delay(300);
//
//     Serial.println("------------------------------");
//     {
//         JsonObjectBuilder objectBuilder(J_OBJECT);
//         objectBuilder.setBool("key1", true);
//         objectBuilder.setBool("key2", true);
//         auto* obj = objectBuilder.setObject("obj1");
//         objectBuilder.setInt("num1",1, obj);
//         objectBuilder.setNull("nullVal", obj);
//         auto* arr = objectBuilder.setArray("arr", obj);
//         objectBuilder.putBool(true, arr);
//         objectBuilder.putInt(1, arr);
//         objectBuilder.putString("str", arr);
//         objectBuilder.putNull(arr);
//         auto* arr1 = objectBuilder.putArray(arr);
//         objectBuilder.putBool(false, arr1);
//         Serial.println(objectBuilder.toString());
//     }
//
//     {
//         auto* val = JsonParser::parse("{\"obj1\":{\"nullVal\":null,\"arr\":[true,1,\"str\",null,[false]],\"num1\":1.000000},\"key1\":true,\"key2\":true}");
//         Serial.println(val->toString());
//         Serial.println(val->get("obj1")->toString());
//         Serial.println(val->get("obj1")->get("arr")->get(0)->toString()); // true
//         Serial.println(val->get("obj1")->get("arr")->get(1)->toString()); // 1
//     }
// }
//
// void loop() {
//
// }
// #include <Arduino.h>
//
//
// // #define PIN_DATA  3
// // #define PIN_CLK   4
// // #define PIN_CS    5
// //
// // void sendRaw(uint8_t data) {
// //     for (int i = 0; i < 8; i++) {
// //         digitalWrite(PIN_CLK, LOW);
// //
// //         digitalWrite(PIN_DATA, (data & 0x80) ? HIGH : LOW);
// //         data <<= 1;
// //
// //         digitalWrite(PIN_CLK, HIGH);
// //     }
// // }
// //
// // void sendCmd(uint8_t cmd) {
// //     digitalWrite(PIN_CS, LOW);
// //
// //     sendRaw(0xF8);                  // command
// //     sendRaw(cmd & 0xF0);
// //     sendRaw((cmd << 4) & 0xF0);
// //
// //     digitalWrite(PIN_CS, HIGH);
// // }
// //
// // void sendData(uint8_t data) {
// //     digitalWrite(PIN_CS, LOW);
// //
// //     sendRaw(0xFA);                  // data (0xF8 + 0x02)
// //     sendRaw(data & 0xF0);
// //     sendRaw((data << 4) & 0xF0);
// //
// //     digitalWrite(PIN_CS, HIGH);
// // }
// //
// // void initLCD() {
// //     delay(100);
// //
// //     sendCmd(0x30); // basic
// //     delay(10);
// //
// //     sendCmd(0x30);
// //     delay(1);
// //
// //     sendCmd(0x0C); // display ON
// //     sendCmd(0x01); // clear
// //     delay(10);
// //
// //     sendCmd(0x34); // extended mode
// //     sendCmd(0x36); // graphics ON
// // }
// //
// // void setAddress(uint8_t y, uint8_t x) {
// //     if (y < 32) {
// //         sendCmd(0x80 | y);        // row
// //         sendCmd(0x80 | x);        // column
// //     } else {
// //         sendCmd(0x80 | (y - 32));
// //         sendCmd(0x80 | (x + 8));  // друга половина
// //     }
// // }
// //
// // void drawPixel(int x, int y) {
// //     uint8_t col = x / 8;
// //     uint8_t bit = 7 - (x % 8);
// //
// //     setAddress(y, col);
// //
// //     sendData(1 << bit); // тільки один піксель
// // }
// //
// // void testFill() {
// //     for (int y = 0; y < 64; y++) {
// //         setAddress(y, 0);
// //
// //         for (int x = 0; x < 16; x++) {
// //             sendData(0xFF); // всі пікселі ON
// //         }
// //     }
// // }
// //
// // void setup() {
// //
// //     Serial.begin(115200);
// //     delay(300);
// //     Serial.println("------init------");
// //
// //     pinMode(PIN_DATA, OUTPUT);
// //     pinMode(PIN_CLK, OUTPUT);
// //     pinMode(PIN_CS, OUTPUT);
// //
// //     digitalWrite(PIN_CS, HIGH);
// //
// //     initLCD();
// //     delay(100);
// //
// //     // тест — піксель по центру
// //     //drawPixel(64, 32);
// //     testFill();
// // }
// //
// // void loop() {
// //   drawPixel(64, 32);
// //
// // }