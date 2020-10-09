#pragma once

#include <string>
#include <vector>

#include "../exceptions/hashing/BucketIndexOccupied.hpp"
#include "../exceptions/hashing/HashTableCapacity.hpp"
#include "../exceptions/hashing/InvalidKeySize.hpp"
#include "../raytracer/Globals.hpp"

using namespace std;

#define HASH_TABLE_MAX_SIZE 1024
#define HASH_TABLE_DEFAULT_SIZE 16
#define HASH_TABLE_MAX_KEY_SIZE 32

#define FNV_PRIME_MOD 22801763489 // Closest prime to 1e9 - 9
#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261

template <typename V>
struct HMEntry {
    HMEntry(const string& key, V& value);
    ~HMEntry();

    string key;
    V value;
    HMEntry *next = nullptr;
};

template <typename V>
HMEntry<V>::HMEntry(const string& key, V& value){
    this->key = key;
    this->value = value;
    this->next = nullptr;
};

template<typename V>
HMEntry<V>::~HMEntry(){};

template <typename V>
class HashTable {
   public:
        explicit HashTable();
        explicit HashTable(int size);
        ~HashTable();

        bool get(const string& key, V& value);
        void insert(const string& key, V value);
        void remove(const string& key);

        size_t bucketsCount() const noexcept;
        size_t size() const noexcept;
        size_t hashFunc(const string& key) const;
   private:
        size_t element_count;
        int table_size;
        vector<HMEntry<V>*> buckets;

        inline void processBucketLinking(size_t hashValue, HMEntry<V>* prev, HMEntry<V>* bucket);

        inline void validateKeySize(const string& key) const;
        inline void getSequentialNonNull(HMEntry<V>* prev, HMEntry<V>* bucket, const string& key);
};

template <typename V>
HashTable<V>::HashTable(int size) {
    if (size > HASH_TABLE_MAX_SIZE) {
        throw HashTableCapacity(size);
    }
    this->table_size = size;
    this->buckets = vector<HMEntry<V>*>(size, nullptr);
    this->element_count = 0;
};

template <typename V>
HashTable<V>::HashTable() {
    this->table_size = HASH_TABLE_DEFAULT_SIZE;
    this->buckets = vector<HMEntry<V>*>(HASH_TABLE_DEFAULT_SIZE, nullptr);
    this->element_count = 0;
};

template <typename V>
HashTable<V>::~HashTable() {
    for (int i = table_size - 1; i != -1; i--) {
        HMEntry<V> *bucket = this->buckets[i];
        while (bucket != nullptr) {
            HMEntry<V>* current = bucket;
            bucket = bucket->next;
            delete current;
        }
        this->buckets[i] = nullptr;
    }
};

template <typename V>
bool HashTable<V>::get(const string& key, V& value) {
    if (this->element_count == 0) {
        throw HashTableCapacity(this->element_count, "No elements in table. Size is: ");
    }

    validateKeySize(key);

    size_t hashValue = hashFunc(key);
    HMEntry<V>* bucket = this->buckets[hashValue];
    while (bucket != nullptr) {
        if (bucket->key == key) {
            value = bucket->value;
            return true;
        }
        bucket = bucket->next;
    }
    return false;
};

template <typename V>
inline void HashTable<V>::getSequentialNonNull(HMEntry<V>* prev, HMEntry<V>* bucket, const string& key) {
    while (bucket != nullptr && bucket->key != key) {
        prev = bucket;
        bucket = bucket->next;
    }
};

template <typename V>
inline void HashTable<V>::processBucketLinking(size_t hashValue, HMEntry<V>* prev, HMEntry<V>* bucket) {
    if (prev == nullptr) {
        this->buckets[hashValue] = bucket;
    } else {
        prev->next = bucket;
    }
}

template <typename V>
void HashTable<V>::insert(const string& key, V value) {
    validateKeySize(key);

    size_t hashValue = hashFunc(key);
    HMEntry<V>* prev = nullptr;
    HMEntry<V>* bucket = this->buckets[hashValue];

    getSequentialNonNull(prev, bucket, key);

    if (bucket != nullptr) {
        bucket->value = value;
        this->element_count++;
        return;
    }

    bucket = new HMEntry<V>(key, value);
    processBucketLinking(hashValue, prev, bucket);
    this->element_count++;
};

template <typename V>
void HashTable<V>::remove(const string& key) {
    validateKeySize(key);
    
    size_t hashValue = hashFunc(key);
    HMEntry<V>* prev = nullptr;
    HMEntry<V>* bucket = this->buckets[hashValue];

    getSequentialNonNull(prev, bucket, key);

    if (bucket == nullptr) {
        return;
    }

    processBucketLinking(hashValue, prev, bucket->next);
    delete bucket;
    this->element_count--;
};

template <typename V>
size_t HashTable<V>::size() const noexcept {
    return this->element_count;
};

template <typename V>
size_t HashTable<V>::bucketsCount() const noexcept {
    return this->table_size;
};

template <typename V>
inline void HashTable<V>::validateKeySize(const string& key) const {
    if (key.length() > HASH_TABLE_MAX_KEY_SIZE) {
        throw InvalidKeySize(key);
    }
}

template <typename V>
size_t HashTable<V>::hashFunc(const string& key) const {
    validateKeySize(key);

    size_t prime_power = 13;
    size_t hashVal = FNV_OFFSET_32;
    for (int i = key.length(); i != -1; i--) {
        hashVal = (hashVal + (key[i] ^ FNV_PRIME_32) * prime_power) % this->table_size;
        prime_power = (prime_power * FNV_PRIME_MOD) % this->table_size;
    }
    return hashVal;
};