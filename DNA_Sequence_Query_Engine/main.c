#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float frq;
    char *dna;   // entire DNA sequence stored as one string
    size_t len;
    unsigned long long *hpref; // prefix hashes (length+1)
} DNA;

// Aho-Corasick trie node
typedef struct ACNode {
    int next[4];
    int fail;
    int *out; int out_cnt; int out_cap;
} ACNode;

unsigned long long *global_pow = NULL;
size_t global_pow_len = 0;

// Forward declarations
void build_trie(char **queries, size_t *qlens, int qcount, ACNode **nodes_out, int *ncnt_out);
void scan_all_samples(ACNode *nodes, DNA *data_DNA, int count,
                      size_t *qlens, int *total_counts,
                      char ***stored, int *stored_cnt, int *last_seen_at);
void print_and_free_results(int qcount, int *total_counts, char ***stored, int *stored_cnt);

void skip_whitespace(char **line_ptr) {
    while (**line_ptr == ' ' || **line_ptr == '\t') {
        (*line_ptr)++;
    }
}

float parse_frequency(char **ptr) {
    skip_whitespace(ptr);
    char *endptr;
    float frq = strtof(*ptr, &endptr);
    if (endptr == *ptr) {
        return -1.0;  // Error signal, don't exit
    }
    *ptr = endptr;
    return frq;
}

char *parse_dna_string(char **ptr, size_t *dna_len) {
    skip_whitespace(ptr);
    *dna_len = strcspn(*ptr, " \t\n");
    if (*dna_len == 0) {
        return NULL;  // Error signal
    }
    return *ptr;
}

void validate_dna(const char *dna, size_t dna_len) {
    for (size_t i = 0; i < dna_len; i++) {
        char c = dna[i];
        if (c != 'A' && c != 'T' && c != 'C' && c != 'G') {
            printf("Nespravny vstup.\n");
            exit(1);
        }
    }
    if (dna_len % 3 != 0) {
        printf("Nespravny vstup.\n");
        exit(1);
    }
}

int read_database(DNA **data_DNA){
    int count = 0;
    int cap = 16;
    *data_DNA = (DNA*)malloc(sizeof(DNA) * cap);

    char *line = NULL;
    size_t len = 0;
    ssize_t r;

    while ((r = getline(&line, &len, stdin)) > 0) {
        if (r == 1 && line[0] == '\n') break;

        char *ptr = line;

        float frq = parse_frequency(&ptr);
        if (frq < 0.0) {
            return -1;  // Error parsing frequency
        }

        skip_whitespace(&ptr);
        if (*ptr != ':') {
            return -1;  // Error in database
        }
        ptr++;

        size_t dna_len;
        char *dna_ptr = parse_dna_string(&ptr, &dna_len);
        if (dna_ptr == NULL) {
            return -1;  // Error parsing DNA
        }

        // Validate DNA without exiting
        int valid = 1;
        for (size_t i = 0; i < dna_len; i++) {
            char c = dna_ptr[i];
            if (c != 'A' && c != 'T' && c != 'C' && c != 'G') {
                valid = 0;
                break;
            }
        }
        if (!valid || dna_len % 3 != 0) {
            return -1;  // Error in database
        }

        if (count >= cap) {
            cap *= 2;
            *data_DNA = (DNA*)realloc(*data_DNA, sizeof(DNA) * cap);
        }

        (*data_DNA)[count].frq = frq;
        (*data_DNA)[count].dna = strndup(dna_ptr, dna_len);
        (*data_DNA)[count].len = dna_len;
        (*data_DNA)[count].hpref = NULL;
        count++;
    }

    if (count == 0) {
        printf("Nespravny vstup.\n");
        exit(1);
    }

    free(line);
    return count;
}

// Compare by frequency descending
int cmp_freq_desc(const void *a, const void *b) {
    DNA *da = (DNA*)a;
    DNA *db = (DNA*)b;
    if (da->frq < db->frq) return 1;
    if (da->frq > db->frq) return -1;
    return 0;
}

int validate_query(char *ptr, size_t qlen) {
    if (qlen == 0) return 0;

    for (size_t i = 0; i < qlen; i++) {
        char c = ptr[i];
        if (c != 'A' && c != 'T' && c != 'C' && c != 'G')
            return 0;
    }

    if (qlen % 3 != 0)
        return 0;

    return 1;
}

// map nucleotide to small integer
static inline unsigned long long nuc_val(char c) {
    switch (c) {
        case 'A': return 1ULL;
        case 'C': return 2ULL;
        case 'G': return 3ULL;
        case 'T': return 4ULL;
    }
    return 0ULL;
}

// map nucleotide char to index 0..3
static inline int idx_of_char(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
    }
    return -1;
}

// Compute prefix hash for a DNA sample (hpref must be freed later)
void compute_prefix_hash(DNA *sample) {
    size_t n = sample->len;
    sample->hpref = (unsigned long long *)malloc(sizeof(unsigned long long) * (n + 1));
    sample->hpref[0] = 0ULL;
    for (size_t i = 0; i < n; ++i) {
        sample->hpref[i+1] = sample->hpref[i] * 131ULL + nuc_val(sample->dna[i]);
    }
}

// Check if query occurs at any codon-aligned position (start positions step by 3) using rolling hash
int match_with_hash(const DNA *sample, const char *query, size_t qlen, const unsigned long long *pow_base) {
    // kept for compatibility but not used when Aho-Corasick is active
    if (qlen > sample->len) return 0;
    unsigned long long qhash = 0ULL;
    for (size_t i = 0; i < qlen; ++i) qhash = qhash * 131ULL + nuc_val(query[i]);

    size_t n = sample->len;
    unsigned long long *hp = sample->hpref;
    unsigned long long pow_q = pow_base[qlen];

    for (size_t i = 0; i + qlen <= n; i += 3) {
        unsigned long long sub = hp[i+qlen] - hp[i] * pow_q;
        if (sub == qhash) {
            if (memcmp(sample->dna + i, query, qlen) == 0) return 1;
        }
    }
    return 0;
}

void print_matches(DNA *data_DNA, int count, const char *query, size_t qlen, const unsigned long long *pow_base) {
    int total_matches = 0;
    int store_cap = 50;
    char **stored = (char**)malloc(sizeof(char*) * store_cap);
    int stored_count = 0;

    for (int i = 0; i < count; ++i) {
        if (data_DNA[i].hpref == NULL) {
            // compute on demand
            compute_prefix_hash(&data_DNA[i]);
        }
        if (match_with_hash(&data_DNA[i], query, qlen, pow_base)) {
            total_matches++;
            if (stored_count < store_cap) {
                stored[stored_count++] = strdup(data_DNA[i].dna);
            }
        }
    }

    printf("Nalezeno: %d\n", total_matches);
    for (int i = 0; i < stored_count; ++i) {
        printf("> %s\n", stored[i]);
        free(stored[i]);
    }
    free(stored);
}

void process_queries(DNA *data_DNA, int count) {
    // Read all queries from stdin first, validate and store them.
    char *line = NULL;
    size_t len = 0;
    ssize_t r;

    // dynamic array for queries
    char **queries = NULL;
    size_t *qlens = NULL;
    int qcap = 0, qcount = 0;
    int error_in_queries = 0;

    // read queries until error or EOF
    while ((r = getline(&line, &len, stdin)) > 0) {
        if (r == 1 && line[0] == '\n') continue; // skip empty lines
        char *ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        size_t qlen = strcspn(ptr, " \t\n");
        
        // validate query
        if (!validate_query(ptr, qlen)) { 
            error_in_queries = 1;
            break;  // Stop reading queries on error
        }
        
        if (qcount >= qcap) { 
            qcap = qcap ? qcap * 2 : 16; 
            queries = (char**)realloc(queries, sizeof(char*) * qcap); 
            qlens = (size_t*)realloc(qlens, sizeof(size_t) * qcap); 
        }
        queries[qcount] = strndup(ptr, qlen);
        qlens[qcount] = qlen;
        qcount++;
    }
    free(line);

    // Build trie, scan samples, print results for valid queries first
    ACNode *nodes = NULL;
    int ncnt = 0;
    
    build_trie(queries, qlens, qcount, &nodes, &ncnt);

    int *total_counts = (int*)calloc(qcount, sizeof(int));
    int *last_seen_at = (int*)malloc(sizeof(int) * qcount);
    for (int i = 0; i < qcount; ++i) last_seen_at[i] = -1;
    char ***stored = (char***)calloc(qcount, sizeof(char**));
    int *stored_cnt = (int*)calloc(qcount, sizeof(int));

    scan_all_samples(nodes, data_DNA, count, qlens, total_counts, stored, stored_cnt, last_seen_at);
    print_and_free_results(qcount, total_counts, stored, stored_cnt);

    // Now print error if there was one
    if (error_in_queries) {
        printf("Nespravny vstup.\n");
    }

    // cleanup
    for (int i = 0; i < ncnt; ++i) free(nodes[i].out);
    free(nodes);
    for (int i = 0; i < qcount; ++i) free(queries[i]);
    free(queries); free(qlens);
    free(total_counts); free(last_seen_at); free(stored); free(stored_cnt);
}

// Build AC trie from queries; nodes_out will be malloc'd and must be freed by caller.
void build_trie(char **queries, size_t *qlens, int qcount, ACNode **nodes_out, int *ncnt_out) {
    ACNode *nodes = NULL; int ncap = 0, ncnt = 0;
    #define NEW_NODE_LOCAL() do { \
        if (ncnt >= ncap) { ncap = ncap ? ncap * 2 : 1024; nodes = (ACNode*)realloc(nodes, sizeof(ACNode) * ncap); } \
        nodes[ncnt].fail = 0; nodes[ncnt].out = NULL; nodes[ncnt].out_cnt = nodes[ncnt].out_cap = 0; \
        for (int _i=0; _i<4; ++_i) { nodes[ncnt].next[_i] = -1; } \
        ++ncnt; } while(0)

    NEW_NODE_LOCAL(); // root
    for (int qi = 0; qi < qcount; ++qi) {
        int cur = 0;
        for (size_t p = 0; p < qlens[qi]; ++p) {
            int id = idx_of_char(queries[qi][p]);
            if (nodes[cur].next[id] == -1) { nodes[cur].next[id] = ncnt; NEW_NODE_LOCAL(); }
            cur = nodes[cur].next[id];
        }
        if (nodes[cur].out_cnt >= nodes[cur].out_cap) {
            nodes[cur].out_cap = nodes[cur].out_cap ? nodes[cur].out_cap * 2 : 4;
            nodes[cur].out = (int*)realloc(nodes[cur].out, sizeof(int) * nodes[cur].out_cap);
        }
        nodes[cur].out[nodes[cur].out_cnt++] = qi;
    }

    // BFS failure construction
    int *queue = (int*)malloc(sizeof(int) * ncnt);
    int qh = 0, qt = 0;
    for (int c = 0; c < 4; ++c) {
        int v = nodes[0].next[c];
        if (v != -1) { nodes[v].fail = 0; queue[qt++] = v; }
        else nodes[0].next[c] = 0;
    }
    while (qh < qt) {
        int v = queue[qh++];
        for (int c = 0; c < 4; ++c) {
            int u = nodes[v].next[c];
            if (u != -1) {
                int f = nodes[v].fail;
                nodes[u].fail = nodes[f].next[c];
                int fnode = nodes[u].fail;
                if (nodes[fnode].out_cnt) {
                    int need = nodes[u].out_cnt + nodes[fnode].out_cnt;
                    if (nodes[u].out_cap < need) { nodes[u].out_cap = need; nodes[u].out = (int*)realloc(nodes[u].out, sizeof(int) * nodes[u].out_cap); }
                    for (int k = 0; k < nodes[fnode].out_cnt; ++k) nodes[u].out[nodes[u].out_cnt++] = nodes[fnode].out[k];
                }
                queue[qt++] = u;
            } else {
                nodes[v].next[c] = nodes[nodes[v].fail].next[c];
            }
        }
    }
    free(queue);
    *nodes_out = nodes; *ncnt_out = ncnt;
}

// Scan all DNA samples with the automaton, count matches and store up to 50 matches per query.
void scan_all_samples(ACNode *nodes, DNA *data_DNA, int count,
                      size_t *qlens, int *total_counts,
                      char ***stored, int *stored_cnt, int *last_seen_at) {
    for (int si = 0; si < count; ++si) {
        const char *s = data_DNA[si].dna; size_t n = data_DNA[si].len; int state = 0;
        for (size_t pos = 0; pos < n; ++pos) {
            int c = idx_of_char(s[pos]); state = nodes[state].next[c];
            if (!nodes[state].out_cnt) continue;
            for (int oi = 0; oi < nodes[state].out_cnt; ++oi) {
                int pid = nodes[state].out[oi];
                size_t plen = qlens[pid];
                if (pos + 1 < plen) continue;
                size_t start = pos + 1 - plen;
                if (start % 3 != 0) continue;
                if (last_seen_at[pid] != si) {
                    last_seen_at[pid] = si; total_counts[pid]++;
                    if (stored_cnt[pid] < 50) { if (!stored[pid]) stored[pid] = (char**)malloc(sizeof(char*) * 50); stored[pid][stored_cnt[pid]++] = strdup(data_DNA[si].dna); }
                }
            }
        }
    }
}

// Print results and free stored strings
void print_and_free_results(int qcount, int *total_counts, char ***stored, int *stored_cnt) {
    for (int qi = 0; qi < qcount; ++qi) {
        printf("Nalezeno: %d\n", total_counts[qi]);
        for (int k = 0; k < stored_cnt[qi]; ++k) { printf("> %s\n", stored[qi][k]); free(stored[qi][k]); }
        free(stored[qi]);
    }
}

int compare_frequency_desc(const void *a, const void *b) {
    DNA *dna_a = (DNA *)a;
    DNA *dna_b = (DNA *)b;
    if (dna_a->frq < dna_b->frq) return 1;
    if (dna_a->frq > dna_b->frq) return -1;
    return 0;
}

int main(void){
    DNA *data_DNA = NULL;
    printf("Databaze DNA:\n");
    int count = read_database(&data_DNA);
    
    // Handle database error
    if (count == -1) {
        printf("Nespravny vstup.\n");
        return 1;
    }
    
    qsort(data_DNA, count, sizeof(DNA), cmp_freq_desc);

    // prepare global power table up to maximum DNA length
    size_t maxlen = 0;
    for (int i = 0; i < count; ++i) if (data_DNA[i].len > maxlen) maxlen = data_DNA[i].len;
    unsigned long long *pow_base = (unsigned long long*)malloc(sizeof(unsigned long long) * (maxlen + 1));
    pow_base[0] = 1ULL;
    for (size_t i = 1; i <= maxlen; ++i) pow_base[i] = pow_base[i-1] * 131ULL;

    // compute prefix hashes for all samples eagerly (trade memory for speed)
    for (int i = 0; i < count; ++i) compute_prefix_hash(&data_DNA[i]);

    printf("Hledani:\n");

    // expose global pow for process_queries defense path
    global_pow = pow_base;
    global_pow_len = maxlen + 1;

    process_queries(data_DNA, count);

    // cleanup
    for (int i = 0; i < count; ++i) {
        free(data_DNA[i].dna);
        free(data_DNA[i].hpref);
    }
    free(data_DNA);
    free(pow_base);
    return 0;
}
