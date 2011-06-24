
#include "libnmea.h"

#define BUFSZ (48)

int main()
{
  char    bufa[BUFSZ] = {'\0'},
          bufb[BUFSZ];
  size_t  lena = 0,
          mem  = BUFSZ;
  nmeamsg_t  nm;
  nmeabuf_t  nb;


  nmea_ctor(&nb, bufb, mem);

  nmea_concat(&nb, ",2*23\n$CAREV,INIT,0.1.2.3.4*45\n$CA", 34);

  nmea_scan(&nb, &nm);
  nmea_debug(stderr, &nb);

  lena = nmea_parse(bufa, mem, &nm);
  if (0 == lena) return -1;
  if (0 != strncmp(bufa, "$CAREV,INIT,0.1.2.3.4*45\n", lena)) return -1;

  bufa[lena-1]='.';
  printf("len=%ld msg=\"%s\"\n", lena, bufa);

  nmea_scan(&nb, &nm);
  nmea_debug(stderr, &nb);

  return 0;
}
