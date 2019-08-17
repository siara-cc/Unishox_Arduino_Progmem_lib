/*
 * Copyright (C) 2019 Siara Logics (cc)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Arundale R.
 *
 */
#include <Arduino.h>
#include "unishox1_progmem.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

typedef unsigned char byte;

#define NICE_LEN 5

const byte uni_bit_len[5] = {6, 12, 14, 16, 21};
const int32_t uni_adder[5] = {0, 64, 4160, 20544, 86080};

// Decoder is designed for using less memory, not speed
// Decode lookup table for code index and length
// First 2 bits 00, Next 3 bits indicate index of code from 0,
// last 3 bits indicate code length in bits
//                        0,            1,            2,            3,            4,
char vcode1_pgm[32] = {2 + (0 << 3), 3 + (3 << 3), 3 + (1 << 3), 4 + (6 << 3), 0,
//                5,            6,            7,            8, 9, 10
                  4 + (4 << 3), 3 + (2 << 3), 4 + (8 << 3), 0, 0,  0,
//                11,          12, 13,            14, 15
                  4 + (7 << 3), 0,  4 + (5 << 3),  0,  5 + (9 << 3),
//                16, 17, 18, 19, 20, 21, 22, 23
                   0,  0,  0,  0,  0,  0,  0,  0,
//                24, 25, 26, 27, 28, 29, 30, 31
                   0, 0,  0,  0,  0,  0,  0,  5 + (10 << 3)};
//                        0,            1,            2, 3,            4, 5, 6, 7,
char hcode1_pgm[32] = {1 + (1 << 3), 2 + (0 << 3), 0, 3 + (2 << 3), 0, 0, 0, 5 + (3 << 3),
//                8, 9, 10, 11, 12, 13, 14, 15,
                  0, 0,  0,  0,  0,  0,  0,  5 + (5 << 3),
//                16, 17, 18, 19, 20, 21, 22, 23
                   0, 0,  0,  0,  0,  0,  0,  5 + (4 << 3),
//                24, 25, 26, 27, 28, 29, 30, 31
                   0, 0,  0,  0,  0,  0,  0,  5 + (6 << 3)};

enum {SHX_SET1 = 0, SHX_SET1A, SHX_SET1B, SHX_SET2, SHX_SET3, SHX_SET4, SHX_SET4A};
char sets1_pgm[][11] = {{  0, ' ', 'e',   0, 't', 'a', 'o', 'i', 'n', 's', 'r'},
                   {  0, 'l', 'c', 'd', 'h', 'u', 'p', 'm', 'b', 'g', 'w'},
                   {'f', 'y', 'v', 'k', 'q', 'j', 'x', 'z',   0,   0,   0},
                   {  0, '9', '0', '1', '2', '3', '4', '5', '6', '7', '8'},
                   {'.', ',', '-', '/', '=', '+', ' ', '(', ')', '$', '%'},
                   {'&', ';', ':', '<', '>', '*', '"', '{', '}', '[', ']'},
                   {'@', '?', '\'', '^', '#', '_', '!', '\\', '|', '~', '`'}};

int getBitVal(const byte *in, int bit_no, int count) {
   return (pgm_read_byte(in + (bit_no >> 3)) & (0x80 >> (bit_no % 8)) ? 1 << count : 0);
}

int getCodeIdx(char *code_type, const byte *in, int len, int *bit_no_p) {
  int code = 0;
  int count = 0;
  do {
    if (*bit_no_p >= len)
      return 199;
    code += getBitVal(in, *bit_no_p, count);
    (*bit_no_p)++;
    count++;
    if (code_type[code] &&
        (code_type[code] & 0x07) == count) {
      return code_type[code] >> 3;
    }
  } while (count < 5);
  return 1; // skip if code not found
}

int32_t getNumFromBits(const byte *in, int bit_no, int count) {
   int32_t ret = 0;
   while (count--) {
     ret += getBitVal(in, bit_no, count);
     bit_no++;
   }
   return ret;
}

int readCount(const byte *in, int *bit_no_p, int len) {
  const byte bit_len[7]   = {5, 2,  7,   9,  12,   16, 17};
  const uint16_t adder[7] = {4, 0, 36, 164, 676, 4772,  0};
  int idx = getCodeIdx(hcode1_pgm, in, len, bit_no_p);
  if (idx > 6)
    return 0;
  int count = getNumFromBits(in, *bit_no_p, bit_len[idx]) + adder[idx];
  (*bit_no_p) += bit_len[idx];
  return count;
}

int32_t readUnicode(const byte *in, int *bit_no_p, int len) {
  int code = 0;
  for (int i = 0; i < 5; i++) {
    code += getBitVal(in, *bit_no_p, i);
    (*bit_no_p)++;
    //int idx = (code == 0 && i == 1 ? 0 : (code == 2 && i == 1 ? 1 : 
    //            (code == 1 && i == 2 ? 2 : (code == 5 && i == 2 ? 3 :
    //            (code == 3 && i == 2 ? 4 : (code == 7 && i == 3 ? 5 :
    //            (code == 15 && i == 4 ? 6 : 
    //            (code == 31 && i == 4 ? 7 : -1))))))));
    int idx = (code == 0 && i == 0 ? 0 : (code == 1 && i == 1 ? 1 : 
                (code == 3 && i == 2 ? 2 : (code == 7 && i == 3 ? 3 :
                (code == 15 && i == 4 ? 4 : 
                (code == 31 && i == 4 ? 5 : -1))))));
    //printf("%d\n", code);
    //if (idx == 0)
    //  return 0;
    if (idx == 5) {
      int idx = getCodeIdx(hcode1_pgm, in, len, bit_no_p);
      return 0x7FFFFF00 + idx;
    }
    if (idx >= 0) {
      int sign = getBitVal(in, *bit_no_p, 1);
      (*bit_no_p)++;
      int32_t count = getNumFromBits(in, *bit_no_p, uni_bit_len[idx]);
      count += uni_adder[idx];
      (*bit_no_p) += uni_bit_len[idx];
      return sign ? -count : count;
    }
  }
  return 0;
}

void writeUTF8(char *out, int *ol, int32_t uni) {
  int32_t limit11 = 1;
  int32_t limit16 = 1;
  limit11 <<= 11;
  limit16 <<= 16;
  if (uni < limit11) {
    out[(*ol)++] = (0xC0 + (uni >> 6));
    out[(*ol)++] = (0x80 + (uni & 63));
  } else
  if (uni < limit16) {
    out[(*ol)++] = (0xE0 + (uni >> 12));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 63));
    out[(*ol)++] = (0x80 + (uni & 63));
  } else {
    out[(*ol)++] = (0xF0 + (uni >> 18));
    out[(*ol)++] = (0x80 + ((uni >> 12) & 63));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 63));
    out[(*ol)++] = (0x80 + (uni & 63));
  }
  //Serial.print("ol:");
  //Serial.print(uni);
  //Serial.println(*ol);
}

int decodeRepeat(const byte *in, const byte * const in_list[], int idx, int len, char *out, int ol, int *bit_no) {
  int dict_len = readCount(in, bit_no, len) + NICE_LEN;
  int dist = readCount(in, bit_no, len);
  int ctx = readCount(in, bit_no, len);
  //Serial.print("OL:");
  //Serial.print(ol);
  //Serial.print("Dict Len:");
  //Serial.print(dict_len);
  //Serial.print(":Dist:");
  //Serial.print(dist);
  //Serial.print(":Ctx:");
  //Serial.println(ctx);
  if (ctx > 0) {
    unishox1_pgm_decompress(in_list, idx - ctx, out + ol + dict_len, dist + dict_len + 1);
    memcpy(out + ol, out + ol + dict_len + dist, dict_len);
  } else {
    memcpy(out + ol, out + dist, dict_len);
  }
  ol += dict_len;
  return ol;
}

int unishox1_pgm_decompress(const byte * const in_list[], int idx, char *out, int end_len) {

  int dstate;
  int bit_no;
  byte is_all_upper;

  int ol = 0;
  bit_no = 0;
  dstate = SHX_SET1;
  is_all_upper = 0;

  byte *in;
  if (sizeof(byte *) == 2)
    in = (byte *) pgm_read_word(&(in_list[idx]));
  else
    in = (byte *) pgm_read_dword(&(in_list[idx]));
  int len = pgm_read_byte(&(in[0]));
  in++;

  int32_t prev_uni = 0;

  len <<= 3;
  out[ol] = 0;
  while (bit_no < len) {
    if (end_len && ol >= end_len)
      return ol;
    int h, v;
    char c = 0;
    byte is_upper = is_all_upper;
    int orig_bit_no = bit_no;
    v = getCodeIdx(vcode1_pgm, in, len, &bit_no);
    if (v == 199) {
      bit_no = orig_bit_no;
      break;
    }
    h = dstate;
    if (v == 0) {
      h = getCodeIdx(hcode1_pgm, in, len, &bit_no);
      if (h == 199) {
        bit_no = orig_bit_no;
        break;
      }
      if (h == SHX_SET1) {
         if (dstate == SHX_SET1) {
           if (is_all_upper) {
             is_upper = is_all_upper = 0;
             continue;
           }
           v = getCodeIdx(vcode1_pgm, in, len, &bit_no);
           if (v == 199) {
             bit_no = orig_bit_no;
             break;
           }
           if (v == 0) {
              h = getCodeIdx(hcode1_pgm, in, len, &bit_no);
              if (h == 199) {
                bit_no = orig_bit_no;
                break;
              }
              if (h == SHX_SET1) {
                 is_all_upper = 1;
                 continue;
              }
           }
           is_upper = 1;
         } else {
            dstate = SHX_SET1;
            continue;
         }
      } else
      if (h == SHX_SET2) {
         if (dstate == SHX_SET1)
           dstate = SHX_SET2;
         continue;
      }
      if (h != SHX_SET1) {
        v = getCodeIdx(vcode1_pgm, in, len, &bit_no);
        if (v == 199) {
          bit_no = orig_bit_no;
          break;
        }
      }
    }
    if (v == 0 && h == SHX_SET1A) {
      if (is_upper) {
        out[ol++] = readCount(in, &bit_no, len);
      } else {
        ol = decodeRepeat(in, in_list, idx, len, out, ol, &bit_no);
      }
      continue;
    }
    if (h == SHX_SET1 && v == 3) {
      do {
        int32_t delta = readUnicode(in, &bit_no, len);
        if ((delta >> 8) == 0x7FFFFF) {
          int spl_code_idx = delta & 0x000000FF;
          if (spl_code_idx == 2)
            break;
          switch (spl_code_idx) {
            case 1:
              out[ol++] = ' ';
              break;
            case 0:
              ol = decodeRepeat(in, in_list, idx, len, out, ol, &bit_no);
              break;
            case 3:
              out[ol++] = ',';
              break;
            case 4:
              if (prev_uni > 0x3000)
                writeUTF8(out, &ol, 0x3002);
              else
                out[ol++] = '.';
              break;
            case 5:
              out[ol++] = 13;
              break;
            case 6:
              out[ol++] = 10;
          }
        } else {
          prev_uni += delta;
          writeUTF8(out, &ol, prev_uni);
        }
      } while (is_upper);
      //printf("Sign: %d, bitno: %d\n", sign, bit_no);
      //printf("Code: %d\n", prev_uni);
      //printf("BitNo: %d\n", bit_no);
      continue;
    }
    if (h < 64 && v < 32)
      c = sets1_pgm[h][v];
    if (c >= 'a' && c <= 'z') {
      if (is_upper)
        c -= 32;
    } else {
      if (is_upper && dstate == SHX_SET1 && v == 1)
        c = '\t';
      if (h == SHX_SET1B) {
         switch (v) {
           case 9:
             out[ol++] = '\r';
             out[ol++] = '\n';
             continue;
           case 8:
             if (is_upper) { // rpt
               int count = readCount(in, &bit_no, len);
               count += 4;
               char rpt_c = out[ol - 1];
               while (count--)
                 out[ol++] = rpt_c;
             } else {
               out[ol++] = '\n';
             }
             continue;
           case 10:
             continue;
         }
      }
    }
    out[ol++] = c;
  }

  return ol;

}
