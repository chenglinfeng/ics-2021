#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char out[256];
  va_list arg;
  va_start(arg, fmt);
  int length = vsprintf(out, fmt, arg);
  va_end(arg);
  putstr(out);
  return length;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int length = 0;
  bool pf = true;
  char* str_p;
  char str_t[64];
  int width = -1;
  int precision = -1;
  char w_fill = ' ';

  precision++; // avoid error for presition is not used yet

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;

     /* flag 
      * -, +, (space), #, 0 
      */
      while(pf) {
        switch(*fmt) {
          case '-': panic("Not implemented"); fmt++; break;
          case '+': panic("Not implemented"); fmt++; break;
          case ' ': panic("Not implemented"); fmt++; break;
          case '#': panic("Not implemented"); fmt++; break;
          case '0': w_fill = '0'; fmt++; break;
          default: pf = false;
        }
      }

     /* width 
      * (num), *
      */
      if (*fmt >= '0' && *fmt <= '9') {
        int i = 0;
        while (*fmt >= '0' && *fmt <= '9') {
          str_t[i] = *fmt;
          i++;
          fmt++;
        }
        str_t[i] = 0;
        width = atoi(str_t);
      } 
      else if (*fmt == '*')
      {
        width = va_arg(ap, int);
        fmt++;
      }

     /* precision 
      * .num, .*
      */
      if (*fmt == '.') {
        fmt++;
        if (*fmt >= '0' && *fmt <= '9') {
          int i = 0;
          while (*fmt >= '0' && *fmt <= '9') {
            str_t[i] = *fmt;
            i++;
            fmt++;
          }
          str_t[i] = 0;
          precision = atoi(str_t);
        } 
        else if (*fmt == '*')
        {
          precision = va_arg(ap, int);
          fmt++;
        }
        else {
          precision = 6;
        }
      }
      
     /* length
      * hh, h, l, ll, j, z, t, L
      * not implemeted yet
      */


     /* specifier
      * d(i), u, o, x, X, f, F, e, E, g, G, a, A, c, s, p, n, %
      */
      size_t len;
      switch(*fmt) {
        case 'd': case 'i':
          itoa(va_arg(ap, long), str_t, 10);
          len = strlen(str_t);
          if (width >= 0 && len < width) {
            memset(out + length, w_fill, width - len);
            length += width - len;
          }
          memcpy(out + length, str_t, len);
          length += len;
          fmt++;
          break;
        case 'u':
          itoa(va_arg(ap, unsigned long), str_t, 10);
          len = strlen(str_t);
          if (width >= 0 && len < width) {
            memset(out + length, w_fill, width - len);
            length += width - len;
          }
          memcpy(out + length, str_t, len);
          length += len;
          fmt++;
          break;
        case 'x':
          itoa(va_arg(ap, unsigned long), str_t, 16);
          len = strlen(str_t);
          if (width >= 0 && len < width) {
            memset(out + length, w_fill, width - len);
            length += width - len;
          }
          memcpy(out + length, str_t, len);
          length += len;
          fmt++;
          break;
        case 's':
          str_p = va_arg(ap, char*);
          len = strlen(str_p);
          memcpy(out + length, str_p, len);
          length += len;
          fmt++;
          break;
        case '%':
          *(out+length) = '%';
          length++;
          fmt++;
          break;
        default: panic("Not implemented");
      }

    }
    else {
      *(out + length) = *fmt;
      length++;
      fmt++;
    }
  }
  *(out + length) = 0;
  return length;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  int length = vsprintf(out, fmt, arg);
  va_end(arg);
  return length;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
