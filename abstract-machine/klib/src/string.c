#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t c = 0;
  while (*s) {
    s++;
    c++;
  }
  return c;
}

char *strcpy(char *dst, const char *src) {
  char *dst_cp = dst;
  while (*src) {
    *dst_cp = *src;
    dst_cp++;
    src++;
  }
  *dst_cp = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *dst_cp = dst;
  for (size_t i = 0; i < n; i++) {
    if (*src) {
      *dst_cp = *src;
      dst_cp++;
      src++;
    }
    else {
      *dst_cp = '\0';
      dst_cp++;
    }
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *dst_tail = dst + strlen(dst);
  while (*src) {
    *dst_tail = *src;
    dst_tail++;
    src++;
  }
  *dst_tail = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 || *s2) {
    if (*s1 < *s2) {
      return -1;
    }
    else if (*s1 > *s2){
      return 1;
    }
    s1++;
    s2++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (*s1 < *s2) {
      return -1;
    }
    else if (*s1 > *s2){
      return 1;
    }
    else if (!*s1 && !*s2) {
      return 0;
    }
    s1++;
    s2++;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  uint8_t* t = s;
  for (size_t i = 0; i < n; i++) {
    *t = c;
    t++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  uint8_t* pDst = dst;
  const uint8_t* pSrc = src;
  uint8_t t[n];
  uint8_t* pT = t;
  size_t i;
  for (i = 0; i < n; i++) {
    *pT = *pSrc;
    pT++;
    pSrc++;
  }
  for (i = 0; i < n; i++) {
    *pDst = *pT;
    pT++;
    pDst++;
  }
  free(t);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  uint8_t* pDst = out;
  const uint8_t* pSrc = in;
  for (size_t i = 0; i < n; i++) {
    *pDst = *pSrc;
    pDst++;
    pSrc++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t* p1 = s1;
  const uint8_t* p2 = s2;
  for (size_t i = 0; i < n; i++) {
    if (*p1 < *p2) {
      return -1;
    }
    else if (*p1 > *p2){
      return 1;
    }
    p1++;
    p2++;
  }
  return 0;
}
#endif
