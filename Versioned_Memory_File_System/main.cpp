#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <initializer_list>
#include <algorithm>
#endif /* __PROGTEST__ */

class Chunk
{
public:
    uint8_t *data;
    size_t used;
    size_t cap;
    int refCount;
    explicit Chunk(size_t cap) : data(cap ? new uint8_t[cap] : nullptr), used(0), cap(cap), refCount(1) {}

    ~Chunk() { delete[] data; }

    static Chunk *create(size_t cap) { return new Chunk(cap); }

    void acquire() { ++refCount; }

    void release() {
        if (--refCount > 0)
            return;
        delete this;
    }
};

class Content
{
public:
    Chunk **chunks;
    size_t chunkCount;
    size_t chunkCapacity;
    size_t logicalSize;
    int refCount;

    size_t allocatedSize;
    size_t minChunkSize;

    Content(size_t k = 64) : chunks(nullptr), chunkCount(0),
                                chunkCapacity(0),
                                logicalSize(0), refCount(1),
                                allocatedSize(0),
                                minChunkSize(k) {}

    ~Content() {
        for (size_t i = 0; i < chunkCount; ++i)
            chunks[i]->release();
        delete[] chunks;
    }

    static Content *createEmpty(size_t k = 2048) { return new Content(k); }

 
    void acquire() { ++refCount; }


    void release() {
        if (--refCount > 0)
            return;
        delete this;
    }

    size_t physicalLen() const { return allocatedSize; }

    //Compute the capacity for a new chunk based on requested need and minimum chunk size.
    size_t computeChunkCapacity(size_t need) const { return need > minChunkSize ? need : minChunkSize; }

    void ensureChunkCapacity() {
        if (chunkCount < chunkCapacity)
            return;
        size_t nc = chunkCapacity ? chunkCapacity * 2 : 4;
        Chunk **nq = new Chunk *[nc];
        for (size_t i = 0; i < chunkCount; ++i)
            nq[i] = chunks[i];
        delete[] chunks;
        chunks = nq;
        chunkCapacity = nc;
    }

    void appendChunk(Chunk *ch) {
        ensureChunkCapacity();
        chunks[chunkCount++] = ch;
        allocatedSize += ch->used;
    }

    // Ensure the chunk at index is unique (copy-on-write) so it can be modified safely.
    void uniqueChunk(size_t idx) {
        Chunk *ch = chunks[idx];
        if (ch->refCount == 1)
            return;
        Chunk *nw = Chunk::create(ch->cap);
        nw->used = ch->used;
        if (ch->used)
            memcpy(nw->data, ch->data, ch->used);
        ch->release();
        chunks[idx] = nw;
    }

    // Ensure the content has enough physical allocation to cover logicalSize;
    // allocate new chunks or extend the last one to reach logicalSize.
    void ensureCapacity() {
        while (allocatedSize < logicalSize) {
            size_t need = logicalSize - allocatedSize;
            if (chunkCount == 0) {
                size_t cap = computeChunkCapacity(need);
                Chunk *ch = Chunk::create(cap);
                size_t add = need < ch->cap ? need : ch->cap;
                ch->used = add;
                appendChunk(ch);
                continue;
            }
            Chunk *last = chunks[chunkCount - 1];
            size_t slack = last->cap - last->used;
            if (slack > 0) {
                uniqueChunk(chunkCount - 1);
                last = chunks[chunkCount - 1];
                size_t add = need < slack ? need : slack;
                last->used += add;
                allocatedSize += add;
            } else {
                size_t cap = computeChunkCapacity(need);
                Chunk *ch = Chunk::create(cap);
                size_t add = need < ch->cap ? need : ch->cap;
                ch->used = add;
                appendChunk(ch);
            }
        }
    }

    // Create a shallow clone of Content: copy chunk pointers and increment their refcounts.
    Content *cloneShallow() const {
        Content *nw = new Content(minChunkSize);
        nw->chunkCount = chunkCount;
        nw->chunkCapacity = chunkCount;
        nw->logicalSize = logicalSize;
        nw->allocatedSize = allocatedSize;
        nw->refCount = 1;
        nw->chunks = chunkCount ? new Chunk *[chunkCount] : nullptr;
        for (size_t i = 0; i < chunkCount; ++i) {
            nw->chunks[i] = chunks[i];
            ++nw->chunks[i]->refCount;
        }
        return nw;
    }

    // Locate the chunk index and offset within that chunk for a global position pos.
    // Returns true and sets chunkIndex/offsetInChunk when pos is within logicalSize.
    bool locate(size_t pos, size_t *chunkIndex, size_t *offsetInChunk) const {
        if (pos >= logicalSize)
            return false;
        size_t acc = 0;
        for (size_t i = 0; i < chunkCount; ++i) {
            Chunk *ch = chunks[i];
            if (pos < acc + ch->used) {
                *chunkIndex = i;
                *offsetInChunk = pos - acc;
                return true;
            }
            acc += ch->used;
        }
        return false;
    }
};

class Version {
public:
    Content *body;
    size_t pos;

    Version() : body(nullptr), pos(0) {}

    Version(Content *b, size_t p) : body(b), pos(p) {}

    Version(const Version &o) : body(o.body), pos(o.pos) { if (body) body->acquire(); }

    Version(Version &&o) noexcept : body(o.body), pos(o.pos) { o.body = nullptr; o.pos = 0; }

    Version &operator=(Version o) { swap(o); return *this; }

    void swap(Version &o) noexcept { std::swap(body,o.body); std::swap(pos,o.pos); }

    ~Version() { if (body) body->release(); }
};

class CFile
{
private:
    Version *versions;
    size_t versionCount;
    size_t versionCapacity;

    // Detach the current Content if it is shared (implements copy-on-write semantics).
    void detachCurrentBody() {
        Version &cur = versions[versionCount - 1];
        Content *old = cur.body;
        if (old->refCount == 1)
            return;
        Content *nw = old->cloneShallow();
        old->release();
        cur.body = nw;
    }

    void freeAllVersions() {
        delete[] versions;
        versions = nullptr;
        versionCount = 0;
        versionCapacity = 0;
    }

    void ensureVersionCapacity() {
        if (versionCount < versionCapacity) return;
        size_t newCap = versionCapacity ? versionCapacity*2 : 4;
        Version *nv = new Version[newCap];
        for (size_t i=0;i<versionCount;++i) nv[i] = versions[i];
        delete[] versions;
        versions = nv;
        versionCapacity = newCap;
    }

public:
    CFile() {
        versionCapacity = 4;
        versions = new Version[versionCapacity];
        versionCount = 1;
        versions[0].body = Content::createEmpty();
        versions[0].pos = 0;
    }

    ~CFile() { freeAllVersions(); }

    CFile(const CFile &other) {
        versionCount = other.versionCount;
        versionCapacity = versionCount ? versionCount : 1;
        versions = new Version[versionCapacity];
        for (size_t i=0;i<versionCount;++i) versions[i] = other.versions[i];
    }

    CFile &operator=(const CFile &other) {
        if (this==&other) return *this;
        CFile tmp(other);
        freeAllVersions();
        versions = tmp.versions;
        versionCount = tmp.versionCount;
        versionCapacity = tmp.versionCapacity;
        tmp.versions = nullptr; tmp.versionCount = 0;
        tmp.versionCapacity = 0;
        return *this;
    }

    bool seek(size_t pos) {
        const Version &cur = versions[versionCount-1];
        if (pos > cur.body->logicalSize) return false;
        versions[versionCount-1].pos = pos;
        return true;
    }

    size_t read(uint8_t dst[], size_t len) {
        Version &cur = versions[versionCount-1];
        Content *c = cur.body;
        size_t avail = c->logicalSize - cur.pos;
        size_t n = len < avail ? len : avail;
        size_t done = 0, globalPos = cur.pos;
        while (done < n) {
            size_t chunkIndex, offsetInChunk;
            if (!c->locate(globalPos, &chunkIndex, &offsetInChunk)) break;
            Chunk *ch = c->chunks[chunkIndex];
            size_t room = ch->used - offsetInChunk;
            size_t take = n - done < room ? n - done : room;
            if (take) memcpy(dst + done, ch->data + offsetInChunk, take);
            done += take; globalPos += take;
        }
        cur.pos += n;
        return n;
    }

    size_t write(const uint8_t src[], size_t len) {
        if (len==0) return 0;
        detachCurrentBody();
        Version &cur = versions[versionCount-1];
        Content *c = cur.body;
        size_t p = cur.pos;
        size_t needEnd = p + len;
        if (needEnd < p) return 0;
        if (needEnd > c->logicalSize) c->logicalSize = needEnd;
        c->ensureCapacity();
        const uint8_t *srcPtr = src;
        size_t left = len;
        size_t globalPos = p;
        while (left > 0) {
            size_t chunkIndex, offsetInChunk;
            if (!c->locate(globalPos, &chunkIndex, &offsetInChunk)) break;
            c->uniqueChunk(chunkIndex);
            Chunk *ch = c->chunks[chunkIndex];
            size_t room = ch->used - offsetInChunk;
            size_t take = left < room ? left : room;
            if (take) memcpy(ch->data + offsetInChunk, srcPtr, take);
            srcPtr += take; globalPos += take; left -= take;
        }
        cur.pos += len;
        return len;
    }

    void truncate() {
        detachCurrentBody();
        Version &cur = versions[versionCount-1];
        Content *c = cur.body;
        size_t newLen = cur.pos;
        if (newLen==0) {
            for (size_t i=0;i<c->chunkCount;++i) c->chunks[i]->release();
            delete[] c->chunks; 
            c->chunks = nullptr;
            c->chunkCount = 0;
            c->chunkCapacity = 0;
            c->logicalSize = 0;
            c->allocatedSize = 0;
            return;
        }
        size_t acc=0, cutIdx=0, cutOff=0;
        for (size_t z=0; z<c->chunkCount; ++z) {
            Chunk *ch = c->chunks[z];
            if (newLen <= acc + ch->used) { cutIdx = z; cutOff = newLen - acc; break; }
            acc += ch->used;
        }
        c->uniqueChunk(cutIdx);
        c->chunks[cutIdx]->used = cutOff;
        for (size_t j=cutIdx+1; j<c->chunkCount; ++j) c->chunks[j]->release();
        c->chunkCount = cutIdx+1; c->logicalSize = newLen;
        /* allocatedSize must reflect actual allocated bytes */
        size_t accAlloc = 0; for (size_t i=0;i<c->chunkCount;++i) accAlloc += c->chunks[i]->used;
        c->allocatedSize = accAlloc;
    }

    size_t fileSize() const { return versions[versionCount-1].body->logicalSize; }

    /* Return current file position of the active version (for testing) */
    size_t position() const { return versions[versionCount-1].pos; }


    void addVersion() { 
        ensureVersionCapacity(); 
        Version &cur = versions[versionCount-1]; 
        versions[versionCount] = cur; ++versionCount; 
    }

    bool undoVersion() { if (versionCount <= 1) return false; --versionCount; return true; }
};

#ifndef __PROGTEST__
bool writeTest(CFile &x, const std::initializer_list<uint8_t> &data, size_t wrLen) { return x.write(data.begin(), data.size()) == wrLen; }
bool readTest(CFile &x, const std::initializer_list<uint8_t> &data, size_t rdLen) {
    uint8_t tmp[100]; uint32_t idx = 0;
    if (x.read(tmp, rdLen) != data.size()) return false;
    for (auto v : data) if (tmp[idx++] != v) return false; return true;
}

int manual_test() {
    using namespace std;
    CFile f;
    string line;
    while (getline(cin, line)) {
        size_t i = 0; while (i < line.size() && isspace((unsigned char)line[i])) ++i;
        if (i >= line.size()) continue;
        size_t j = i; while (j < line.size() && !isspace((unsigned char)line[j])) ++j;
        string cmd = line.substr(i, j - i);
        while (j < line.size() && isspace((unsigned char)line[j])) ++j;
        string rest = j < line.size() ? line.substr(j) : string();

        if (cmd == "W" || cmd == "write") {
            f.write((const uint8_t*)rest.data(), rest.size());
            cout << "OK\n";
            continue;
        }
        if (cmd == "R" || cmd == "read") {
            if (rest.empty()) { cout << '\n'; continue; }
            size_t n = 0; try { n = stoul(rest); } catch(...) { cout << '\n'; continue; }
            vector<uint8_t> buf(n);
            size_t r = f.read(buf.data(), n);
            if (r == 0) cout << '\n'; else cout << string((const char*)buf.data(), r) << '\n';
            continue;
        }
        if (cmd == "S" || cmd == "seek") { try { size_t p = stoul(rest); cout << (f.seek(p) ? "OK" : "ERR") << '\n'; } catch(...) { cout << "ERR\n"; } continue; }
        if (cmd == "A" || cmd == "add") { f.addVersion(); cout << "OK\n"; continue; }
        if (cmd == "U" || cmd == "undo") { cout << (f.undoVersion() ? "OK" : "ERR") << '\n'; continue; }
        cout << "UNKNOWN\n";
    }
    return EXIT_SUCCESS;
}

int auto_test ()
{
  CFile f0;

  assert ( writeTest ( f0, {10, 20, 30}, 3 ) );
  assert ( f0 . fileSize () == 3 );
  assert ( writeTest ( f0, {60, 70, 80}, 3 ) );
  assert ( f0 . fileSize () == 6 );
  assert ( f0 . seek ( 2 ));
  assert ( writeTest ( f0, {5, 4}, 2 ) );
  assert ( f0 . fileSize () == 6 );
  assert ( f0 . seek ( 1 ));
  assert ( readTest ( f0, {20, 5, 4, 70, 80}, 7 ));
  assert ( f0 . seek ( 3 ));
  f0 . addVersion();
  assert ( f0 . seek ( 6 ));
  assert ( writeTest ( f0, {100, 101, 102, 103}, 4 ) );
  f0 . addVersion();
  assert ( f0 . seek ( 5 ));
  CFile f1 ( f0 );
  f0 . truncate ();
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70}, 20 ));
  assert ( f0 . undoVersion () );
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70, 80, 100, 101, 102, 103}, 20 ));
  assert ( f0 . undoVersion () );
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70, 80}, 20 ));
  assert ( !f0 . seek ( 100 ));
  assert ( writeTest ( f1, {200, 210, 220}, 3 ) );
  assert ( f1 . seek ( 0 ));
  assert ( readTest ( f1, {10, 20, 5, 4, 70, 200, 210, 220, 102, 103}, 20 ));
  assert ( f1 . undoVersion () );
  assert ( f1 . undoVersion () );
  assert ( readTest ( f1, {4, 70, 80}, 20 ));
  assert ( !f1 . undoVersion () );
  std::cout << "All tests passed!" << std::endl;
  return EXIT_SUCCESS;
}

int main(void){
    using namespace std;
    char c = 'd';
    cout << "write a for auto test or m for manual test or e for exit" << endl;
    while (c != 'e'){
        c = getchar();
        if(c == 'a'){
            return(auto_test());
        }else if (c == 'm')
        {
            return(manual_test());
        }
    }
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */