#ifndef S3G_PLANNER_POSITION_H_

#define S3G_PLANNER_POSITION_H_

extern int position[NAXES];

void s3g_position_init(void);
void s3g_position_set(int *current);
void s3g_position_mark_unknown(uint8_t mask);
void s3g_position_mark_known(int axis, int coordinate);

#endif
