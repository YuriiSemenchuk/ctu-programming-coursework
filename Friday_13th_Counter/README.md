# Friday the 13th Counter

A C program for Gregorian calendar arithmetic on the course-defined calendar (years from 1900, leap rule with a 4000-year exception). It counts Friday-the-13th dates in inclusive ranges and exposes helper functions for neighbouring such dates.

## Files

| File | Description |
| --- | --- |
| `main.c` | Date logic and stdin driver |
| `in/`, `out/` | Seven paired test cases (`0000`–`0006`) |
| `test.sh` | Compiles with `gcc` and diffs output against `out/` |

## Public API

| Function | Description |
| --- | --- |
| `prevFriday13(TDATE *date)` | Move `date` to the previous Friday that falls on the 13th; `false` if none exists from 1900. |
| `nextFriday13(TDATE *date)` | Move `date` to the next Friday the 13th; `false` if none exists. |
| `countFriday13(TDATE from, TDATE to, long long *cnt)` | Count Friday-the-13th dates in `[from, to]`; `false` on invalid or reversed range. |

`TDATE` holds `m_Year`, `m_Month`, `m_Day`. Course helpers `makeDate` and `equalDate` are under `#ifndef __PROGTEST__`.

## Calendar rules

- Valid dates: year ≥ 1900, valid month/day.
- Leap year: `(year % 4 == 0 && year % 100 != 0) || (year % 400 == 0 && year % 4000 != 0)`.
- Large-range counting uses repeating 400-year and 28,000-year Friday-the-13th cycles (`jump()`).

## Input format (`main`)

Stdin: integer `N`, then `N` pairs of dates:

```text
3
2015 2 13
2015 2 13
2015 2 14
2015 2 14
2000 1 1
2005 12 31
```

Each date: `year month day` (space-separated).

## Output format

On success:

```text
<from_year>-<from_month>-<from_day> -> <to_year>-<to_month>-<to_day> = <count>
```

On invalid range or date:

```text
<year>-<month>-<day> -> INVALID DATE
```

## Build and run

```bash
gcc -Wall -Wextra -pedantic main.c -o friday-13th
./friday-13th < in/0000_in.txt
```

Run all tests:

```bash
./test.sh
```

## Concepts

Calendar arithmetic, leap-year edge cases, modular optimisation for long intervals, structs and boolean validation.
