#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "Simulator.hh"
#include "StepperAccelPlannerExtras.hh"
#include "StepperAccel.hh"
#include "EepromMap.hh"
#include "Point.hh"
#include "Steppers.hh"
#include "s3g.h"

static char pending_notices[10240];

static void pending_notice(const char *fmt, ...)
{
     va_list ap;
     size_t len;

     va_start(ap, fmt);

     len = strlen(pending_notices);
     vsnprintf(pending_notices + len, sizeof(pending_notices) - len, fmt, ap);

     va_end(ap);
}

static void handle_pending_notices(void)
{
     if (pending_notices[0] == '\0')
	  return;

     plan_block_notice("%s", pending_notices);
     pending_notices[0] = '\0';
}

typedef struct {
     char buf[1024];
} myctx_t;

static s3g_write_proc_t display;
static ssize_t display(void *ctx_, unsigned char *str, size_t len)
{
     myctx_t *ctx = (myctx_t *)ctx_;
     if (!ctx)
	  return((ssize_t)-1);

     snprintf(ctx->buf, sizeof(ctx->buf), "*** %.*s ***",
	      (int)(0x7fffffff & len), (const char *)str);

     return(0);
}

static void usage(FILE *f, const char *prog)
{
     if (f == NULL)
	  f = stderr;

     fprintf(f,
"Usage: %s [? | -h] [-m] [-s] [-d mask] [-r rate] [-u] [file]\n"
"     file -- The name of the .s3g file to dump.  If not supplied then stdin is dumped\n"
"  -d mask -- Selectively enable debugging with a bit mask \"mask\"\n"
"       -m -- Display actual s3g move commands and\n"
"  -r rate -- Flag feed rates which exceed \"rate\"\n"
"       -s -- Display block initial, peak and final speeds (mm/s) along with rates\n"
"       -u -- Display significant differences between interval based and us based feed rates\n"
"    ?, -h -- This help message\n",
	     prog ? prog : "s3gdump");
}

int main(int argc, const char *argv[])
{
     char c;
     s3g_command_t cmd;
     s3g_context_t *ctx;
     myctx_t myctx;
     int show_moves = 0;

     steppers::init();
     steppers::reset();

     // Enable acceleration: it's off by default
     init_extras(true);

     pending_notices[0] = '\0';

     simulator_use_max_feed_rate = false;
     simulator_dump_speeds = false;
     simulator_show_alt_feed_rate = false;

     while ((c = getopt(argc, (char **)argv, ":a:c:hd:mr:su?")) != -1)
     {
	  switch(c)
	  {
	  // Unknown switch
	  case ':' :
	  default :
	       usage(stderr, argv[0]);
	       return(1);

	  // Explicit help request
	  case 'h' :
	  case '?' :
	       usage(stdout, argv[0]);
	       return(1);

          // max accelerations
	  case 'a' :
	  // max speed changes
	  case 'c' :
	  {
	       char cc;
	       int index = 0;
	       const char *ptr = optarg;
	       int16_t v = 0, vals[5];

	       while ((cc = *ptr++))
	       {
		    if (cc == ',')
		    {
			 if (index >= 5)
			 {
			      fprintf(stderr,
				      "Too many values specified in \"%s\"\n",
				      optarg);
			      return(1);
			 }
			 vals[index++] = v;
			 v = 0;
		    }
		    else if ('0' <= cc && cc <= '9')
			 v = v * 10 + (cc - '0');
		    else
		    {
			 fprintf(stderr, "Invalid syntax for \"%s\"\n", optarg);
			 return(1);
		    }
	       }
	       if (index >= 5)
	       {
		    fprintf(stderr,
			    "Too many values specified in \"%s\"\n",
			    optarg);
		       return(1);
	       }
	       vals[index++] = v;

	       if (c == 'a')
	       {
		    int j;
		    for (j = 0; j < STEPPER_COUNT; j++)
		    {
			 float steps_per_mm = (float)replicator_axis_steps_per_mm::axis_steps_per_mm[j] / 1000000.0f;
			 max_acceleration_units_per_sq_second[j] = (uint32_t)vals[j];
			 // Limit the max accelerations so that the calculation of block->acceleration & JKN Advance K2
			 // can be performed without overflow issues
			 if (max_acceleration_units_per_sq_second[j] > (uint32_t)((float)0xFFFFF / steps_per_mm))
			      max_acceleration_units_per_sq_second[j] = (uint32_t)((float)0xFFFFF / steps_per_mm);
			 axis_steps_per_sqr_second[j] = (uint32_t)((float)max_acceleration_units_per_sq_second[j] * steps_per_mm);
			 axis_accel_step_cutoff[j] = (uint32_t)0xffffffff / axis_steps_per_sqr_second[j];
		    }
	       }
	       else
	       {
		    int j;
		    for (j = 0; j < index; j++)
			 max_speed_change[j] = FTOFP((float)vals[j]);
	       }
	       break;
	  }

	  // Debug
	  case 'd' :
	  {
	       char *ptr = NULL;
	       simulator_debug = (uint32_t)(0xffffffff & strtoul(optarg, &ptr, 0));
	       if (ptr == NULL || ptr == optarg)
	       {
		    fprintf(stderr, "%s: unable to parse the debug mask, \"%s\", as an integer\n",
			    argv[0], optarg);
		    return(1);
	       }
	  }
	  break;

	  // Show moves
	  case 'm' :
	       show_moves = 1;
	       break;

	  // Max feed rate
	  case 'r' :
	  {
	       char *ptr = NULL;
	       float rate;

	       rate = strtof(optarg, &ptr);
	       if (ptr == NULL || ptr == optarg)
	       {
		    fprintf(stderr, "%s: unable to parse the feed rate, \"%s\", as a floating point number\n",
			    argv[0], optarg);
		    return(1);
	       }
	       simulator_use_max_feed_rate = true;
	       simulator_max_feed_rate = FTOFP(rate);
	  }
	  break;

          // Display speeds as well as rates
	  case 's' :
	       simulator_dump_speeds = true;
	       break;

          // Display significant differences between interval based and us based feed rates
	  case 'u' :
	       simulator_show_alt_feed_rate = true;
	       break;
	  }
     }

     argc -= optind;
     argv += optind;
     if (argc == 0)
	  // Open stdin
	  ctx = s3g_open(0, NULL);
     else
	  // Open the specified file
	  ctx = s3g_open(0, (void *)argv[0]);

     if (!ctx)
	  // Assume that s3g_open() has complained
	  return(1);

     // Add a writer to use when converting an .s3g packet to 
     // human readable text
     s3g_add_writer(ctx, &display, &myctx);

     // Now loop over the input .s3g stream | file

     while (!s3g_command_read(ctx, &cmd))
     {
	  // Convert the command to human readable text
	  myctx.buf[0] = '\0';
	  s3g_command_display(ctx, &cmd);

	  if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW)
	  {
	       Point target = Point(cmd.t.queue_point_new.x, cmd.t.queue_point_new.y,
				    cmd.t.queue_point_new.z, cmd.t.queue_point_new.a, 
				    cmd.t.queue_point_new.b);
	       steppers::setTargetNew(target, cmd.t.queue_point_new.us, cmd.t.queue_point_new.rel);
	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();
	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1);
	  }
	  else if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW_EXT)
	  {
	       Point target = Point(cmd.t.queue_point_new_ext.x, cmd.t.queue_point_new_ext.y,
				    cmd.t.queue_point_new_ext.z, cmd.t.queue_point_new_ext.a,
				    cmd.t.queue_point_new_ext.b);
	       steppers::setTargetNewExt(target, cmd.t.queue_point_new_ext.dda_rate,
					 cmd.t.queue_point_new_ext.rel,
					 cmd.t.queue_point_new_ext.distance,
					 cmd.t.queue_point_new_ext.feedrate_mult_64);
	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();
	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1);
	  }
	  else if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_EXT)
	  {
	       Point target = Point(cmd.t.queue_point_ext.x, cmd.t.queue_point_ext.y,
				    cmd.t.queue_point_ext.z, cmd.t.queue_point_ext.a,
				    cmd.t.queue_point_ext.b);
	       steppers::setTarget(target, cmd.t.queue_point_ext.dda);
	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();
	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1);
	  }
	  else if (cmd.cmd_id == HOST_CMD_SET_POSITION_EXT)
	  {
	       Point target = Point(cmd.t.set_position_ext.x, cmd.t.set_position_ext.y,
				    cmd.t.set_position_ext.z, cmd.t.set_position_ext.a,
				    cmd.t.set_position_ext.b);
	       steppers::definePosition(target);
	       if (myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();
	  }
	  else if (cmd.cmd_id == HOST_CMD_SET_ACCELERATION_TOGGLE)
	  {
	       steppers::setSegmentAccelState((cmd.t.set_segment_acceleration.s != 0) ? true : false);		  
	       if (myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	  }
	  else
	  {
	       // Dump queued blocks?
	       if (cmd.cmd_id != HOST_CMD_TOOL_COMMAND &&
		   cmd.cmd_id != HOST_CMD_ENABLE_AXES &&
		   cmd.cmd_id != HOST_CMD_SET_BUILD_PERCENT &&
		   cmd.cmd_id != HOST_CMD_CHANGE_TOOL &&
		   cmd.cmd_id != HOST_CMD_SET_POSITION_EXT)
	       {
		    bool warn = movesplanned() != 0;
		    if (warn) printf("*** >>> Draining planning buffer <<< ***\n");
		    while (movesplanned() != 0)
			 plan_dump_current_block(1);
		    if (warn) printf("*** >>> Planning buffer drained <<< ***\n");
	       }

	       if (myctx.buf[0] != '\0')
	       {
		    if (cmd.cmd_id == HOST_CMD_CHANGE_TOOL ||
			cmd.cmd_id == HOST_CMD_ENABLE_AXES ||
			cmd.cmd_id == HOST_CMD_SET_BUILD_PERCENT ||
			cmd.cmd_id == HOST_CMD_SET_POSITION_EXT ||
			cmd.cmd_id == HOST_CMD_TOOL_COMMAND)
			 pending_notice("%s\n", myctx.buf);
		    else
		    {
			 puts(myctx.buf);
		    }
	       }
	  }
     }

     // Dump any remaining blocks
     while (movesplanned() != 0)
	  plan_dump_current_block(1);

     s3g_close(ctx);

     plan_dump_run_data();

     return(0);
}
