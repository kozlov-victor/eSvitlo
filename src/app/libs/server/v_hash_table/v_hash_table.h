#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <Arduino.h>

template<typename U>
class VHashTable {
private:
    static const size_t TABLE_SIZE = 128; // Size of the hash table
    struct Entry {
        String key;
        U value;
        Entry* next;
    };
    Entry* table[TABLE_SIZE];
    size_t numElements;

    size_t hashFunction(String key) {
        size_t hash = 0;
        for (size_t i = 0; i < key.length(); i++) {
            hash = (hash * 31) + key.charAt(i);
        }
        return hash % TABLE_SIZE;
    }

public:
    VHashTable():numElements(0) {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }

    ~VHashTable() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                Entry* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
    }

    void forEach(std::function<void(const String, const U&)> callback) {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                callback(entry->key, entry->value);
                entry = entry->next;
            }
        }
    }

    void put(String key, U value) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        entry = new Entry{key, value, table[hash]};
        table[hash] = entry;
        this->numElements++;
    }

    U get(String key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        return U(); // Return default value if key not found
    }

    bool has(String key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return true;
            }
            entry = entry->next;
        }
        return false;
    }

    void remove(String key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        Entry* prev = nullptr;
        while (entry != nullptr) {
            if (entry->key == key) {
                if (prev != nullptr) {
                    prev->next = entry->next;
                } else {
                    table[hash] = entry->next;
                }
                delete entry;
                this->numElements--;
                return;
            }
            prev = entry;
            entry = entry->next;
        }
    }

    void clear() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                Entry* prev = entry;
                entry = entry->next;
                delete prev;
            }
            table[i] = nullptr;
        }
        this->numElements = 0;
    }

    U& operator[](String key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        entry = new Entry{key, U(), table[hash]};
        table[hash] = entry;
        this->numElements++;
        return entry->value;
    }

    size_t size() const {
        return this->numElements;
    }

};

#endif // HASH_TABLE_H