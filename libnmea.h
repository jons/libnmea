/*!
 * \file libnmea.h
 * \author jon <jon@wroth.org>
 * \date september 2009
 * \brief functions for manipulating streams of NMEA sentences.
 */

#ifndef LIBNMEA_H
#define LIBNMEA_H

#include <stdlib.h>
#include <stdio.h>


/* nmea scanner states
 */
#define NMEA_S_CLEAR  (0)   /*!< nmeabuf::start points to junk, nmeabuf::index points to next char to scan or end of buffer */
#define NMEA_S_MARK   (1)   /*!< nmeabuf::start points to '$' at start of a message */


#define NMEABUF_INITIALIZER(sbuf, sasz) \
  { .buffer = sbuf, \
    .alloc  = sasz, \
    .length = 0, \
    .index  = 0, \
    .start  = 0, \
    .state  = NMEA_S_CLEAR }


#define NMEAMSG_INITIALIZER(nbuf) \
  { .nmeabuf = nbuf, \
    .start   = 0, \
    .length  = 0 }


/*!
 * \brief a buffer which will be populated with NMEA sentences using
 * nmea_concat(). sentences can be found with nmea_scan() and removed with
 * nmea_parse().
 */
typedef struct nmeabuf
{
  char *  buffer;  /*!< source buffer where input from NMEA device goes */
  size_t  alloc,   /*!< number of bytes allocated to source buffer (does not change) */
          length,  /*!< number of good bytes in buffer */
          index,   /*!< location of next char to scan */
          start,   /*!< location of first char in last NMEA sentence found by scan */
          state;   /*!< MARK if scan has started on a sentence, start<index */
} nmeabuf_t;


/*!
 * \brief message data returned by the scanner to indicate the location of an
 * NMEA sentence within a buffer.
 */
typedef struct nmeamsg
{
  nmeabuf_t *  nmeabuf;  /*!< buffer from which this message was scanned */
  size_t       start,    /*!< start of sentence in nmeabuf->buffer */
               length;   /*!< length in bytes of sentence */
} nmeamsg_t;


/*!
 * construct new nmeabuf_t with pointer to allocated area where data will go.
 * does not allocate any memory.
 * same as NMEABUF_INITIALIZER except calls nmea_reset() after initializing.
 *
 * \param buffer NMEA buffer context
 * \param source space allocated for data
 * \param maxlen size in bytes of allocated area \a source
 */
void nmea_ctor(nmeabuf_t *buffer, const char *source, const size_t maxlen);


/*!
 * clear the allocated area with NUL chars, set length to zero, clear the
 * scanner state.
 *
 * \param buffer NMEA buffer context
 */
void nmea_reset(nmeabuf_t *buffer);


/*!
 * copy new data into allocated area without breaking pointers to incompletely
 * scanned NMEA sentences; safely concatenate without losing state.
 *
 * \param buffer NMEA buffer context
 * \param rbuf string to concat to end of the NMEA buffer
 * \param rlen number of bytes to concatenate
 *
 * \returns one on success, zero if there is no space to copy or if \a rlen is zero.
 */
int nmea_concat(nmeabuf_t *buffer, const char *rbuf, const size_t rlen);


/*!
 * run the scanner on all the newest, unscanned chars in the buffer.
 *
 * \param buffer NMEA buffer context
 * \param message if not NULL, where scanned message info will be stored.
 *
 * \returns one if an NMEA sentence was found, zero otherwise. if \a message is
 * not NULL and a sentence was found, it will be populated with the necessary
 * info for nmea_parse() to safely extract that sentence from \a buffer without
 * losing state.
 */
int nmea_scan(nmeabuf_t *buffer, nmeamsg_t *message);


/*!
 * copy NMEA sentence previously scanned from nmeabuf_t to nmeamsg_t into a
 * regular string buffer.
 *
 * \param wbuf location to copy sentence to
 * \param maxlen maximum available number of bytes in \a wbuf
 * \param message NMEA message info to pull from buffer
 *
 * \returns length of sentence; number of characters written to \a wbuf.
 */
size_t nmea_peek(char *wbuf, const size_t maxlen, nmeamsg_t *message);


/*!
 * perform nmea_peek to copy NMEA sentence out of buffer, then safely update
 * pointers in nmeabuf_t pointed to by the message without losing state.
 *
 * \param wbuf location to copy sentence to
 * \param maxlen maximum available number of bytes in \a wbuf
 * \param message NMEA message info to pull from buffer
 *
 * \returns length of the sentence; number of characters written to \a wbuf.
 */
size_t nmea_parse(char *wbuf, const size_t maxlen, nmeamsg_t *message);


/*!
 * calculate and verify checksum
 *
 * \pre \a nmeasz is zero-terminated so that strtol() does not fail.
 * \post if not NULL, \a cksum will contain the calculated checksum.
 *
 * \returns zero if checksum present in sentence, if a checksum can be
 * calculated from the sentence contents, and if they match.
 */
int nmea_cksum(const char *nmeasz, long *cksum);


/*!
 * nmea_cksum wrapper for the nmeamsg_t type.
 * \see nmea_cksum
 */
int nmea_cksum_msg(nmeamsg_t *message, long *cksum);


/*!
 * display debug info for buffer. print buffer contents between position 0 and
 * the current buffer length minus one to \a stream, followed by a status line
 * indicating where the stream markers currently are. nonprintable characters
 * appear as a period ('.').
 *
 *   S  start-of-sentence.
 *   I  scanner index pointer.
 *   X  both S and I.
 *
 * \param stream target stream, e.g. stderr
 * \param buffer NMEA buffer context
 */
void nmea_debug(FILE *stream, const nmeabuf_t *buffer);


#endif
