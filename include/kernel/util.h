#include <kernel/print.h>

#define reterr(x) do { \
  int ret = (x); \
  if (ret) { \
    return ret; \
  } \
} while (0)

#define reterrm(x, msg) do { \
  int ret = (x); \
  if (ret) { \
    print(msg); \
    return ret; \
  } \
} while (0)
