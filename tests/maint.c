
#include "libnmea.h"

#define BUFSZ (24)

int main()
{
  char    bufa[BUFSZ],
          bufb[BUFSZ] = {"ABC"};
  size_t  mem = BUFSZ;

  nmeabuf_t  na,
             nb = NMEABUF_INITIALIZER(bufa, mem);

  nmeamsg_t nm = NMEAMSG_INITIALIZER(&na);


  nmea_ctor(&na, bufa, mem);

  if (0 != memcmp(&na, &nb, sizeof(nmeabuf_t))) return -1;

  nmea_ctor(&nb, bufb, mem);

  if (0 == memcmp(&na, &nb, sizeof(nmeabuf_t))) return -1;

  if (nmea_concat(&na, "$CACYC,0,0,0,0,0,0*00\n", BUFSZ+1)) return -1;

  if (!nmea_concat(&na, "$CACYC,0,0,0,0,0,0*00\n", 23)) return -1;

  return 0;
}
