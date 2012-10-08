#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "s3g_stdio.h"

// Identify temporary read errors
// Can be os-specific

#define FD_TEMPORARY_ERR() \
     (errno == EINTR || errno == ENOMEM || errno == ENOSR || errno == ENOBUFS)

// This driver's private context

typedef struct {
     int fd;  // File descriptor; < 0 indicates that the file is not open
} s3g_rw_stdio_ctx_t;


// stdio_close
//
// Close the input source and release the allocated driver context
//
// Call arguments:
//
//   void *ctx
//     Private driver context allocated by stdio_open().
//
// Return values:
//
//   0 -- Success
//  -1 -- Error; check errno may be a close() error which is significant when writing

static s3g_close_proc_t stdio_close;
static int stdio_close(void *ctx)
{
     int fd;
     s3g_rw_stdio_ctx_t *myctx = (s3g_rw_stdio_ctx_t *)ctx;

     // Sanity check
     if (!myctx)
     {
	  errno = EINVAL;
	  return(-1);
     }

     fd = myctx->fd;
     free(myctx);

     if (fd < 0)
     {
	  errno = EBADF;
	  return(-1);
     }

     return(close(fd));
}


// stdio_read_retry
//
// Read the specified number of bytes into the buffer.  Temporary read errors will
// be handled by this routine which will try to read all nbytes unless EOF is
// encountered.
//
// When the end of the file is reached before reading nbytes, the return value, R, will
// satisfy 0 <= R < nbytes.
//  
// *** Length checking is not performed on the buffer
// *** That is presumed to be done by stdio_read()
//
// Call arguments:
//
//   int fd
//     Descriptor of the open file to read from.
//
//   unsigned char *buf
//     Pointer to a buffer to read the data to.  Length of the buffer must be at
//     least nbytes long.  Buffer will NOT be NUL terminated.
//
//   size_t nbytes
//     The number of bytes to read from the input source.
//
// Return values:
//
//   > 0 -- Number of bytes read.  Guaranteed to be == nbytes UNLESS a permanent read
//          error occurs or the end of the file is reached.
//     0 -- End of file reached (unless nbytes == 0)
//    -1 -- File error; check errno

static ssize_t stdio_read_retry(int fd, unsigned char *buf, size_t nbytes)
{
     ssize_t n, nread;

     // Save read() the bother of this test
     if (fd < 0)
     {
	  errno = EBADF;
	  return((ssize_t)-1);
     }

     // Repeatedly call until we've read the requested amount of data or gotten
     // an error which isn't temporary
     nread = 0;
     for (;;)
     {
	  if ((n = read(fd, buf, nbytes)) <= 0 && FD_TEMPORARY_ERR())
	       continue;
	  if (n == 0)
	       // EOF reached
	       return(nread);
	  nread  += n;
	  nbytes -= n;
	  if (nbytes == 0)
	       return(nread);
	  buf += n;
     }
}


// stdio_read
//
// Read the specified number of bytes from the input source, placing at most
// maxbuf bytes into the buffer, buf.  If nbytes > maxbuf, then nbytes will be
// read.  However, only the first maxbuf bytes read will be stored in buf.  Note
// that buf will not be NUL terminated.
//
// Unless a permanent error occurs, stdio_read() guarantees that nbytes will actually
// be read.  If less than nbytes is returned, then either an end-of-file condition
// occurred or there was a non-temporary read error.
//
// On return, stdio_read() indicates how many bytes were actually read.
//
// Call arguments:
//
//   void *ctx
//     Private driver context created by stdio_open().
//
//   unsigned char *buf
//     Buffer into which to read the data.  At most maxbuf bytes will be stored in this
//     buffer.  Any bytes read beyond maxbuf will be discarded.  That only occurs when
//     nbytes > maxbuf.  If buf == NULL, then maxbuf will be considered 0 and nbytes
//     will be read and discarded.
//
//   size_t maxbuf
//     Maximum number of bytes to store in buf.  Maxbuf must not exceed the size of
//     the buffer pointed at by buf.
//
//   size_t nbytes
//     The number of bytes to read from the input source.
//
// Return values:
//
//  > 0 -- Number of bytes read.  If the returned value is less than nbytes, then an
//           end of file condition has occurred.
//    0 -- End of file reached or nbytes == 0
//   -1 -- Read error or invalid call arguments; check errno
//
static s3g_read_proc_t stdio_read;
static ssize_t stdio_read(void *ctx, unsigned char *buf, size_t maxbuf, size_t nbytes)
{
     s3g_rw_stdio_ctx_t *myctx = (s3g_rw_stdio_ctx_t *)ctx;
     ssize_t n;

     // Sanity check
     if (!myctx)
     {
	  errno = EINVAL;
	  return((ssize_t)-1);
     }

     // Return now if nothing to read
     if (nbytes == 0)
	  return((ssize_t)0);

     // Treat NULL for buf as though maxbuf == 0
     if (!buf)
	  maxbuf = 0;

     // Buffer is big enough to contain the entire read
     if (nbytes <= maxbuf)
	  return(stdio_read_retry(myctx->fd, buf, nbytes));

     // Buffer is not large enough to contain the entire read
     if ((n = stdio_read_retry(myctx->fd, buf, maxbuf)) <= 0)
	  return(n);

     // Read the remaining number of bytes requested without
     // shoving them into buf (which is full)
     nbytes -= n;
     {
	  unsigned char tmpbuf[1024];
	  size_t nread;

	  while (nbytes != 0)
	  {
	       
	       nread = (sizeof(tmpbuf) < nbytes) ? (size_t)sizeof(tmpbuf) : nbytes;
	       if ((n = stdio_read_retry(myctx->fd, tmpbuf, nread)) <= 0)
		    return(n);
	       nbytes -= nread;
	  }
     }
     return(0);
}


// s3g_stdio_open
// Our public open routine.  This is the only public routine for the driver.
//
// Call arguments
//
//   s3g_context_t *ctx
//     s3g context to associate ourselves with.
//
//   void *src
//     Input source information.  For this driver, a value of NULL indicates that
//     the input source is stdin.  Otherwise, the value is treated as a "const char *"
//     pointer pointing to the name of a file to open in read only mode.  A ".s3g"
//     will NOT be appended to the file name.  The file name must be the complete
//     file name (but need not be an absolute file path).
//
// Return values:
//
//   0 -- Success
//  -1 -- Error; check errno

int s3g_stdio_open(s3g_context_t *ctx, void *src)
{
     s3g_rw_stdio_ctx_t *tmp;

     // Sanity check
     if (!ctx)
     {
	  fprintf(stderr, "s3g_stdio_open(%d): Invalid call; ctx=NULL\n", __LINE__);
	  errno = EINVAL;
	  return(-1);
     }

     // Allocate memory for our "driver" context
     tmp = (s3g_rw_stdio_ctx_t *)calloc(1, sizeof(s3g_rw_stdio_ctx_t));
     if (tmp == NULL)
     {
	  fprintf(stderr, "s3g_open(%d): Unable to allocate VM; %s (%d)\n",
		  __LINE__, strerror(errno), errno);
	  return(-1);
     }

     // What sort of input source: named file or stdin?
     if (src == NULL)
     {
	  // Assume we're using stdin
	  tmp->fd = fileno(stdin);
     }
     else
     {
	  const char *fname = (const char *)src;
	  int fd = open(fname, O_RDONLY);
	  if (fd < 0)
	  {
	       fprintf(stderr, "s3g_open(%d): Unable to open the file \"%s\"; %s (%d)\n",
		       __LINE__, fname, strerror(errno), errno);
	       free(tmp);
	       return(-1);
	  }
	  tmp->fd = fd;
     }

     // All finished and happy
     ctx->close  = stdio_close;
     ctx->read   = stdio_read;
     ctx->write  = NULL;
     ctx->r_ctx  = tmp;
     ctx->w_ctx  = NULL;

     return(0);
}
