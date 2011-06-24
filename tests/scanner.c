
#include "libnmea.h"

#define BUFSZ (48)

int main()
{
  char    bufa[BUFSZ],
          bufb[BUFSZ];
  size_t  mem = BUFSZ;
  nmeamsg_t  nm;
  nmeabuf_t  na,
             nb = NMEABUF_INITIALIZER(bufa, mem);


  nmea_ctor(&na, bufa, mem);
  nmea_ctor(&nb, bufb, mem);

  nmea_concat(&na, "$CACYC,0,1,2", 12);
  nmea_debug(stderr, &na);

  if (nmea_scan(&na, NULL)) return -1;
  nmea_debug(stderr, &na);

  nmea_concat(&na, ",2,1,0,*00\n", 11);
  nmea_debug(stderr, &na);

  if (!nmea_scan(&na, NULL)) return -1;
  nmea_debug(stderr, &na);

  nmea_concat(&nb, ",2*23\n$CARE", 11);
  nmea_debug(stderr, &nb);

  if (nmea_scan(&nb, NULL)) return -1;
  nmea_debug(stderr, &nb);

  nmea_concat(&nb, "V,INIT,0.1.2.3.4*45\n", 20);
  nmea_debug(stderr, &nb);

  if (!nmea_scan(&nb, &nm)) return -1;
  nmea_debug(stderr, &nb);

  if (nmea_scan(&nb, NULL)) return -1;
  if (nmea_scan(&nb, NULL)) return -1;
  if (nmea_scan(&nb, NULL)) return -1;
  if (nmea_scan(&nb, NULL)) return -1;
  
  return 0;
}
