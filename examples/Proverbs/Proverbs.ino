/*
  This example demonstrates de-compression of compressed proverbs
  Source: Wikipedia
  Decompresses data from Proverbs_Uni.h, which was generated using Unishox compression utility
  found at https://github.com/siara-cc/Unishox
  Command to generate Html.h: `./unishox1 -g Proverbs_Uni.sample Proverbs_Uni

  Original size : 4459 bytes
  Compressed size: 2793 bytes
  Savings: 37.36%

  How Unishox works:
  https://github.com/siara-cc/Unishox/blob/master/Unishox_Article_1.pdf?raw=true

  Other projects using Unishox:
  Compression library for Arduino - https://github.com/siara-cc/Unishox_Arduino_lib
  As SQLite loadable extension - https://github.com/siara-cc/Unishox_Sqlite_UDF
  Sqlite3 Library for ESP32 - https://github.com/siara-cc/esp32_arduino_sqlite3_lib
  Sqlite3 Library for ESP8266 - https://github.com/siara-cc/esp_arduino_sqlite3_lib
  Sqlite3 Library for ESP-IDF - https://github.com/siara-cc/esp32-idf-sqlite3
*/
#include "unishox1_progmem.h"
#include "Proverbs_Uni.h"

void setup() {

  Serial.begin(115200);
  randomSeed(analogRead(0));

}

void loop() {

  // buffer for decompression
  // since -G option was used to generate Proverbs.h, the buffer size has to be
  // atleast Proverbs0_2_max_len * 2).
  // The call to unishox1_pgm_decompress() can be kept inside a function with this buffer
  // to release it immediately.
  char out[Proverbs_Uni_max_len * 2];

  Serial.write("Welcome\n");
  Serial.write("-------\n");

  Serial.write("1. Print all Proverbs\n");
  Serial.write("2. Print random proverb\n\n");
  Serial.write("Enter choice:\n");
  int ch = 0;
  while (true) {
      if (Serial.available()) {
          ch = Serial.read();
          if (ch == '1' || ch == '2')
            break;
      }
  }

  if (ch == '1') {
    for (int i=0; i < Proverbs_Uni_line_count; i++) {
        int len = unishox1_pgm_decompress(Proverbs_Uni, i, out, 0);
        out[len] = 0;
        Serial.write(out);
        Serial.write("\n");
    }
  } else if (ch == '2') {
    int len = unishox1_pgm_decompress(Proverbs_Uni, random(0, Proverbs_Uni_line_count - 1), out, 0);
    out[len] = 0;
    Serial.write(out);
    Serial.write("\n\n");
  }

}
