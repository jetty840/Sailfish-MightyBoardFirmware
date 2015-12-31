#ifndef S3G_PLANNER_QUEUE_H_

#define S3G_PLANNER_QUEUE_H_

#include <stdbool.h>
#include "s3g.h"

void s3g_queue_init(void);
int s3g_queue_len(void);
int s3g_queue_add_cmd(unsigned char *cmd, size_t len);
int s3g_queue_flush(s3g_context_t *ctx);
int s3g_queue_unaccelerated(s3g_command_t *cmd, int *target, uint8_t relmask);
int s3g_queue_accelerated(int *target, bool acceleration_enabled, int dda_rate,
  float distance, float feedrate, uint8_t relmask);

#endif
