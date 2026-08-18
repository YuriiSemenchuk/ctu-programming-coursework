#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef struct TDate
{
  unsigned       m_Year;
  unsigned short m_Month;
  unsigned short m_Day;
} TDATE;

TDATE makeDate ( unsigned       y,
                 unsigned short m,
                 unsigned short d )
{
  TDATE res = { y, m, d };
  return res;
}


bool  equalDate ( TDATE a,
                  TDATE b )
{
  return a . m_Year == b . m_Year
         && a . m_Month == b . m_Month
         && a . m_Day == b . m_Day;
}
#endif /* __PROGTEST__ */


bool is_leap_year(unsigned year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0 && year % 4000 != 0);
}


bool is_Bad_Date(TDATE date){
  if (date.m_Year < 1900 || date.m_Month < 1 || date.m_Month > 12 || date.m_Day < 1)
    return true;

  unsigned short days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

 if (is_leap_year(date.m_Year))
    days_in_month[1] = 29;

  if (date.m_Day > days_in_month[date.m_Month - 1])
    return true;

  return false;
}

bool is_friday(TDATE date){
  int d = date.m_Day;
  int m = date.m_Month;
  int y = date.m_Year;
  if (m == 1 || m == 2) {
    m += 12;
    y -= 1;
  }
  int K = y % 100;
  int J = y / 100;
  int leap_correction = y / 4000;
  if ((y < 4000 * leap_correction) ||
      (y == 4000 * leap_correction && m < 2)) {
    leap_correction -= 1;
  }
  int h = ((d + 13 * (m + 1) / 5 + K + K / 4 + J / 4 + 5 * J) - leap_correction) % 7;
  /* h = 0 Saturday, 1 Sunday, ..., 6 Friday */
  return (h == 6);
}


unsigned jump(TDATE from, TDATE to, long long *count) {
  // Friday counts per 400-year period repeat every 7 cycles (28,000 years)
  const int millenium_fridays[] = {688, 684, 687, 685, 685, 687, 684};

  // Early exit if we can't jump at least 400 years
  if (from.m_Year + 400 > to.m_Year) {
    return from.m_Year;
  }

  unsigned year = from.m_Year;

  while (year + 28000 <= to.m_Year) {
    unsigned newYear = year + 28000;
    if (newYear == to.m_Year &&
        (to.m_Month < from.m_Month ||
         (to.m_Month == from.m_Month && to.m_Day < from.m_Day))) {
      break;
    }
    *count += 48000;
    year = newYear;
  }

  // Jump forward in 400-year increments as long as possible
  while (year + 400 < to.m_Year) {
    // Don't cross 4000-year boundaries (leap year rules change)
    if ((year / 4000) != ((year + 400) / 4000)) {
      break;
    }

    // Determine which cycle (0-6) repeats every 7 cycles
    int cycle = (year / 4000) % 7;

    // Add the Friday count for this 400-year period
    *count += millenium_fridays[cycle];
    
    year += 400;

    // Check if we've gone past the target date
    if (year > to.m_Year) {
      break;
    }
    if (year == to.m_Year && 
        (to.m_Month < from.m_Month ||
         (to.m_Month == from.m_Month && to.m_Day < from.m_Day))) {
      break;
    }
  }

  return year;
}


TDATE this_or_next_friday_13(TDATE date) {
  TDATE d = date;
  if (d.m_Day > 13){ // > 13 because we want to find the THIS or NEXT Friday the 13th
    d.m_Month++;
    if (d.m_Month == 13){
      d.m_Month = 1;
      d.m_Year++;
    }
  }
  d.m_Day = 13;

  while (true){
    if (is_friday(d)){
      return d;
    }
    d.m_Month++;
    if (d.m_Month == 13){
      d.m_Month = 1;
      d.m_Year++;
    }
  }
}

bool prevFriday13 ( TDATE * date ){
  TDATE d = *date;
  if (is_Bad_Date(d))
    return false;

  if (d.m_Day <= 13){ // <= 13 because we want to find the PREVIOUS Friday the 13th
    d.m_Month--;
    if (d.m_Month == 0){
      d.m_Month = 12;
      d.m_Year--;
    }
  }
  d.m_Day = 13;

  while (d.m_Year >= 1900){
    if (is_friday(d)){
      *date = d;
      return true;
    }
    d.m_Month--;
    if (d.m_Month == 0){
      d.m_Month = 12;
      d.m_Year--;
    }
  }
  return false;
}

bool nextFriday13 ( TDATE * date ){
  TDATE d = *date;
  if (is_Bad_Date(d))
    return false;

  if (d.m_Day >= 13){ // >= 13 because we want to find the NEXT Friday the 13th
    d.m_Month++;
    if (d.m_Month == 13){
      d.m_Month = 1;
      d.m_Year++;
    }
  }
  d.m_Day = 13;

  while (true){
    if (is_friday(d)){
      *date = d;
      return true;
    }
    d.m_Month++;
    if (d.m_Month == 13){
      d.m_Month = 1;
      d.m_Year++;
    }
  }
}

bool countFriday13 ( TDATE from, TDATE to, long long int * cnt ){
  if (is_Bad_Date(from) || is_Bad_Date(to))
    return false;

  if (from.m_Year > to.m_Year
      || (from.m_Year == to.m_Year && from.m_Month > to.m_Month)
      || (from.m_Year == to.m_Year && from.m_Month == to.m_Month && from.m_Day > to.m_Day))
    return false;

  if (equalDate(from, to)) {
    *cnt = (is_friday(from) && from.m_Day == 13) ? 1 : 0;
    return true;
  }

  long long int count = 0;
  TDATE date = this_or_next_friday_13(from);

  while (date.m_Year < to.m_Year
         || (date.m_Year == to.m_Year && date.m_Month < to.m_Month)
         || (date.m_Year == to.m_Year && date.m_Month == to.m_Month && date.m_Day <= to.m_Day)) {
    count++;

    date.m_Year = jump(date, to, &count);

    if (nextFriday13(&date) == false)
      break;
  }
  *cnt = count;
  return true;
}

#ifndef __PROGTEST__


int main(void) {
  /* Read from stdin using scanf/printf only (no fopen/fclose) */
  int n = 0;
  if (scanf("%d", &n) != 1) {
    fprintf(stderr, "Missing or invalid count in input\n");
    return EXIT_FAILURE;
  }

  long long cnt = 0;
  unsigned y = 0, y2 = 0;
  unsigned short mo = 0, mo2 = 0, d = 0, d2 = 0;
  TDATE from, to;

  for (int i = 0; i < n; i++) {
    /* Use plain scanf to read the two dates */
    y = y2 = 0;
    mo = mo2 = d = d2 = 0;
    int r1 = scanf("%u %hu %hu", &y, &mo, &d);
    int r2 = scanf("%u %hu %hu", &y2, &mo2, &d2);

    if (r1 != 3 || r2 != 3) {
      /* Failed to read a full pair -> print INVALID DATE for what we read */
      printf("%u-%hu-%hu -> INVALID DATE\n", y, mo, d);
      /* If input is malformed further, continue to attempt remaining reads */
      continue;
    }

    from = makeDate(y, mo, d);
    to = makeDate(y2, mo2, d2);

    if (countFriday13(from, to, &cnt)) {
      printf("%u-%hu-%hu -> %u-%hu-%hu = %lld\n", y, mo, d, y2, mo2, d2, cnt);
    } else {
      printf("%u-%hu-%hu -> INVALID DATE\n", y, mo, d);
    }
  }

  return 0;
}
#endif /* __PROGTEST__ */
