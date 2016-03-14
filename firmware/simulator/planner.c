// Simple tool to dump a .s3g file either from disk of from stdin
//
//     s3gcopy filename
//
// or
//
//     s3gcopy < filename

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include "s3g.h"
#include "planner.h"
#include "planner_queue.h"
#include "planner_position.h"

#if defined(__arm__)
#define GETOPTS_END (char)-1
#else
#define GETOPTS_END -1
#endif

static void usage(FILE *f, const char *prog)
{
     if (f == NULL)
	  f = stderr;

     fprintf(f,
"Usage: %s -hE [file]\n"
"   file  -- The .s3g file to dump.  If not supplied then stdin is dumped\n"
"  ?, -h  -- This help message\n"
"     -E  -- Display distance moved and ratio of extruder steps to distance\n",
	     prog ? prog : "s3gdump");
}

int main(int argc, const char *argv[])
{
     bool acceleration_enabled, blocking;
     s3g_command_t cmd;
     int tool_id;
     s3g_context_t *in_ctx, *out_ctx;
     int32_t tool_offsets[NTOOLS][NAXES];

     assert(sizeof(int) >= sizeof(int32_t));

     acceleration_enabled = false;
     in_ctx  = NULL;
     out_ctx = NULL;

     if (argc < 2)
	  in_ctx = s3g_open(0, NULL, O_RDONLY, 0);
     else
	  in_ctx = s3g_open(0, (void *)argv[1], O_RDONLY, 0);

     if (!in_ctx)
	  // Assume that s3g_open() has complained
	  return(1);

     if (!(out_ctx = s3g_open(0, "./out.x3g", O_CREAT | O_WRONLY, 0644)))
	  goto done;

     s3g_position_init();
     tool_id = 0;

     while (!s3g_command_read(in_ctx, &cmd))
     {
	  // Is this command a blocking command?
	  blocking = s3g_command_isblocking(&cmd);

	  switch (cmd.cmd_id)
	  {
	  default :
	       break;

	  // The following commands disrupt the known position

	  case HOST_CMD_FIND_AXES_MINIMUM :
	  case HOST_CMD_FIND_AXES_MAXIMUM :
	       s3g_position_mark_unknown((uint8_t)(cmd.t.find_axes_minmax.flags & 0xff));
	       break;

	  case HOST_CMD_RECALL_HOME_POSITION :
	       // Recalling X, Y, or Z makes them unknown
	       s3g_position_mark_unknown((uint8_t)(cmd.t.recall_home_position.axes & AXES_XYZ_MASK));

	       // Recalling A or B generally always makes them known: 0
	       s3g_position_mark_known(3, (int)0);
	       s3g_position_mark_known(4, (int)0);

	       break;

	  // The following commands define the current position

	  case HOST_CMD_SET_POSITION_EXT :
	       s3g_position_mark_known(0, (int)cmd.t.set_position_ext.x);
	       s3g_position_mark_known(1, (int)cmd.t.set_position_ext.y);
	       s3g_position_mark_known(2, (int)cmd.t.set_position_ext.z);
	       s3g_position_mark_known(3, (int)cmd.t.set_position_ext.a);
	       s3g_position_mark_known(4, (int)cmd.t.set_position_ext.b);
	       break;

	  // Change acceleration state

	  case HOST_CMD_SET_ACCELERATION_TOGGLE :
	       acceleration_enabled = cmd.t.set_segment_acceleration.s == 1;
	       // Strip command from X3G stream
	       cmd.cmd_raw_len = 0;
	       break;

	  // Unaccelerated motion commands

	  case HOST_CMD_QUEUE_POINT_NEW :
	  case HOST_CMD_QUEUE_POINT_EXT :
	  {
	       int target[NAXES];
	       uint8_t relmask;

	       if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW)
	       {
		    target[0] = (int)cmd.t.queue_point_new.x;
		    target[1] = (int)cmd.t.queue_point_new.y;
		    target[2] = (int)cmd.t.queue_point_new.z;
		    target[3] = (int)cmd.t.queue_point_new.a;
		    target[4] = (int)cmd.t.queue_point_new.b;
		    relmask   = (uint8_t)(cmd.t.queue_point_new.rel & 0xff);
	       }
	       else
	       {
		    target[0] = (int)cmd.t.queue_point_ext.x;
		    target[1] = (int)cmd.t.queue_point_ext.y;
		    target[2] = (int)cmd.t.queue_point_ext.z;
		    target[3] = (int)cmd.t.queue_point_ext.a;
		    target[4] = (int)cmd.t.queue_point_ext.b;
		    relmask   = (uint8_t)0;
	       }
	       if (s3g_queue_unaccelerated(&cmd, target, relmask))
	       {
		    fprintf(stderr, "*** queue_unaccelerated() badness ***\n");
		    return(-1);
	       }
	       cmd.cmd_raw_len = 0;
	       break;
	  }

	  // Accelerated motion command

	  case HOST_CMD_QUEUE_POINT_NEW_EXT :
	  {
	       int32_t target[NAXES];

	       target[0] = (int)cmd.t.queue_point_new_ext.x;
	       target[1] = (int)cmd.t.queue_point_new_ext.y;
	       target[2] = (int)cmd.t.queue_point_new_ext.z;
	       target[3] = (int)cmd.t.queue_point_new_ext.a;
	       target[4] = (int)cmd.t.queue_point_new_ext.b;

	       if (s3g_queue_accelerated(target, acceleration_enabled,
					 (int)cmd.t.queue_point_new_ext.dda_rate,
					 (float)cmd.t.queue_point_new_ext.distance,
					 (float)cmd.t.queue_point_new_ext.feedrate_mult_64/64.0,
					 (uint8_t)(cmd.t.queue_point_new_ext.rel & 0xff)))
	       {
		    fprintf(stderr, "*** queue_accelerated() badness ***\n");
		    return(-1);
	       }
	       cmd.cmd_raw_len = 0;
	       break;
	  }
	  }

	  if (s3g_queue_len() == 0)
	  {
	       // Nothing is presently queued -- just flush
	       //   Note: accelerated motion command went into the queue; len(queue) != 0
	       //         unaccelerated motion command went into the queue; len(queue) != 0

	       if (s3g_command_write(out_ctx, &cmd))
	       {
		    fprintf(stderr, "*** 1 write() badness ***\n");
		    return(-1);
	       }
	  }
	  else if (!blocking)
	  {
	       // Motion commands are queued
	       // This command doesn't block any motion commands so just add it to the queue
	       if (s3g_queue_add_cmd(cmd.cmd_raw, cmd.cmd_raw_len))
	       {
		    fprintf(stderr, "*** queue_add_cmd() badness ***\n");
		    return(-1);
	       }
	  }
	  else
	  {
	       // We have queued motion commands
	       // This command cannot be executed by the bot until the motion commands finish
	       if (s3g_queue_flush(out_ctx)) {
		    fprintf(stderr, "*** 2 s3g_queue_flush() badness ***\n");
		    return(-1);
	       }

	       if (s3g_command_write(out_ctx, &cmd))
	       {
		    fprintf(stderr, "*** 3 s3g_command_write() badness ***\n");
		    return(-1);
	       }
	  }
     }

     if (s3g_queue_flush(out_ctx))
     {
	  fprintf(stderr, "*** 4 s3g_queue_flush() badness ***\n");
	  return(-1);
     }

done:
     s3g_close(in_ctx);
     s3g_close(out_ctx);
     
     return(0);
}
