/*!
 * \file libnmea.c
 * \author jon <jon@wroth.org>
 * \date september 2009
 * \brief library for manipulating streams of NMEA strings.
 */

/*!
 * \mainpage libnmea: NMEA stream library
 *
 * you can use this library to manage NMEA streams from a device, however you
 * happen to get them. use nmea_concat to add any new input to the buffer
 * manager, partial or not. then use nmea_scan and nmea_parse to trigger your
 * event, foo, on the presence of a message boundary. here's an example.
 *
 * \code
     nmeamsg_t  msg;
     nmeabuf_t  nbuf;
     char       src[SRCSZ];
     nmea_ctor(&nbuf, src, SRCSZ);
     while (1)
       if (poll(fd, ...))
         if (rxlen = read(fd, rxbuf, ...))
           if (nmea_concat(&nbuf, rxbuf, (size_t)rxlen))
             while (nmea_scan(&nbuf, &msg))
             {
               txlen = nmea_parse(txbuf, BUFSZ, &msg);
               foo(txbuf, txlen);
             }
 * \endcode
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "libnmea.h"


static void nmeabuf_shift(nmeabuf_t *buffer)
{
  size_t  off;
  switch(buffer->state)
  {
    case NMEA_S_CLEAR: off = buffer->index; break;
    case NMEA_S_MARK:  off = buffer->start; break;
    default: return;
  }
  buffer->start   = 0;
  buffer->index  -= off;
  buffer->length -= off;
  if (buffer->length) memmove(buffer->buffer, buffer->buffer + off, sizeof(char) * buffer->length);
}


static void ckstep(int *ret, int *sta, long *calcsum, char *pos)
{
  switch(*sta)
  {
    case 1:
      *sta = (*pos == '$') ? 2 : -1;
      break;

    case 2:
      if (*pos == '*') *sta = 3;
      else *calcsum ^= *pos;
      break;

    case 3:
    {
      long checksum = strtol(pos, (char **)NULL, 16);
      if (0 > checksum)
      {
        perror("strtol");
        return;
      }
      *sta = 0;
      *ret = (*calcsum == checksum) ? 0 : -1;
      break;
    }
    default:
      break;
  }
}


size_t circindex(size_t idx, size_t inc, size_t len)
{
  register size_t i=idx+inc;
  return (i >= len) ? (i % len) : i;
}


unsigned char dtox(unsigned char c)
{
  c &= 0xf;
  if (c < 10) return c+'0';
  if (c < 16) return c-10+'A';
  return '.';
}


unsigned char xtod(unsigned char c)
{
  if (!isxdigit(c)) return 0;
  if (c < 'A') return c-'0';
  if (c < 'a') return c-'A'+10;
  return c-'a'+10;
}


/*!
 */
void nmea_ctor(nmeabuf_t *buffer, const char *source, const size_t maxlen)
{
  buffer->buffer = (char *)source;
  buffer->alloc  = maxlen;
  nmea_reset(buffer);
}


/*!
 */
void nmea_reset(nmeabuf_t *buffer)
{
  memset(buffer->buffer, '\0', sizeof(char) * buffer->alloc);
  buffer->length = 0;
  buffer->index  = 0;
  buffer->start  = 0;
  buffer->state  = NMEA_S_CLEAR;
}


/*!
 */
int nmea_concat(nmeabuf_t *buffer, const char *rbuf, const size_t rlen)
{
  nmeabuf_shift(buffer);

  if (!rlen) return 0;

  if (rlen > (buffer->alloc - buffer->length)) return 0;

  memcpy(buffer->buffer + buffer->length, rbuf, sizeof(char) * rlen);

  buffer->length += rlen;

  return 1;
}


/*!
 */
int nmea_scan(nmeabuf_t *buffer, nmeamsg_t *message)
{
  size_t  i = buffer->index;
  int     ret = 0;

  while ((i < buffer->length) && (0 == ret))
  {
    switch(buffer->state)
    {
      case NMEA_S_CLEAR:
        if ('$' == buffer->buffer[i])
        {
          buffer->start = i;
          buffer->state = NMEA_S_MARK;
        }
        break;

      case NMEA_S_MARK:
        if ('\n' == buffer->buffer[i])
        {
          if (message)
          {
            message->nmeabuf = buffer;
            message->start   = buffer->start;
            message->length  = i - buffer->start + 1;
          }
          buffer->state = NMEA_S_CLEAR;
          ret = 1;
        }
        break;
    }
    i++;
  }
  buffer->index = i;
  return ret;
}


/*!
 */
size_t nmea_peek(char *wbuf, const size_t maxlen, nmeamsg_t *message)
{
  if (message->length > maxlen) return 0;
  memcpy(wbuf, message->nmeabuf->buffer + message->start, sizeof(char) * message->length);
  return message->length;
}


/*!
 */
size_t nmea_parse(char *wbuf, const size_t maxlen, nmeamsg_t *message)
{
  size_t len = nmea_peek(wbuf, maxlen, message);
  nmeabuf_shift(message->nmeabuf);
  return len;
}


/*!
 */
int nmea_cksum(const char *nmeasz, long *cksum)
{
  char *  end      = NULL,
       *  start    = NULL;
  long    calcsum  = 0,
          checksum = 0;

  /* get start/end of sentence
   * verify they're in order
   */
  start = strchr(nmeasz, '$');
  if (NULL == start) return -1;
  end = strchr(start, '*');
  if (NULL == end) return -1;

  checksum = strtol(end+1, (char **)NULL, 16);
  if (0 > checksum)
  {
    perror("strtol");
    return -1;
  }

  start++;                                    /* begin after '$' */
  while (start < end) calcsum ^= *(start++);  /* calculate */
  if (NULL != cksum) *cksum = calcsum;        /* store calculated checksum */
  return (calcsum == checksum) ? 0 : -1;      /* test, return */
}


/*!
 */
int nmea_cksum_msg(nmeamsg_t *message, long *cksum)
{
  int ckret = -1;
  int cksta = 1;
  long calcsum = 0;
  size_t i = 0;
  size_t j = circindex(message->start, 0, message->nmeabuf->alloc);
  while ((cksta > 0) && (i < message->length))
  {
    ckstep(&ckret, &cksta, &calcsum, message->nmeabuf->buffer+j);
    j = circindex(j, 1, message->nmeabuf->alloc);
    i++;
  }
  if (NULL != cksum) *cksum = calcsum;
  return ckret;
}


/*!
 */
size_t nmea_hexencode(char *wbuf, size_t maxlen, const char *rbuf, size_t rlen)
{
  size_t wlen = rlen << 1;
  char *wp;
  maxlen &= ~0x1;
  if (wlen > maxlen) wlen = maxlen;
  wp = wbuf+wlen-1;
  rbuf += rlen-1;
  while (wp > wbuf) /* who will CPSR&#1<<28 with me? not i, said the JGE. */
  {
    *wp-- = dtox(*rbuf);
    *wp-- = dtox(*rbuf-- >> 4);
  }
  return wlen;
}


/*!
 */
size_t nmea_hexdecode(char *wbuf, size_t maxlen, const char *rbuf, size_t rlen)
{
  size_t wlen = rlen >> 1;
  char *rstp;
  if (wlen > maxlen) wlen = maxlen;
  rstp = (char *)rbuf+(wlen<<1);
  while (rbuf < rstp) /* he did it again! */
  {
    *wbuf    = 0xf0 & (xtod(*rbuf++) << 4);
    *wbuf++ |= 0x0f &  xtod(*rbuf++);
  }
  return wlen;
}


/*!
 */
void nmea_debug(FILE *stream, const nmeabuf_t *buffer)
{
  size_t i;

  for (i = 0; i < buffer->length; i++)
    fprintf(stream, "%c", isprint(buffer->buffer[i]) ? buffer->buffer[i] : '.');
  fprintf(stream, "\n");

  for (i = 0; i <= buffer->length; i++)
    if (i == buffer->start)
    {
      if (i == buffer->index)
        fprintf(stream, "X");
      else
        fprintf(stream, "S");
    }
    else
    {
      if (i == buffer->index)
        fprintf(stream, "I");
      else
        fprintf(stream, " ");
    }
  fprintf(stream, "\n");
}
