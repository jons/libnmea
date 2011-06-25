#include <stdio.h>
#include "libnmea.h"

#define BUFSZ (256)

int main()
{
  long      cks, ckr;
  int       ret;
  char      buf[BUFSZ];
  size_t    mem = BUFSZ;
  nmeabuf_t na  = NMEABUF_INITIALIZER(buf, mem);
  nmeamsg_t nm;

  ckr = 'T'^'E'^'S'^'T'^','^'6';
  ret = nmea_cksum("$TEST,6*0C\r\n", &cks);
  printf("expected: %02hhx calculated: %02hhx\n", ckr, cks);
  if (ret) return -1;
  if (cks != ckr) return -1;

  ckr = 'C'^'A'^'C'^'Y'^'C'^','^'0'^','^'1'^','^'2'^','^'2'^','^'1'^','^'0'^',';
  nmea_concat(&na, "$CACYC,0,1,2", 12);
  nmea_concat(&na, ",2,1,0,*77\n", 11);
  if (nmea_scan(&na, &nm))
  {
    ret = nmea_cksum_msg(&nm, &cks);
    printf("expected: %02hhx calculated: %02hhx\n", ckr, cks);
    if (ret) return -1;
    if (cks != ckr) return -1;
  }

  return 0;
}
