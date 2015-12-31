#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "s3g_private.h"
#include "s3g_stdio.h"
#include "s3g.h"
#include "planner.h"
#include "planner_queue.h"
#include "planner_position.h"

typedef struct {
     size_t        len;
     unsigned char cmd[MAX_S3G_CMD_LEN];
} cmd_raw_t;

typedef struct cmd_move_s {
     struct cmd_move_s *prev;
     struct cmd_move_s *next;
     int      t[NAXES];     // TESTING
     int      steps[NAXES]; // axial displacement (steps)
     int      msteps;       // highest step count for any axis
     int      msteps_index; // index of axis with highest step count
     float    d[NAXES];     // axial displacement (mm)
     float    s[NAXES];     // axial speed (mm/s)
     float    feedrate;     // feedrate (mm/s)
     float    distance;     // distance (mm)
     int      dda;          // DDA rate
     uint8_t  relmask;
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

int s3g_queue_len(void)
{
     return (queue_next);
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

int s3g_queue_unaccelerated(s3g_command_t *cmd, int *target, uint8_t relmask)
{
     int i;

     if (!cmd || !target)
     {
	  errno = EINVAL;
	  return(-1);
     }

     // Resolve relative positions
     for (i = 0; i < NAXES; i++)
	  if (relmask & (1 << i))
	       target[i] += position[i];

     // Update the current position
     s3g_position_set(target);

     // And add this to the queue of commands
     if (s3g_queue_add_cmd(cmd->cmd_raw, cmd->cmd_raw_len))
	  return(-1);

     return(0);
}


int s3g_queue_accelerated(int *target, bool accelerated, int dda_rate, float distance,
			  float feedrate, uint8_t relmask)
{
     int i;
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

     ptr->msteps       = 0;
     ptr->msteps_index = 0;

     for (i = 0; i < NAXES; i++)
     {
	  // REMOVE
	  ptr->t[i] = target[i];

	  // Resolve relative positions
	  if (relmask & (1 << i))
	       target[i] += position[i];

	  // Axial step count; can be positive or negative

	  ptr->steps[i] = (int)(target[i] - position[i]);

	  // Axis with the highest step count
	  if (abs(ptr->steps[i]) > ptr->msteps)
	  {
	       ptr->msteps       = abs(ptr->steps[i]);
	       ptr->msteps_index = i;
	  }

	  // Axial distance in millimeters
	  ptr->d[i] = 1.0; // steps_to_mm(i, ptr->steps[i]);
     }

     ptr->dda      = dda_rate;
     ptr->distance = distance;
     ptr->feedrate = feedrate;
     ptr->relmask  = relmask;

     if (queue_add(1, (void *)ptr))
     {
	  free(ptr);
	  if (prev_move)
	       prev_move->next = NULL;
	  return(-1);
     }

     s3g_position_set(target);
     prev_move = ptr;

     return(0);
}

int s3g_queue_flush(s3g_context_t *ctx)
{
     int i, iret;

     if (!ctx)
     {
	  errno = EINVAL;
	  return(-1);
     }

     iret = 0;
     for (i = 0; i < queue_next; i++)
     {
	  if (!queue[i].cmd)
	       // Should never happen
	       continue;
	  else if (iret)
	       free(queue[i].cmd);
	  else if (queue[i].is_move)
	  {
	       unsigned char buf[32], *bufptr;
	       size_t len;
	       cmd_move_t *ptr = (cmd_move_t *)queue[i].cmd;

	       bufptr = buf;
	       bufptr = w8(bufptr,     (uint8_t)HOST_CMD_QUEUE_POINT_NEW_EXT);
	       bufptr = w32(bufptr,    (int32_t)ptr->t[0]);
	       bufptr = w32(bufptr,    (int32_t)ptr->t[1]);
	       bufptr = w32(bufptr,    (int32_t)ptr->t[2]);
	       bufptr = w32(bufptr,    (int32_t)ptr->t[3]);
	       bufptr = w32(bufptr,    (int32_t)ptr->t[4]);
	       bufptr = w32(bufptr,    (int32_t)ptr->dda);
	       bufptr = w8(bufptr,     (uint8_t)ptr->relmask);
	       bufptr = wfloat(bufptr, (float)ptr->distance);
	       bufptr = w16(bufptr,    (int16_t)(ptr->feedrate * 64.0));

	       len = bufptr - buf;
	       if ((ssize_t)len != (*ctx->write)(ctx->w_ctx, buf, len))
		    iret = -1;
	       free(ptr);
	  }
	  else {
	       cmd_raw_t *ptr = (cmd_raw_t *)queue[i].cmd;

	       if ((ssize_t)ptr->len != (*ctx->write)(ctx->w_ctx, ptr->cmd, ptr->len))
		    iret = -1;
	       free(ptr);
	  }
	  queue[i].cmd = NULL;
     }

     queue_next = 0;
     prev_move  = NULL;

     return(iret);
}

