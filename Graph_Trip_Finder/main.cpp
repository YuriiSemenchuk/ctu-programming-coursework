#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct TTrip{
  struct TTrip * m_Next;
  char         * m_Desc;
  int            m_Cities;
  int            m_Cost;
} TTRIP;

TTRIP * makeTrip ( char    desc[],
                   int     cities,
                   int     cost,
                   TTRIP * next ){
  TTRIP * res = (TTRIP *) malloc ( sizeof ( *res ) );
  res -> m_Next = next;
  res -> m_Desc = desc;
  res -> m_Cities = cities;
  res -> m_Cost = cost;
  return res;
}

void freeTripList ( TTRIP * x ){
  while ( x ){
    TTRIP * tmp = x -> m_Next;
    free ( x -> m_Desc );
    free ( x );
    x = tmp;
  }
}
#endif /* __PROGTEST__ */

#define CITY_NAME_MAX 100
#define MAX_CITIES 1000
#define MAX_ADJ 100

struct Edge {
    char from[CITY_NAME_MAX + 1];
    char to[CITY_NAME_MAX + 1];
    int cost;
};

struct AdjEdge {
    int to;
    int cost;
};

struct City {
    char name[CITY_NAME_MAX + 1];
    struct AdjEdge adj[MAX_ADJ];
    int adjCount;
};

typedef struct {
    TTRIP ** data;
    size_t size;
    size_t capacity;
} TripArray;

void tripArrayInit(TripArray * arr) {
    arr->size = 0;
    arr->capacity = 64;
    arr->data = (TTRIP **) malloc(arr->capacity * sizeof(TTRIP *));
}

void tripArrayAdd(TripArray * arr, TTRIP * trip) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (TTRIP **) realloc(arr->data, arr->capacity * sizeof(TTRIP *));
    }
    arr->data[arr->size++] = trip;
}

void tripArrayFree(TripArray * arr) {
    free(arr->data);
}

// ---------- Check duplicate manually ----------
int isDuplicate(TripArray * trips, const char * newDesc) {
    for (size_t i = 0; i < trips->size; i++) {
        if (strcmp(trips->data[i]->m_Desc, newDesc) == 0)
            return 1;
    }
    return 0;
}

// ---------- Parse edges ----------
int parseEdges(const char * data, struct Edge edges[], int maxEdges) {
    int count = 0;
    const char * p = data;
    while (*p && count < maxEdges) {
        int cost;
        char from[CITY_NAME_MAX + 1], to[CITY_NAME_MAX + 1];
        int read = sscanf(p, " %d : %100s -> %100s", &cost, from, to);
        if (read != 3) break;
        edges[count].cost = cost;
        strcpy(edges[count].from, from);
        strcpy(edges[count].to, to);
        count++;
        const char * nl = strchr(p, '\n');
        if (!nl) break;
        p = nl + 1;
    }
    return count;
}

// ---------- City helpers ----------
int getCityIndex(struct City cities[], int * cityCount, const char * name) {
    for (int i = 0; i < *cityCount; i++)
        if (strcmp(cities[i].name, name) == 0) return i;
    strcpy(cities[*cityCount].name, name);
    cities[*cityCount].adjCount = 0;
    return (*cityCount)++;
}

void buildGraph(struct Edge edges[], int edgeCount, struct City cities[], int * cityCount) {
    *cityCount = 0;
    for (int i = 0; i < edgeCount; i++) {
        int from = getCityIndex(cities, cityCount, edges[i].from);
        int to = getCityIndex(cities, cityCount, edges[i].to);
        cities[from].adj[cities[from].adjCount].to = to;
        cities[from].adj[cities[from].adjCount].cost = edges[i].cost;
        cities[from].adjCount++;
    }
}

// ---------- DFS ----------
void dfs(int cur, int start, int cost, int costMax,
         struct City cities[], int visited[],
         int path[], int pathLen, TripArray * trips) {
    for (int i = 0; i < cities[cur].adjCount; i++) {
        int next = cities[cur].adj[i].to;
        int edgeCost = cities[cur].adj[i].cost;

        if (cost + edgeCost > costMax) continue;

        if (next == start && pathLen >= 1) {
            char buffer[2048] = "";
            for (int j = 0; j < pathLen; j++) {
                strcat(buffer, cities[path[j]].name);
                strcat(buffer, " -> ");
            }
            strcat(buffer, cities[start].name);

            if (!isDuplicate(trips, buffer)) {
                char * desc = (char *) malloc(strlen(buffer) + 1);
                strcpy(desc, buffer);
                TTRIP * trip = makeTrip(desc, pathLen, cost + edgeCost, NULL);
                tripArrayAdd(trips, trip);
            }
            continue;
        }

        if (!visited[next]) {
            visited[next] = 1;
            path[pathLen] = next;
            dfs(next, start, cost + edgeCost, costMax, cities, visited, path, pathLen + 1, trips);
            visited[next] = 0;
        }
    }
}

// ---------- Sorting ----------
int cmpTrip(const void * a, const void * b) {
    TTRIP * t1 = *(TTRIP **)a;
    TTRIP * t2 = *(TTRIP **)b;
    return t1->m_Cost - t2->m_Cost;
}

TTRIP * buildLinkedList(TripArray * trips) {
    if (trips->size == 0) return NULL;
    qsort(trips->data, trips->size, sizeof(TTRIP *), cmpTrip);
    for (size_t i = 0; i < trips->size - 1; i++)
        trips->data[i]->m_Next = trips->data[i+1];
    trips->data[trips->size-1]->m_Next = NULL;
    return trips->data[0];
}

TTRIP * findTrips(const char data[], const char from[], int costMax) {
    struct Edge edges[2000];
    int edgeCount = parseEdges(data, edges, 2000);
    if (edgeCount <= 0) return NULL;

    struct City cities[MAX_CITIES];
    int cityCount;
    buildGraph(edges, edgeCount, cities, &cityCount);

    int start = -1;
    for (int i = 0; i < cityCount; i++)
        if (strcmp(cities[i].name, from) == 0) { start = i; break; }
    if (start == -1) return NULL;

    int visited[MAX_CITIES] = {0};
    int path[MAX_CITIES];
    visited[start] = 1;
    path[0] = start;

    TripArray trips;
    tripArrayInit(&trips);

    dfs(start, start, 0, costMax, cities, visited, path, 1, &trips);

    TTRIP * result = buildLinkedList(&trips);
    tripArrayFree(&trips);
    return result;
}

#ifndef __PROGTEST__
int main ()
{
  const char * data0 = R"(
100: Prague -> London
80: Prague -> Paris
90: Paris -> London
75: London -> Madrid
95: Madrid -> Prague
1000: London -> Prague
50: Berlin -> Prague
80: Madrid -> Berlin
90: Rome -> Prague
100: Wien -> Rome
90: Prague -> Lisabon
80: Lisabon -> Dublin
)";
  const char * data1 = R"(
100: Prague -> London
107: London -> Prague
80: Prague -> Paris
78: Paris -> Prague
38: Paris -> London
43: London -> Paris
69: Prague -> Wien
89: London -> Wien
73: Paris -> Wien
63: Wien -> Prague
82: Wien -> London
77: Wien -> Paris
163: Zagreb -> Tallinn
282: Tallinn -> Lisboa
377: Lisboa -> Zagreb
)";
  TTRIP * t;
  t = findTrips ( data0, "Prague", 300 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 700 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 1100 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 5000 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1170 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "London", 2000 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "London -> Madrid -> Prague -> London" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "London -> Madrid -> Berlin -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "London -> Madrid -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Madrid -> Berlin -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1170 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Prague", 270 );
  assert ( t );
  assert ( t -> m_Cost == 132 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> Wien -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 158 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 207 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 216 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 221 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 224 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Wien -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 225 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Wien -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Wien", 270 );
  assert ( t );
  assert ( t -> m_Cost == 132 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "Wien -> Prague -> Wien" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 150 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Wien -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 171 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 198 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 204 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Paris -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 216 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 224 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Paris -> Prague -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Prague -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> Paris -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "London", 400 );
  assert ( t );
  assert ( t -> m_Cost == 81 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "London -> Paris -> London" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 171 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "London -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 198 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 204 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 207 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 221 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 225 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 272 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Prague -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 279 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Wien -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 291 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Wien -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 342 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 344 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Paris -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Lisboa", 1000 );
  assert ( t );
  assert ( t -> m_Cost == 822 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Lisboa -> Zagreb -> Tallinn -> Lisboa" ) );
  assert ( t -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Oslo", 1000 );
  assert ( t == nullptr );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
