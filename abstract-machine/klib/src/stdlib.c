#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

char* itoa(long val, char* str, int base) {
  /* base = 1, 2, ..., 10, 16 */
  uint8_t buf[64];
  int idx = 0;
  char* pStr = str;
  unsigned long val_u;
  if (val < 0) {
    val_u = -val;
    *pStr = '-';
    pStr++;
  }
  else{
    val_u = val;
  }

  do {
    buf[idx] = val_u % base;
    val_u /= base;
    idx++;
  } while (val_u);

  for (; idx--;) {
    if (buf[idx] > 9) {
      *pStr = buf[idx] - 10 + 'a';
    }
    else {
      *pStr = buf[idx] + '0';
    }
    pStr++;
  }
  *pStr = '\0';
  return str;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
