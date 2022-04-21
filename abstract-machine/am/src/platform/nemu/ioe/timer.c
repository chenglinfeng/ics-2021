#include <am.h>
#include <nemu.h>

AM_TIMER_UPTIME_T sttime;

void __am_timer_init() {
  sttime.us = inl(RTC_ADDR + 4);
  sttime.us <<= 32;
  sttime.us += inl(RTC_ADDR);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  AM_TIMER_UPTIME_T ttime;
  ttime.us = inl(RTC_ADDR + 4);
  ttime.us <<= 32;
  ttime.us += inl(RTC_ADDR);
  uptime->us = ttime.us - sttime.us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
