/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <string.h>
#include "line_t.h"

static int __line_t_hash_computed;
static int64_t __line_t_hash;

int64_t __line_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __line_t_get_hash)
            return 0;

    const __lcm_hash_ptr cp = { p, (void*)__line_t_get_hash };
    (void) cp;

    int64_t hash = 0x9aac7e0bb4207a88LL
         + __point_t_hash_recursive(&cp)
         + __int8_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __line_t_get_hash(void)
{
    if (!__line_t_hash_computed) {
        __line_t_hash = __line_t_hash_recursive(NULL);
        __line_t_hash_computed = 1;
    }

    return __line_t_hash;
}

int __line_t_encode_array(void *buf, int offset, int maxlen, const line_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __point_t_encode_array(buf, offset + pos, maxlen - pos, p[element].point, 2);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].confidence), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int line_t_encode(void *buf, int offset, int maxlen, const line_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __line_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __line_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __line_t_encoded_array_size(const line_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __point_t_encoded_array_size(p[element].point, 2);

        size += __int8_t_encoded_array_size(&(p[element].confidence), 1);

    }
    return size;
}

int line_t_encoded_size(const line_t *p)
{
    return 8 + __line_t_encoded_array_size(p, 1);
}

int __line_t_decode_array(const void *buf, int offset, int maxlen, line_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __point_t_decode_array(buf, offset + pos, maxlen - pos, p[element].point, 2);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].confidence), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int __line_t_decode_array_cleanup(line_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __point_t_decode_array_cleanup(p[element].point, 2);

        __int8_t_decode_array_cleanup(&(p[element].confidence), 1);

    }
    return 0;
}

int line_t_decode(const void *buf, int offset, int maxlen, line_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __line_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __line_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int line_t_decode_cleanup(line_t *p)
{
    return __line_t_decode_array_cleanup(p, 1);
}

int __line_t_clone_array(const line_t *p, line_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __point_t_clone_array(p[element].point, q[element].point, 2);

        __int8_t_clone_array(&(p[element].confidence), &(q[element].confidence), 1);

    }
    return 0;
}

line_t *line_t_copy(const line_t *p)
{
    line_t *q = (line_t*) malloc(sizeof(line_t));
    __line_t_clone_array(p, q, 1);
    return q;
}

void line_t_destroy(line_t *p)
{
    __line_t_decode_array_cleanup(p, 1);
    free(p);
}

int line_t_publish(lcm_t *lc, const char *channel, const line_t *p)
{
      int max_data_size = line_t_encoded_size (p);
      uint8_t *buf = (uint8_t*) malloc (max_data_size);
      if (!buf) return -1;
      int data_size = line_t_encode (buf, 0, max_data_size, p);
      if (data_size < 0) {
          free (buf);
          return data_size;
      }
      int status = lcm_publish (lc, channel, buf, data_size);
      free (buf);
      return status;
}

struct _line_t_subscription_t {
    line_t_handler_t user_handler;
    void *userdata;
    lcm_subscription_t *lc_h;
};
static
void line_t_handler_stub (const lcm_recv_buf_t *rbuf,
                            const char *channel, void *userdata)
{
    int status;
    line_t p;
    memset(&p, 0, sizeof(line_t));
    status = line_t_decode (rbuf->data, 0, rbuf->data_size, &p);
    if (status < 0) {
        fprintf (stderr, "error %d decoding line_t!!!\n", status);
        return;
    }

    line_t_subscription_t *h = (line_t_subscription_t*) userdata;
    h->user_handler (rbuf, channel, &p, h->userdata);

    line_t_decode_cleanup (&p);
}

line_t_subscription_t* line_t_subscribe (lcm_t *lcm,
                    const char *channel,
                    line_t_handler_t f, void *userdata)
{
    line_t_subscription_t *n = (line_t_subscription_t*)
                       malloc(sizeof(line_t_subscription_t));
    n->user_handler = f;
    n->userdata = userdata;
    n->lc_h = lcm_subscribe (lcm, channel,
                                 line_t_handler_stub, n);
    if (n->lc_h == NULL) {
        fprintf (stderr,"couldn't reg line_t LCM handler!\n");
        free (n);
        return NULL;
    }
    return n;
}

int line_t_subscription_set_queue_capacity (line_t_subscription_t* subs,
                              int num_messages)
{
    return lcm_subscription_set_queue_capacity (subs->lc_h, num_messages);
}

int line_t_unsubscribe(lcm_t *lcm, line_t_subscription_t* hid)
{
    int status = lcm_unsubscribe (lcm, hid->lc_h);
    if (0 != status) {
        fprintf(stderr,
           "couldn't unsubscribe line_t_handler %p!\n", hid);
        return -1;
    }
    free (hid);
    return 0;
}
