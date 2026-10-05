#include <stdint.h>

void* str_impl_utf8_decode(const void *buf, uint32_t *c){
  static const char lengths[] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
  };
  static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
  static const int shiftc[] = {0, 18, 12, 6, 0};

  unsigned char *s = (unsigned char *)buf;
  int len = lengths[s[0] >> 3];

  unsigned char *next = s + len + !len;

  *c  = (uint32_t)(s[0] & masks[len]) << 18;
  if(len>1) *c |= (uint32_t)(s[1] & 0x3f) << 12;
  if(len>2) *c |= (uint32_t)(s[2] & 0x3f) <<  6;
  if(len>3) *c |= (uint32_t)(s[3] & 0x3f) <<  0;
  *c >>= shiftc[len];

  return next;
}

uint32_t* str_impl_utf8_encode(const uint32_t *cps, unsigned char *out){
  static const unsigned char leads[] = {0x00, 0x00, 0xc0, 0xe0, 0xf0};
  static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
  static const int shiftc[] = {0, 18, 12, 6, 0};

  uint32_t c = cps[0];
  int len = 1 + (c > 0x7f) + (c > 0x7ff) + (c > 0xffff);

  c <<= shiftc[len];

  out[0] = leads[len] | ((c >> 18) & masks[len]);
  out[1] = len > 1 ? 0x80 | ((c >> 12) & 0x3f) : 0;
  out[2] = len > 2 ? 0x80 | ((c >>  6) & 0x3f) : 0;
  out[3] = len > 3 ? 0x80 | ((c >>  0) & 0x3f) : 0;

  return (uint32_t *)cps + 1;
}