# Hierarchical DNS Zone Model

A C++ object-oriented model of DNS zones and records. Zones can nest; lookup walks the domain hierarchy and returns matching records.

## Files

| File | Description |
| --- | --- |
| `main.cpp` | Record types, `CZone`, tests |
| `ipaddress.h` | `CIPv4` and `CIPv6` types used by address records |

## Record types

| Type | Class | Purpose |
| --- | --- | --- |
| `A` | `CRecA` | IPv4 address |
| `AAAA` | `CRecAAAA` | IPv6 address |
| `MX` | `CRecMX` | Mail exchange |
| `CNAME` | `CRecCNAME` | Canonical name |
| `SPF` | `CRecSPF` | SPF text |
| Zone | `CZone` | Nested subdomain |

All records inherit from `CEntry` with virtual `type()`, `print()`, `clone()`, and `equals()`.

## Main classes

- **`CEntry`** — abstract base; polymorphic printing and deep copy via `clone()`.
- **`CZone`** — container of `std::shared_ptr<CEntry>`; supports `add`, `del`, `search`, and tree-style `print`.
- **`CSearchResult`** — ordered view of search hits; indexable and stream-printable.

### Lookup

`CZone::search(std::string q)` splits `q` on `.` and descends into child zones, then collects records at the final label. Returns empty result if the path is invalid.

### Collision rules

- A name cannot hold both a `CNAME` and other records, nor a zone and conflicting entries.
- Duplicate identical records are rejected on `add`.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o dns-zone-model
./dns-zone-model
```

`main()` runs extensive `assert` tests under `#ifndef __PROGTEST__` (zone construction, search, add/del, printing). No stdin interface.

## Concepts

Inheritance, polymorphism, cloning, `std::shared_ptr`, hierarchical search, DNS-style naming, stream formatting.
