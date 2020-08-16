
#ifndef EXPANDABLE_HASH_MAP_INCLUDED
#define EXPANDABLE_HASH_MAP_INCLUDED

#include <list>
#include <iostream>
#include <ostream>
#include <functional>
#include <string>


// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
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
    struct KeyValuePair
    {
        KeyType m_key;
        ValueType m_value;
    };
    
    int num_values, num_buckets;
    double max_load_factor;
    
    unsigned int getBucketNumber(const KeyType& k) const;
    
    std::list<KeyValuePair>* hashArray;
    
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    num_buckets = 8;
    num_values = 0;
    max_load_factor = maximumLoadFactor;
    
    hashArray = new std::list<KeyValuePair>[num_buckets];
    
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::~ExpandableHashMap()
{
    reset();
    delete [] hashArray;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::reset()
{
    for  (int i = 0 ; i < num_buckets ; i++)
    {
        hashArray[i].clear();
    }
    delete [] hashArray;
    
    num_buckets = 8;
    num_values = 0;
    
    hashArray = new std::list<KeyValuePair>[num_buckets];
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::size() const
{
    return num_values;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::associate(const KeyType& key, const ValueType& value)
{
    unsigned int hashKey = getBucketNumber(key);

    // search the list for the key. if it is already in the map, update the value. if not, do nothing
    typename std::list<KeyValuePair>::iterator it;
    for ( it = hashArray[hashKey].begin() ; it != hashArray[hashKey].end() ; it++ )
    {
        if ( it->m_key == key)
        {
            it->m_value = value;
            return;
        }
    }
    
    
    // the value is not in the hashMap, we'll add it
    num_values++;

    KeyValuePair insertPair;
    insertPair.m_key = key;
    insertPair.m_value = value;
    
    hashArray[hashKey].push_back(insertPair);


    // value is not in the hash map, and we have to re-hash
    if (num_values > max_load_factor * num_buckets)
    {
        num_buckets *= 2;
        std::list<KeyValuePair>* newHashArray = new std::list<KeyValuePair>[num_buckets];
        
        for (int i = 0 ; i < num_buckets/2 ; i++)
        {
            typename std::list<KeyValuePair>::iterator it;
            for ( it = hashArray[i].begin() ; it != hashArray[i].end() ; it++ )
            {
                KeyType currKey = it->m_key;
                ValueType currVal = it->m_value;
                KeyValuePair currPair;
                currPair.m_key = currKey;
                currPair.m_value = currVal;
                
                unsigned int newHashKey = getBucketNumber(currKey);
                newHashArray[newHashKey].push_back(currPair);
            }
            hashArray[i].clear();
        }
        
        delete [] hashArray;
        hashArray = newHashArray;
    }
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType,ValueType>::find(const KeyType& key) const
{
    unsigned int hashKey = getBucketNumber(key);
    typename std::list<KeyValuePair>::iterator it;
    for ( it = hashArray[hashKey].begin() ; it != hashArray[hashKey].end() ; it++ )
    {
        if ( it->m_key == key)
            return &(it->m_value);
    }
    return nullptr;
}


template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType,ValueType>::getBucketNumber(const KeyType &k) const
{
    unsigned int hasher(const KeyType& k); // prototype
    unsigned int IDnum = hasher(k);
    return IDnum % num_buckets;
}


#endif // EXPANDABLE_HASH_MAP_INCLUDED

