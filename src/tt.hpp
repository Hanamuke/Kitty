#ifndef TT_INCLUDED
#define TT_INCLUDED
#include "types.hpp"
#include <cstring>

template <typename Entry>
class HashTable
{
public:
  HashTable() : table(nullptr), tableSize(0) {}
  HashTable(size_t nMB) { resize(nMB); }
  virtual ~HashTable()
  {
    if (table != nullptr)
      delete[] table;
  }
  void resize(size_t nMB)
  {
    tableSize = nMB * (1024 * 1024 / cacheLineSize);
    assert(tableSize <= (1ULL << 32));
    if (table != nullptr)
      delete[] table;
    table = new Bucket[tableSize];
    clear();
  }
  void clear() { std::memset(table, 0, tableSize * sizeof(Bucket)); }
  Entry *probe(Key key, bool &hit) const
  {
    Bucket *b = bucket(key);
    Entry *ret = nullptr;
    int replacementPriority = -1;
    for (size_t i = 0; i < bucketSize; ++i)
    {
      if (b->key(i) == key)
      {
        hit = true;
        return &(*b)[i];
      }
      if (b->priority(i) > replacementPriority)
      {
        replacementPriority = b->priority(i);
        ret = &(*b)[i];
      }
    }
    hit = false;
    return ret;
  }
  int hashfull() const
  {
    assert(1000 / bucketSize < tableSize);
    int hashfull = 0;
    for (int i = 1; i < 1000 / bucketSize + 1; ++i)
    {
      Bucket *b = table + i;
      for (int j = 0; j < bucketSize; ++j)
        if (b[i].key)
          hashfull++;
    }
    return hashfull;
  } // estimates fullness

private:
  static constexpr size_t cacheLineSize = 64;
  static constexpr size_t bucketSize = cacheLineSize / sizeof(Entry);

  struct Bucket
  {
    Entry entry[bucketSize];
    inline Entry &operator[](size_t i) { return entry[i]; }
    inline Key key(size_t i) const { return entry[i].key; }
    inline int priority(size_t i) const { return entry[i].priority(); }
  } __attribute__((aligned(cacheLineSize)));
  inline Bucket *bucket(Key key) const
  {
    return table + (((key & 0xffffffffULL) * (tableSize - 1)) >> 32);
  }
  Bucket *table;
  size_t tableSize;
};

struct PerftEntry
{
  Key key;
  uint64_t perft;
  Depth depth;
  inline int priority() const { return depth; }
} __attribute__((packed));

#endif