// Simple tool to dump a .s3g file either from disk of from stdin
//
//     s3gdump filename
//
// or
//
//     s3gdump < filename

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "s3g.h"

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

// For a stream of .s3g commands, cherry pick out the movement related
// commands, displaying the distance moved in 3-space (xyz) and the
// density of extruder steps over that distance [extruder steps / distance].

// BUG: For the time being, the steps/mm for each axis is hard coded
static void edensity(const s3g_command_t *cmd)
{
#define AXIS_COUNT 5
#define X_AXIS     0
#define Y_AXIS     1
#define Z_AXIS     2
#define A_AXIS     3
#define B_AXIS     4
#define E_AXIS     (A_AXIS)

     // On the first call, display column headings
     static int32_t first = 1;

     // Track the previous position
     static int32_t last_position[AXIS_COUNT] = {0, 0, 0, 0, 0};

     // Steps per mm for each axis
     static float axis_steps_per_unit[AXIS_COUNT] = {
	  47.069852,     // x
	  47.069852,     // y
	  200.0,         // z
	  50.2354788069, // a
	  50.2354788069  // b
     };

     if (!cmd)
	  return;

     // Generate column headings
     if (first)
     {
	  printf(" distance e-density (esteps)\n");
	  first = 0;
     }

     // Now take an action based upon the command id
     switch (cmd->cmd_id)
     {

     // Non motion related command; simply print the command's name
     default :
	  if (cmd->cmd_name != NULL)
	       printf("- %s -\n", cmd->cmd_name);
	  break;

     // Command for the extruder controller
     case HOST_CMD_TOOL_COMMAND :
	  if (cmd->cmd_name != NULL)
	  {
	       if (cmd->t.tool.subcmd_name != NULL)
		    printf("- %s (%s (%d) for tool %u to %u) -\n",
			   cmd->cmd_name, cmd->t.tool.subcmd_name, cmd->t.tool.subcmd_id,
			   cmd->t.tool.index, cmd->t.tool.subcmd_value);
	       else
		    printf("- %s -\n", cmd->cmd_name);
	  }
	  break;

     // Move to a new position
     case HOST_CMD_QUEUE_POINT_NEW :
     case HOST_CMD_QUEUE_POINT_EXT :
	  {
	       float d, delta, e_density;
	       int32_t esteps, t[AXIS_COUNT], zsteps;
	       int i;
	       uint8_t rel;

	       // Extract the x, y, z, a, b data from the command

	       if (cmd->cmd_id == HOST_CMD_QUEUE_POINT_NEW)
	       {
		    t[X_AXIS] = cmd->t.queue_point_new.x;
		    t[Y_AXIS] = cmd->t.queue_point_new.y;
		    t[Z_AXIS] = cmd->t.queue_point_new.z;
		    t[A_AXIS] = cmd->t.queue_point_new.a;
		    t[B_AXIS] = cmd->t.queue_point_new.b;
		    rel = cmd->t.queue_point_new.rel;
	       }
#if 0
	       else if (cmd->cmd_id == HOST_CMD_QUEUE_POINT_ABS)
	       {
		    t[X_AXIS] = cmd->t.queue_point_abs.x;
		    t[Y_AXIS] = cmd->t.queue_point_abs.y;
		    t[Z_AXIS] = cmd->t.queue_point_abs.z;
		    t[A_AXIS] = 0;
		    t[B_AXIS] = 0;
		    rel = 0;
	       }
#endif
	       else // cmd->cmd_id == HOST_CMD_QUEUE_POINT_EXT
	       {
		    t[X_AXIS] = cmd->t.queue_point_ext.x;
		    t[Y_AXIS] = cmd->t.queue_point_ext.y;
		    t[Z_AXIS] = cmd->t.queue_point_ext.z;
		    t[A_AXIS] = cmd->t.queue_point_ext.a;
		    t[B_AXIS] = cmd->t.queue_point_ext.b;

		    // No relative coordinates for "queue point ext"
		    rel = 0;
	       }

	       // Process any relative coordinates
	       if (rel != 0)
		    for (i = 0; i < AXIS_COUNT; i++)
			 if ((rel & (1 << i)) != 0)
			      t[i] += last_position[i];

	       // Warn about B_AXIS steps
	       if (t[B_AXIS] != 0)
		    printf("*** Warning: %d B axis steps taken ***\n", t[B_AXIS]);

	       // Now, compute the distance moved along the first three
	       // axes.  Those would be the x, y, and z axes.
	       d = 0.0;
	       for (i = X_AXIS; i <= Z_AXIS; i++)
	       {
		    // Steps moved is new position less the old position
		    // Distance moved in mm is then (steps moved) / (steps / mm)
		    delta = (float)(t[i] - last_position[i]) /
			                           axis_steps_per_unit[i];
		    d += delta * delta;
	       }

	       // Distance is the square root of the sum of delta^2
	       d = sqrt(d);

	       // Distance moved along the z axis
	       zsteps = t[Z_AXIS] - last_position[Z_AXIS];

	       // Steps moved along the e (a) axis
	       esteps = t[A_AXIS] - last_position[A_AXIS];

	       // Compute the ratio of e-steps moved to distance moved
	       if (d > 1.0e-5)
		    e_density = (float)esteps / d;
	       else
		    e_density = 0.0;

	       // Now generate a line of output
	       // We indicate if we moved up or down in layer height as well
	       //    that's useful for identifying the start of a new layer
	       if (zsteps == 0)
		    printf("  %7.3f    %7.2f (%6d)\n", d, e_density, esteps);
	       else
		    printf("%c %7.3f    %7.2f (%6d)\n",
			   (zsteps > 0) ? '^' : 'v', d, e_density, esteps);

	       // Now save the current position t as the "last position"
	       //   for our next call
	       memcpy(last_position, t, AXIS_COUNT * sizeof(int32_t));
	  }
	  break;

     // Set the current position
     case HOST_CMD_SET_POSITION_EXT :
	  last_position[0] = cmd->t.set_position_ext.x;
	  last_position[0] = cmd->t.set_position_ext.y;
	  last_position[0] = cmd->t.set_position_ext.z;
	  last_position[0] = cmd->t.set_position_ext.a;
	  last_position[0] = cmd->t.set_position_ext.b;
	  break;
     }

     return;
}

int main(int argc, const char *argv[])
{
     char c;
     s3g_context_t *ctx;
     s3g_command_t cmd;
     int do_edensity;

     do_edensity = 0;
     while ((c = getopt(argc, (char **)argv, ":hE?")) != -1)
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
	       return(0);

	       // -E print e stepper density
	  case 'E' :
	       do_edensity = -1;
	       break;
	  }
     }

     // Not that we care at this point
     argc -= optind;
     argv += optind;

     if (argc == 0)
	  ctx = s3g_open(0, NULL);
     else
	  ctx = s3g_open(0, (void *)argv[0]);

     if (!ctx)
	  // Assume that s3g_open() has complained
	  return(1);

     while (!s3g_command_read(ctx, &cmd))
     {
	  if (do_edensity == 0)
	       printf("%s (%d)\n", cmd.cmd_name, cmd.cmd_id);
	  else
	       edensity(&cmd);
     }

     s3g_close(ctx);

     return(0);
}
