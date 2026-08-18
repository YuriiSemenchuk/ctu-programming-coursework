# Graph Trip Finder

A C++ library function that finds round trips in a weighted directed graph: routes that start and end in a given city, visit no city twice on the path (except returning to the start), and stay within a maximum total cost.

## Files

| File | Description |
| --- | --- |
| `main.cpp` | `findTrips`, graph helpers, and assert-based tests |

## Public API

```cpp
TTRIP * findTrips(const char data[], const char from[], int costMax);
```

- **`data`** — text description of directed edges (see below).
- **`from`** — name of the starting city; must exist in the graph.
- **`costMax`** — maximum allowed trip cost (inclusive).

Returns a singly linked list of `TTRIP` nodes, sorted by increasing `m_Cost`, or `NULL` if no trips exist. Free with `freeTripList()` (provided under `#ifndef __PROGTEST__`).

Each `TTRIP` contains:

| Field | Meaning |
| --- | --- |
| `m_Desc` | Human-readable path, e.g. `Prague -> London -> Prague` |
| `m_Cities` | Number of cities visited on the path (excluding repeated start) |
| `m_Cost` | Total edge cost |
| `m_Next` | Next trip in the list |

## Edge text format

One edge per line:

```text
cost: source -> destination
```

Example:

```text
100: Prague -> London
80: Prague -> Paris
```

Parsed with `sscanf` (`%d : %s -> %s`). City names are up to 100 characters.

## Algorithm

1. Parse edges and build an adjacency list (max 1000 cities, 100 edges per city).
2. Depth-first search from `from`, marking visited cities.
3. When an edge returns to the start with at least one intermediate city, record the trip if cost ≤ `costMax` and the path description is not duplicate.
4. Sort trips by cost and link into a `TTRIP` list.

## Build and run

There is no stdin driver. Local tests live in `main()` behind `#ifndef __PROGTEST__`:

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o graph-trip-finder
./graph-trip-finder
```

Success exits with code 0 when all `assert` checks pass.

## Concepts

Weighted directed graphs, DFS, path enumeration, pruning, sorting, linked lists, manual `malloc` / `free`.
