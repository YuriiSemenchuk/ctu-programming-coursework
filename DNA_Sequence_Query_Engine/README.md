# DNA Sequence Query Engine

A C command-line program that searches a DNA sample database for multiple query sequences. Samples are ranked by frequency; matches are reported per query using an Aho–Corasick automaton with codon-aligned scanning.

## Files

| File | Description |
| --- | --- |
| `main.c` | Full implementation and `main()` |
| `in/`, `out/` | Five paired test inputs and expected outputs (`0000`–`0004`) |
| `test.sh` | Compiles with `gcc` and diffs output against `out/` |

## Behaviour

1. Read DNA samples from stdin until a blank line.
2. Sort samples by frequency (descending).
3. Read query sequences from stdin until EOF or the first invalid query.
4. For each valid query, count samples that contain the query at a **codon-aligned** position (start index divisible by 3).
5. Print match count and up to 50 matching sample sequences per query.

Validation rules:

- Nucleotides must be `A`, `C`, `G`, or `T`.
- Sample and query lengths must be divisible by 3.
- On database format errors, the program prints `Nespravny vstup.` and exits.
- Invalid queries are processed after valid ones; `Nespravny vstup.` is printed once at the end if a bad query was seen.

## Algorithm

- **Aho–Corasick trie** over all queries, with BFS failure links.
- Single left-to-right scan of each sample; only codon-aligned match starts are counted.
- Samples sorted with `qsort` by frequency before matching.
- Prefix hashes are also computed on samples (support code); query matching uses the automaton in `process_queries()`.

## Input format

Database lines (one per line, until empty line):

```text
frequency:DNA_SEQUENCE
```

Whitespace around the colon is allowed. Example:

```text
0.85 : ATGCGATAC
0.50 : ATGATGCCC

ATG
CCC
```

After the blank line, each non-empty line is one query sequence.

## Output format

```text
Databaze DNA:
Hledani:
Nalezeno: <count>
> <matching_sample_1>
> <matching_sample_2>
...
```

Up to 50 `>` lines per query. Czech labels match the course assignment.

## Build and run

```bash
gcc -Wall -Wextra -pedantic main.c -o dna-query
./dna-query < in/0000_in.txt
```

Run all tests:

```bash
./test.sh
```

## Concepts

Multi-pattern string matching, trie automata, dynamic memory (`malloc` / `realloc` / `free`), sorting, stdin parsing.
