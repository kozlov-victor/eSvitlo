#ifndef V_ARRAY_LIST_H
#define V_ARRAY_LIST_H

#include <Arduino.h>

template <class T>
class VArrayList {
private:
    T *Array;
    size_t pointer;
    size_t arrayLength;

    bool needToResize() {
        return this->pointer == this->arrayLength;
    }
    void resize() {
        size_t newLength = this->arrayLength + 10;
        T *temp = new T[newLength];
        for(size_t i = 0; i < this->pointer; i++){
            temp[i] = this->Array[i];
        }
        delete [] this->Array;
        this->Array = temp;
        this->arrayLength = newLength;
    }

    bool checkRange(size_t index) {
        return index < this->pointer;
    }

public:
    explicit VArrayList(size_t size=10) {
        this->Array = new T[size];
        this->pointer = 0;
        this->arrayLength = size;
    }

    ~VArrayList() {
        delete[] this->Array;
    }

    void addAt(const T &a, size_t index) {
        if (!this->checkRange(index)) return;
        if(this->needToResize()){
            this->resize();
        }
        for(size_t i = this->pointer; i > index; i--){
            this->Array[i] = this->Array[i - 1];
        }
        this->Array[index] = a;
        ++this->pointer;
    }

    void add(const T &a) {
        if(this->needToResize()){
            this->resize();
        }
        this->Array[pointer] = a;
        ++this->pointer;
    }

    T& getAt(size_t index) {
        if (!this->checkRange(index)) {
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            Serial.printf("VArrayList::getAt: index out of bounds %i, size: %i", index, this->size());
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            Serial.println("VArrayList::error::");
            return this->Array[0];
        };
        return this->Array[index];
    }

    void removeAt(size_t index) {
        if (!this->checkRange(index)) return;

        for(size_t i = index; i < this->pointer - 1; i++){
            this->Array[i] = this->Array[i+1];
        }

        --this->pointer;
    }

    bool empty() {
        return this->pointer == 0;
    }

    size_t indexOf(const T &item) {
        for(size_t i = 0; i < this->pointer; i++){
            if(this->Array[i] == item){
                return i;
            }
        }
        return 0;
    }

    bool has(const T &item) {
        for(size_t i = 0; i < this->pointer; i++){
            if(this->Array[i] == item){
                return true;
            }
        }
        return false;
    }

    size_t size() const {
        return this->pointer;
    }

    VArrayList(const VArrayList&) = delete;
    VArrayList& operator=(const VArrayList&) = delete;

};

#endif // V_ARRAY_LIST_H