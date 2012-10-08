// s3g_stdio.h
// Private declarations for the standard unix i/o file driver

#ifndef S3G_STDIO_H_

#define S3G_STDIO_H_

#include "s3g_private.h"

#ifdef __cplusplus
extern "C" {
#endif

// Driver's open procedure

s3g_open_proc_t s3g_stdio_open;

#ifdef __cplusplus
}
#endif

#endif
