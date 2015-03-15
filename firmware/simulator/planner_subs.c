#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "s3g.h"
#include "planner.h"
#include "planner_subs.h"

typedef struct {
     size_t        len;
     unsigned char cmd[MAX_S3G_CMD_LEN];
} cmd_raw_t;

typedef struct cmd_move_s {
     struct cmd_move_s *prev;
     struct cmd_move_s *next;
     int32_t            t[NAXES];  // target stepper space (steps)
     float              s[NAXES];  // axial speed (mm/s)
     float              feedrate;  // feedrate (mm/s)
     float              distance;  // distance (mm)
     uint32_t           dda;       // DDA rate
     uint8_t            rel;
} cmd_move_t;

typedef struct queue_s {
     int   is_move;
     void *cmd;
} queue_t;

static queue_t *queue = NULL;
static size_t queue_max = 0;
static size_t queue_next = 0;
static cmd_move_t *prev_move = NULL;

static unsigned char *w8(unsigned char *buf, unsigned char v)
{
     *buf++ = v;
     return (buf);
}

static unsigned char *w16(unsigned char *buf, uint16_t v)
{
     union {
	  uint16_t      v;
	  unsigned char b[2];
     } uv;

     uv.v = v;

     *buf++ = uv.b[0];
     *buf++ = uv.b[1];

     return(buf);
}

static unsigned char *wfloat(unsigned char *buf, float v)
{
    union {
        float         v;
        unsigned char b[4];
    } uv;

    uv.v = v;
    *buf++ = uv.b[0];
    *buf++ = uv.b[1];
    *buf++ = uv.b[2];
    *buf++ = uv.b[3];

    return (buf);
}


static unsigned char *w32(unsigned char *buf, uint32_t v)
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#error big endian -> little endian code needed
#else
     union {
	  uint32_t      v;
	  unsigned char b[4];
     } uv;

     uv.v = v;
     *buf++ = uv.b[0];
     *buf++ = uv.b[1];
     *buf++ = uv.b[2];
     *buf++ = uv.b[3];

     return (buf);
#endif
}

static int queue_add(int is_move, void *cmd)
{
     queue_t *tmp;

     // Room for more in the queue?
     if (queue_next >= queue_max)
     {
	  // Need to grow the queue
	  size_t newmax = queue_max + 1000;
	  tmp = realloc(queue, newmax * sizeof(queue_t));
	  if (!tmp)
	       return(-1);

	  queue = tmp;
	  queue_max = newmax;
     }

     queue[queue_next].is_move = is_move;
     queue[queue_next].cmd     = cmd;
     ++queue_next;

     return(0);
}

void s3g_queue_init(void)
{
     if (queue)
     {
	  int i;

	  if (queue_next > queue_max)
	       queue_next = queue_max;

	  for (i = 0; i < queue_next; i++)
	  {
	       if (queue[i].cmd)
	       {
		    free(queue[i].cmd);
		    queue[i].cmd = NULL;
	       }
	  }
	  queue_next = 0;
     }
     else
     {
	  queue_next = 0;
	  queue_max  = 0;
     }
}

int s3g_queue_add_cmd(unsigned char *cmd, size_t len)
{
     cmd_raw_t *ptr;

     if (!cmd || len > MAX_S3G_CMD_LEN)
     {
	  errno = EINVAL;
	  return(-1);
     }

     if (len == 0)
	  return(0);

     ptr = calloc(1, sizeof(cmd_raw_t));
     if (!ptr)
	  return(-1);

     ptr->len = len;
     memcpy(ptr->cmd, cmd, len);

     if (queue_add(0, (void *)ptr))
     {
	  free(ptr);
	  return(-1);
     }

     return(0);
}

int s3g_queue_add_move(int32_t *target, int32_t dda, float distance, float feedrate,
		       uint8_t rel)
{
     cmd_move_t *ptr;

     if (!target)
     {
	  errno = EINVAL;
	  return(-1);
     }

     ptr = calloc(1, sizeof(cmd_move_t));
     if (!ptr)
	  return(-1);

     ptr->prev = prev_move;
     if (prev_move)
	  prev_move->next = ptr;

     memcpy(ptr->t, target, NAXES * sizeof(int32_t));
     ptr->dda      = dda;
     ptr->distance = distance;
     ptr->feedrate = feedrate;
     ptr->rel        = rel;

     if (queue_add(0, (void *)ptr))
     {
	  free(ptr);
	  if (prev_move)
	       prev_move->next = NULL;
	  return(-1);
     }

     prev_move = ptr;

     return(0);
}

int s3g_queue_flush(int outfd)
{
     int i;

     for (i = 0; i < queue_next; i++)
     {
	  if (!queue[i].cmd)
	       // Should never happen
	       continue;

	  if (queue[i].is_move)
	  {
	       unsigned char buf[32], *bufptr;
	       cmd_move_t *ptr = (cmd_move_t *)queue[i].cmd;

	       bufptr = buf;
	       bufptr = w32(bufptr, ptr->t[0]);
	       bufptr = w32(bufptr, ptr->t[1]);
	       bufptr = w32(bufptr, ptr->t[2]);
	       bufptr = w32(bufptr, ptr->t[3]);
	       bufptr = w32(bufptr, ptr->t[4]);
	       bufptr = w32(bufptr, ptr->dda);
	       bufptr = w8(bufptr,     ptr->rel);
	       bufptr = wfloat(bufptr, ptr->distance);
	       bufptr = w16(bufptr,    (int16_t)(ptr->feedrate * 64.0));

	       write(outfd, buf, bufptr - buf);
	       free(ptr);
	  }
	  else {
	       cmd_raw_t *ptr = (cmd_raw_t *)queue[i].cmd;

	       write(outfd, ptr->cmd, ptr->len);
	       free(ptr);
	  }
	  queue[i].cmd = NULL;
     }

     queue_next = 0;
     prev_move  = NULL;

     return(0);
}
