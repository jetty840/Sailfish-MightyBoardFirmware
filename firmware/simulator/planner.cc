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
#if !defined(SAILTIME)
     va_list ap;
     size_t len;

     va_start(ap, fmt);

     len = strlen(pending_notices);
     vsnprintf(pending_notices + len, sizeof(pending_notices) - len, fmt, ap);

     va_end(ap);
#endif
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

#if !defined(SAILTIME)
     snprintf(ctx->buf, sizeof(ctx->buf), "*** %.*s ***",
	      (int)(0x7fffffff & len), (const char *)str);
#endif

     return(0);
}

#if defined(__arm__)
#define GETOPTS_END (char)-1
#else
#define GETOPTS_END -1
#endif

#if defined(SAILTIME)
#define PROGNAME "sailtime"
#define OPTIONS "[-? | -h] [-a x,y,z,a,b] [-c x,y,z,a,b]"
#define GETOPTS ":a:c:h?"
#define REPORT 0
#else
#define PROGNAME "planner"
#define OPTIONS "[-? | -h] [-a x,y,z,a,b] [-c x,y,z,a,b] [-mstu] [-d mask] [-r rate]"
#define GETOPTS ":a:c:hd:mr:stu?"
#define REPORT -1
#endif

static void usage(FILE *f, const char *prog)
{
     if (f == NULL)
	  f = stderr;

     fprintf(f,
"Usage: %s " OPTIONS " [file]\n"
"         file -- The name of the .s3g or .x3g file to dump.  If not supplied then stdin is dumped\n"
" -a x,y,z,a,b -- Maximum x, y, z, a, and b accelerations (mm/s^2)\n"
" -c x,y,z,a,b -- Maximum x, y, z, a, and b speed changes (mm/s)\n"
#if !defined(SAILTIME)
"      -d mask -- Selectively enable debugging with a bit mask \"mask\"\n"
"           -m -- Display actual s3g/x3g move commands and\n"
"      -r rate -- Flag feed rates which exceed \"rate\"\n"
"           -s -- Display block initial, peak and final speeds (mm/s) along with rates\n"
"           -u -- Display significant differences between interval based and us based feed rates\n"
#endif
"        ?, -h -- This help message\n"
"\n"
" Default maximum accelerations are:\n"
"     x, y = %d mm/s^2\n"
"        z = %d mm/s^2\n"
"     a, b = %d mm/s^2\n"
"\n"
" Default maximum speed changes are:\n"
"     x, y = %d mm/s\n"
"        z = %d mm/s\n"
"     a, b = %d mm/s\n",
	     prog ? prog : PROGNAME,
	     DEFAULT_MAX_ACCELERATION_AXIS_X, DEFAULT_MAX_ACCELERATION_AXIS_Z,
	     DEFAULT_MAX_ACCELERATION_AXIS_A,
	     DEFAULT_MAX_SPEED_CHANGE_X, DEFAULT_MAX_SPEED_CHANGE_Z,
	     DEFAULT_MAX_SPEED_CHANGE_A);
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

     while ((c = getopt(argc, (char **)argv, GETOPTS)) != GETOPTS_END)
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

	       int32_t ab[2] = { target[A_AXIS], target[B_AXIS] };
	       for (int i = 0; i < 2; i ++ )
	       {
		    if ( cmd.t.queue_point_new.rel & (1 << (A_AXIS + i)))
		    {
			 filamentLength[i] += (int64_t)ab[i];
			 lastFilamentPosition[i] += ab[i];
		    }
		    else
		    {
			 filamentLength[i] += (int64_t)(ab[i] - lastFilamentPosition[i]);
			 lastFilamentPosition[i] = ab[i];
		    }
	       }

	       steppers::setTargetNew(target, 0, cmd.t.queue_point_new.us, cmd.t.queue_point_new.rel);

	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();

	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1, REPORT);
	  }
	  else if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_NEW_EXT)
	  {
	       Point target = Point(cmd.t.queue_point_new_ext.x, cmd.t.queue_point_new_ext.y,
				    cmd.t.queue_point_new_ext.z, cmd.t.queue_point_new_ext.a,
				    cmd.t.queue_point_new_ext.b);

	       int32_t ab[2] = { target[A_AXIS], target[B_AXIS] };
	       for (int i = 0; i < 2; i ++ )
	       {
		    if ( cmd.t.queue_point_new.rel & (1 << (A_AXIS + i)))
		    {
			 filamentLength[i] += (int64_t)ab[i];
			 lastFilamentPosition[i] += ab[i];
		    }
		    else
		    {
			 filamentLength[i] += (int64_t)(ab[i] - lastFilamentPosition[i]);
			 lastFilamentPosition[i] = ab[i];
		    }
	       }

	       steppers::setTargetNewExt(target, cmd.t.queue_point_new_ext.dda_rate,
					 cmd.t.queue_point_new_ext.rel,
					 cmd.t.queue_point_new_ext.distance,
					 cmd.t.queue_point_new_ext.feedrate_mult_64);

	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();

	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1, REPORT);
	  }
	  else if (cmd.cmd_id == HOST_CMD_QUEUE_POINT_EXT)
	  {
	       Point target = Point(cmd.t.queue_point_ext.x, cmd.t.queue_point_ext.y,
				    cmd.t.queue_point_ext.z, cmd.t.queue_point_ext.a,
				    cmd.t.queue_point_ext.b);

	       filamentLength[0] += (int64_t)(target[A_AXIS] - lastFilamentPosition[0]);
	       filamentLength[1] += (int64_t)(target[B_AXIS] - lastFilamentPosition[1]);
	       lastFilamentPosition[0] = target[A_AXIS];
	       lastFilamentPosition[1] = target[B_AXIS];

	       steppers::setTargetNew(target, cmd.t.queue_point_ext.dda, 0, 0);
	       if (show_moves && myctx.buf[0]) pending_notice("%s\n", myctx.buf);

	       handle_pending_notices();

	       if (movesplanned() >= (BLOCK_BUFFER_SIZE >> 1)) plan_dump_current_block(1, REPORT);
	  }
	  else if (cmd.cmd_id == HOST_CMD_SET_POSITION_EXT)
	  {
	       Point target = Point(cmd.t.set_position_ext.x, cmd.t.set_position_ext.y,
				    cmd.t.set_position_ext.z, cmd.t.set_position_ext.a,
				    cmd.t.set_position_ext.b);

	       lastFilamentPosition[0] = target[A_AXIS];
	       lastFilamentPosition[1] = target[B_AXIS];

	       steppers::definePosition(target, false);

	       if (myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	       handle_pending_notices();
	  }
	  else if (cmd.cmd_id == HOST_CMD_SET_ACCELERATION_TOGGLE)
	  {
	       steppers::setSegmentAccelState((cmd.t.set_segment_acceleration.s != 0) ? true : false);		  
	       if (myctx.buf[0]) pending_notice("%s\n", myctx.buf);
	  }
	  else if (cmd.cmd_id == HOST_CMD_RECALL_HOME_POSITION)
	  {
	       // Assume A and B axis have 0 for their home positions
	       if (cmd.t.recall_home_position.axes & (1 << A_AXIS))
		    lastFilamentPosition[0] = 0;
	       if (cmd.t.recall_home_position.axes & (1 << B_AXIS))
		    lastFilamentPosition[0] = 0;
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
		    if (warn && REPORT) {
			printf("*** >>> Draining planning buffer <<< ***\n");
			fflush(stdout);
		    }
		    while (movesplanned() != 0)
			plan_dump_current_block(1, REPORT);
		    if (warn && REPORT) {
			printf("*** >>> Planning buffer drained <<< ***\n");
			fflush(stdout);
		    }
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
	 plan_dump_current_block(1, REPORT);

     s3g_close(ctx);

     plan_dump_run_data((REPORT) ? 0 : -1);

     return(0);
}
