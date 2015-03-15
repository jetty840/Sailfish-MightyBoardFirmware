#ifndef S3G_PLANNER_SUBS_H_

#define S3G_PLANNER_SUBS_H_

void s3g_queue_init(void);
int s3g_queue_add_cmd(unsigned char *cmd, size_t len);
int s3g_queue_add_move(int32_t *target, int32_t dda, float distance,
  float feedrate, uint8_t rel);
int s3g_queue_flush(int outfd);

#endif
