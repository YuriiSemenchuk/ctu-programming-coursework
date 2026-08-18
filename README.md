# CTU Programming Coursework

A collection of C and C++ coursework projects from the Faculty of Information Technology, Czech Technical University in Prague (CTU / ČVUT).

The projects cover data structures, algorithms, object-oriented design, manual memory management, and generic programming. Source files use the `__PROGTEST__` conditional-compilation pattern from the course platform; local test harnesses are included where available.

## Repository layout

```
ctu-programming-coursework/
├── DNA_Sequence_Query_Engine/      # C — multi-pattern DNA matching
├── Friday_13th_Counter/            # C — Gregorian calendar / Friday the 13th
├── Graph_Trip_Finder/              # C++ — weighted graph round-trip search
├── Versioned_Memory_File_System/   # C++ — in-memory file with versions (COW)
├── Military_Base_Log_Analyzer/     # C++ — log parsing and zone reachability
├── Hierarchical_DNS_Zone_Model/    # C++ — DNS zone hierarchy and lookup
└── README.md
```

## Projects

| Project | Language | Main file | Local tests |
| --- | --- | --- | --- |
| [DNA Sequence Query Engine](DNA_Sequence_Query_Engine) | C | `main.c` | `test.sh` — 5 stdin I/O cases (`in/`, `out/`) |
| [Friday the 13th Counter](Friday_13th_Counter) | C | `main.c` | `test.sh` — 7 stdin I/O cases (`in/`, `out/`) |
| [Graph Trip Finder](Graph_Trip_Finder) | C++ | `main.cpp` | Assert harness in `main()` (`#ifndef __PROGTEST__`) |
| [Versioned Memory File System](Versioned_Memory_File_System) | C++ | `main.cpp` | Interactive menu: auto asserts or manual REPL |
| [Military Base Log Analyzer](Military_Base_Log_Analyzer) | C++ | `main.cpp` | Assert harness; expects `base.txt` and log files |
| [Hierarchical DNS Zone Model](Hierarchical_DNS_Zone_Model) | C++ | `main.cpp`, `ipaddress.h` | Assert harness in `main()` |

See each project’s README for input formats, APIs, and run instructions.

## Building

Each project is a single translation unit with no external dependencies.

**C++ (typical):**

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o project
```

**C (typical):**

```bash
gcc -Wall -Wextra -pedantic main.c -o project
```

The bundled `test.sh` scripts for C projects use `gcc` with `-Wall -Wextra -pedantic`. Project READMEs list exact commands and binary names.

## Notes

These are educational assignments, not production software. Assignment specifications (`zadani.txt`) and sample data are included only where they were already part of the local project files.

## Author

Yurii Semenchuk · [GitHub](https://github.com/YuriiSemenchuk) · [GitLab](https://gitlab.fit.cvut.cz/semenyu1)
