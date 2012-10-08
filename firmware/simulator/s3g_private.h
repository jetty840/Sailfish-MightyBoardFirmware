// s3g_private.h
// Private declarations for the s3g library

#ifndef S3G_PRIVATE_H_

#define S3G_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

// read, write, and close procedure definitions for .s3g file drivers
typedef ssize_t s3g_read_proc_t(void *ctx, unsigned char *buf, size_t maxbuf, size_t nbytes);
typedef ssize_t s3g_write_proc_t(void *ctx, unsigned char *buf, size_t nbytes);
typedef int s3g_close_proc_t(void *ctx);

// The actual s3g_context_t declaration

#ifndef S3G_CONTEXT_T_
#define S3G_CONTEXT_T_
typedef struct {
     s3g_read_proc_t  *read;    // File driver read procedure; req'd for reading
     s3g_write_proc_t *write;   // File driver write procedure; req'd for writing
     s3g_close_proc_t *close;   // File driver close procedure; optional
     void             *r_ctx;   // File driver private context
     void             *w_ctx;   // File driver private context
} s3g_context_t;
#endif

// File driver open procedure; no need at present to keep this in the context

typedef int s3g_open_proc_t(s3g_context_t *ctx, void *src);

#ifdef __cplusplus
}
#endif

#endif
