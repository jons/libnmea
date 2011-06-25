#include <stdio.h>
#include <string.h>
#include "libnmea.h"


void dbprintf(char *buf, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    printf("%c", isprint(buf[i]) ? buf[i] : '.');
  putchar(' ');
  for (i = 0; i < len; i++)
    printf("%c%02hhx", i?' ':'[',buf[i]);
  printf("]\n");
}


int main()
{
  char a, b, mem[32] = {'\0'};
  const char pkt[32] = {0x01, 0x02, 0xe4, 0x08, 0x0a, 0x0c, 0x0f, 0xf0 },
             hex[32] = {"0102E4080A0C0FF0"};

  size_t len;

  // note: dtox/hexencode produces capital A-F, don't test a-f


  //xtod

  if ((xtod('0')   != 0x0) ||
      (xtod('9')   != 0x9) ||
      (xtod('A')   != 0xa) ||
      (xtod('F')   != 0xf) ||
      (xtod('F'+1) != 0x0) ||
      (xtod('0'-1) != 0x0)) return -1;

  printf("xtod: %s ", hex);
  for (len = 0; len < 16; len++)
    printf("%1hhx%c", xtod(hex[len]), (len < 15) ? ' ' : '\n');

  // dtox

  if ((dtox(0)   != '0') ||
      (dtox(0xa) != 'A') ||
      (dtox(20)  != '4') ||
      (dtox(-1)  != 'F')) return -1;

  printf("dtox: ");
  for (len = 0; len < 8; len++)
    printf("%c%c%c", dtox(pkt[len] >> 4), dtox(pkt[len]), (len < 7) ? ' ' : '\n');

  // encode
  memset(mem, 0, sizeof(mem));
  a=mem[0];b=mem[17];
  len = nmea_hexencode(mem+1, 16, pkt, 8);
  if (len != 16) return -1;
  dbprintf(mem, len+2);
  if (memcmp(mem+1, hex, len)) return -1;
  if (mem[0]!=a || mem[len+1]!=b) return -1;
  printf("nmea_hexencode ok\n");

  // decode

  memset(mem, 0, sizeof(mem));
  a=mem[0];b=mem[9];
  len = nmea_hexdecode(mem+1, 8, hex, 16);
  if (len != 8) return -1;
  dbprintf(mem, len+2);
  if (memcmp(mem+1, pkt, len)) return -1;
  if (mem[0]!=a || mem[len+1]!=b) return -1;
  printf("nmea_hexdecode ok\n");

  // test in-place encoder/decoder
  memset(mem, 0, sizeof(mem));
  memcpy(mem+1, pkt, 8);
  a=mem[0];b=mem[17];
  len = nmea_hexencode(mem+1, 16, mem+1, 8);
  dbprintf(mem, len+2);
  if (len != 16) return -1;
  if (memcmp(mem+1, hex, len)) return -1;
  if (mem[0]!=a || mem[len+1]!=b) return -1;
  printf("nmea_hexencode in-place ok\n");

  // test maxlen too

  a=mem[0];b=mem[5];
  len = nmea_hexdecode(mem+1, 4, mem+1, 16);
  dbprintf(mem, len+2);
  if (len != 4) return -1;
  printf("nmea_hexdecode len ok\n");
  if (memcmp(mem+1, pkt, len)) return -1;
  printf("nmea_hexdecode memory ok\n");
  if (mem[0]!=a || mem[len+1]!=b) return -1;
  printf("nmea_hexdecode in-place ok\n");


  return 0;
}
