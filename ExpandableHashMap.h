#include <list>
#include <string>
#include <functional>
#include <iostream>
// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

template<typename KeyType, typename ValueType>
class ExpandableHashMap {
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    //void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct Entry {
        Entry(KeyType key, ValueType value) : key(key), value(value){
        }
        KeyType key;
        ValueType value;
    };
    
    double loadFactor;
    int curSize;
    int numAssocs;
    std::list<Entry> *map;
    void rehash();
    int mapFunc(KeyType key) const;
};

//returns bucket number for a given key
template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::mapFunc(KeyType key) const{
    unsigned int hasher(const KeyType& k);
    return hasher(key) % curSize;
    
    //std::hash<std::string> str_hash;
    //return str_hash(key) % curSize;
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor){
    this->curSize = 8;
    this->numAssocs = 0;
    this->loadFactor = maximumLoadFactor;
    this->map = new std::list<Entry>[8];
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap(){
    delete[] map;
}

//template<typename KeyType, typename ValueType>
//void ExpandableHashMap<KeyType, ValueType>::reset(){
//    //reset to new map with 8 buckets, delete old map
//    this->curSize = 8;
//    this->numAssocs = 0;
//    std::list<Entry> *temp = new std::list<Entry>[8];
//    delete[] this->map;
//    this->map = temp;
//}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const{
    return this->numAssocs;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value){
    int bucketNum = mapFunc(key);
    for(auto it = map[bucketNum].begin(); it != map[bucketNum].end(); it++){
        if((*it).key == key){
            (*it).value = value;
            return;
        }
    }
    this->map[bucketNum].push_front(Entry(key, value));
    this->numAssocs++;
    
    if((double)numAssocs/curSize > loadFactor){
        rehash();
    }
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const{
    int bucketNum = mapFunc(key);
    for(auto it = map[bucketNum].begin(); it != map[bucketNum].end(); it++){
        if((*it).key == key){
            return &((*it).value);
        }
            
    }
    return nullptr;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::rehash() {
    this->numAssocs = 0;
    this->curSize *=2;
    std::list<Entry> *temp = new std::list<Entry>[curSize];
    
    for(int i = 0; i < curSize/2; i++){
        for(auto it = map[i].begin(); it != map[i].end(); it++){
            Entry e = *it;
            int bucketNum = mapFunc(e.key);
            temp[bucketNum].push_front(Entry(e.key, e.value));
            this->numAssocs++;
        }
    }
   
    delete[] this->map;
    this->map = temp;
}


