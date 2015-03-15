// Simple tool to dump a .s3g file either from disk of from stdin
//
//     s3gcopy filename
//
// or
//
//     s3gcopy < filename

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "s3g.h"
#include "planner.h"
#include "planner_subs.h"

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
     s3g_context_t *inctx;
     s3g_command_t cmd;
     int blocking, i, outfd, position_axis_mask, position_known, queued;
     int32_t position[NAXES], target[NAXES];
     uint8_t relative_mask;

     outfd  = -1;
     queued = 0;

     if (argc < 2)
	  inctx = s3g_open(0, NULL);
     else
	  inctx = s3g_open(0, (void *)argv[1]);

     
     if (!inctx)
	  // Assume that s3g_open() has complained
	  return(1);

     outfd = open("./out.x3g", O_CREAT | O_WRONLY, 0644);

     queued             = 0;
     position_known     = 0;
     position_axis_mask = 0x00;
     for ( i = 0; i < NAXES; i++)
     {
	  position[i] = 0;
	  position_axis_mask |= 1 << i;
     }

     while (!s3g_command_read(inctx, &cmd))
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
	       for (i = 0; i < NAXES; i++)
		    if (cmd.t.find_axes_minmax.flags & ( 1 << i ))
			 position_axis_mask |= ( 1 << i );
	       if ((position_axis_mask & 0x1F) != 0x00)
		    position_known = 0;
	       break;

	  case HOST_CMD_RECALL_HOME_POSITION :
	       // Recalling X, Y, or Z makes them unknown
	       for (i = 0; i < 3; i++)
		    if (cmd.t.recall_home_position.axes & ( 1 << i ))
			 position_axis_mask |= ( 1 << i );

	       // Recalling A or B generally always makes them known: 0
	       for (i = 3; i < NAXES; i++)
	       {
		    if (cmd.t.recall_home_position.axes & ( 1 << i ))
		    {
			 position[i] = 0;
			 position_axis_mask &= ~(1 << i);
		    }
	       }

	       if ((position_axis_mask & 0x1F) != 0x00)
		    position_known = 0;

	       break;

	  // The following commands define the current position

	  case HOST_CMD_SET_POSITION_EXT :
	       position[0] = cmd.t.set_position_ext.x;
	       position[1] = cmd.t.set_position_ext.y;
	       position[2] = cmd.t.set_position_ext.z;
	       position[3] = cmd.t.set_position_ext.a;
	       position[4] = cmd.t.set_position_ext.b;
	       position_axis_mask = 0;
	       position_known     = -1;
	       break;

	  // Motion commands

	  case HOST_CMD_QUEUE_POINT_NEW :
	  case HOST_CMD_QUEUE_POINT_EXT :
	  case HOST_CMD_QUEUE_POINT_NEW_EXT :
	       if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW)
	       {
		    target[0] = cmd.t.queue_point_new.x;
		    target[1] = cmd.t.queue_point_new.y;
		    target[2] = cmd.t.queue_point_new.z;
		    target[3] = cmd.t.queue_point_new.a;
		    target[4] = cmd.t.queue_point_new.b;
		    relative_mask = cmd.t.queue_point_new.rel;
	       }
	       else if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_EXT)
	       {
		    target[0] = cmd.t.queue_point_ext.x;
		    target[1] = cmd.t.queue_point_ext.y;
		    target[2] = cmd.t.queue_point_ext.z;
		    target[3] = cmd.t.queue_point_ext.a;
		    target[4] = cmd.t.queue_point_ext.b;
		    relative_mask = 0;
	       }
	       else
	       {
		    target[0] = cmd.t.queue_point_new_ext.x;
		    target[1] = cmd.t.queue_point_new_ext.y;
		    target[2] = cmd.t.queue_point_new_ext.z;
		    target[3] = cmd.t.queue_point_new_ext.a;
		    target[4] = cmd.t.queue_point_new_ext.b;
		    relative_mask = cmd.t.queue_point_new_ext.rel;
	       }

	       // Translate relative coordinates to absolute
	       //   By rights we should ensure that the position is known
	       //   for an axis using a relative position.  However, there's
	       //   not much we can do if a relative position is used for an
	       //   axis whose position is unknown....

	       for (i = 0; i < NAXES; i++)
	       {
		    if (relative_mask & (1 << i))
			 target[i] += position[i];
		    position[i] = target[i];
	       }

	       position_known     = -1;
	       position_axis_mask = 0x00;

	       if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW_EXT)
	       {
		    if (s3g_queue_add_move(target, cmd.t.queue_point_new_ext.dda_rate,
				       cmd.t.queue_point_new_ext.distance,
				       (float)cmd.t.queue_point_new_ext.feedrate_mult_64/64.0,
				       cmd.t.queue_point_new_ext.rel))
		    {
			 fprintf(stderr, "*** queue_add_move() badness ***\n");
			 return(-1);
		    }
		    queued = -1;
	       }

	       break;
	  }

	  if (!queued)
	  {
	       // Nothing is presently queued -- just flush
	       s3g_queue_flush(outfd);
	       if ((ssize_t)cmd.cmd_raw_len != write(outfd, cmd.cmd_raw, cmd.cmd_raw_len))
	       {
		    fprintf(stderr, "*** write() badness ***\n");
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
	       queued = -1;
	  }
	  else
	  {
	       // We have queued motion commands
	       // This command cannot be executed by the bot until the motion commands
	       // finish.
	       s3g_queue_flush(outfd);
	       write(outfd, cmd.cmd_raw, cmd.cmd_raw_len);
	       queued = 0;
	  }
     }

     s3g_queue_flush(outfd);
     close(outfd);

     s3g_close(inctx);

     return(0);
}
