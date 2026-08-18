# Military Base Log Analyzer

A C++ program that models a military base as a weighted undirected graph of zones, parses visitor entry/exit logs (text and binary), and answers audit queries: which people could have been in a target zone during a time window.

## Files

| File | Description |
| --- | --- |
| `main.cpp` | Graph, log parser, interval builder, search |
| `base.txt` | Sample zone topology (travel times between adjacent zones) |
| `in1.log` | Sample log file (text + binary sections) |
| `zadani.txt` | Original assignment specification (Czech) |

**Note:** `main()` also runs tests against `in2.log` and `in3.log`. Only `in1.log` is present in this repository; add the other logs from the course materials to run the full local test suite.

## Data model

### Base topology (`base.txt`)

Each line: two zone names and travel time (minutes, 1–9):

```text
headquarters tacticalRoom 1
headquarters parking#1 1
```

Loaded by `CMilBase::readBase(path)`.

### Log formats

Logs may contain multiple sections. Each section starts with a 4-byte magic:

| Magic | Format |
| --- | --- |
| `TEXT` | Text block: zone name, event count, then lines `YYYY-MM-DD HH:MM person_name` |
| `IIII` (0x49×4) | Little-endian binary records |
| `MMMM` (0x4D×4) | Big-endian binary records |

Binary timestamps are packed into 32 bits (year, month, day, hour, minute). Parsed by `CLogParser::parseLogFile`.

### Audit query

`CVisitorLog::search(CAuditFilter)` returns a `std::set<std::string>` of person names who could have been in the filter’s zone during the optional time bounds.

`CAuditFilter` is built as:

```cpp
CAuditFilter("flyingSaucerHangar")
  .notBefore(2026, 3, 10, 9, 0)
  .notAfter(2026, 3, 10, 13, 0);
```

## Algorithm

1. **Dijkstra** on the zone graph — shortest travel times from the target zone to all zones.
2. Build per-person intervals from enter/exit log events (open interval if exit missing).
3. For each interval, compute earliest arrival and latest departure at the target using travel times; intersect with the filter’s time range.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o log-analyzer
./log-analyzer
```

`main()` loads `base.txt`, processes `in1.log` (and `in2.log`, `in3.log` if available), and runs `basicTests()` with fixed `assert` expectations under `#ifndef __PROGTEST__`.

## Concepts

Graph shortest paths, binary I/O and endianness, timestamp decoding, interval reasoning, `std::map` / `std::set`, file parsing.
