/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <stdint.h>
#include <stdlib.h>
#include <lcm/lcm_coretypes.h>
#include <lcm/lcm.h>

#ifndef _line_t_h
#define _line_t_h

#ifdef __cplusplus
extern "C" {
#endif

#include "point_t.h"
typedef struct _line_t line_t;
struct _line_t
{
    point_t    point[2];
    int8_t     confidence;
};

line_t   *line_t_copy(const line_t *p);
void line_t_destroy(line_t *p);

typedef struct _line_t_subscription_t line_t_subscription_t;
typedef void(*line_t_handler_t)(const lcm_recv_buf_t *rbuf,
             const char *channel, const line_t *msg, void *user);

int line_t_publish(lcm_t *lcm, const char *channel, const line_t *p);
line_t_subscription_t* line_t_subscribe(lcm_t *lcm, const char *channel, line_t_handler_t f, void *userdata);
int line_t_unsubscribe(lcm_t *lcm, line_t_subscription_t* hid);
int line_t_subscription_set_queue_capacity(line_t_subscription_t* subs,
                              int num_messages);


int  line_t_encode(void *buf, int offset, int maxlen, const line_t *p);
int  line_t_decode(const void *buf, int offset, int maxlen, line_t *p);
int  line_t_decode_cleanup(line_t *p);
int  line_t_encoded_size(const line_t *p);

// LCM support functions. Users should not call these
int64_t __line_t_get_hash(void);
int64_t __line_t_hash_recursive(const __lcm_hash_ptr *p);
int     __line_t_encode_array(void *buf, int offset, int maxlen, const line_t *p, int elements);
int     __line_t_decode_array(const void *buf, int offset, int maxlen, line_t *p, int elements);
int     __line_t_decode_array_cleanup(line_t *p, int elements);
int     __line_t_encoded_array_size(const line_t *p, int elements);
int     __line_t_clone_array(const line_t *p, line_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif