#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <math.h>
#include <sys/shm.h>
#include "ShmCommon.h"

#ifdef HASH_TABLE_INFO

struct HashInfo {
    uint32_t m_capacity;
    uint32_t m_size;
    uint32_t m_numInsert;
    uint32_t m_numRemove;
    uint32_t m_numGet;
    uint32_t m_numGet2;

    void Init(uint32_t _capacity) {
        m_capacity = _capacity;
        m_size = 0;
        m_numInsert = 0;
        m_numRemove = 0;
        m_numGet = 0;
        m_numGet2 = 0;
    }
};
#endif //HASH_TABLE_INFO

template<class T>
class HashTable {
public:

    HashTable()
    : hashTime_(0)
    , hashLen_(0)
    , hashPrimers_(NULL)
    , hashTable_(NULL) {
    }

    ~HashTable() {
        if (NULL != hashPrimers_) {
            delete [] hashPrimers_;
        }
    }
    //return (1: have shm, 0: create new shm, -1: error)

    int Init(int hashLen, int hashTime, int shmKey) {
        hashLen_ = hashLen;
        hashTime_ = hashTime;

        InitHashPrime();
        return InitShm(shmKey, hashLen_ * hashTime_ * sizeof (T));
    }

    void Init(int hashLen, int hashTime) {
        hashLen_ = hashLen;
        hashTime_ = hashTime;
        hashTable_ = new T[hashTime_ * hashLen_];

        InitHashPrime();
    }

    int InsertElement(const T* pInE);
    int RemoveElement(const T* pInE);
    T* GetElement(const T* pInE);
    T* GetElement2(const T* pInE);
    bool Begin();
    T* Next();
    bool IsEnd();

    void Reset(); //dangerous function. It will clear all hash and hash' function.

private:
    void InitHashPrime();
    int InitShm(int shmKey, int shmSize);

private:
#ifdef HASH_TABLE_INFO
    HashInfo* info_;
#endif //HASH_TABLE_INFO
    int hashTime_;
    int hashLen_;
    int *hashPrimers_;
    T *hashTable_;
    unsigned int travelIdx;
};

inline bool IsPrimer(int num) {
    int q = sqrt(num);

    for (int i = 2; i < q; ++i) {
        if (0 == num % q)
            return false;
    }
    return true;
}

template <class T>
void HashTable<T>::InitHashPrime() {
    hashPrimers_ = new int[hashTime_];

    int count = 0;

    for (int i = hashLen_ - 1; i > 1; --i) {
        if (IsPrimer(i)) {
            hashPrimers_[count++] = i;
            if (count >= hashTime_) {
                break;
            }
        }
    }
}

template <class T>
bool HashTable<T>::Begin() {
    travelIdx = 0;
    if (hashTime_ <= 0 || hashLen_ <= 0)
        return false;
    return true;
}

template <class T>
T* HashTable<T>::Next() {
    return &hashTable_[travelIdx++];
}

template <class T>
bool HashTable<T>::IsEnd() {
    return (travelIdx >= hashTime_ * hashLen_);
}

template <class T>
int HashTable<T>::InsertElement(const T* pInE) {
#ifdef HASH_TABLE_INFO
    info_->m_numInsert++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &hashTable_[i * hashLen_ + hash];
        if (!pE->IsValid() || pE->Key() == pInE->Key()) {
            *pE = *pInE;
            if (i == hashTime_ - 2) {
                DBG_LOG(WARN,"HashTable: InsertElement -> Hash Full");
            }
#ifdef HASH_TABLE_INFO
    	    info_->m_size++;
#endif //HASH_TABLE_INFO
            return 0;
        }
    }
    return -1;
}

template <class T>
int HashTable<T>::RemoveElement(const T* pInE) {
#ifdef HASH_TABLE_INFO
    info_->m_numRemove++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &hashTable_[i * hashLen_ + hash];
        if (pE->IsValid() && pE->Key() == pInE->Key()) {
            pE->Clear();
#ifdef HASH_TABLE_INFO
    info_->m_size--;
#endif //HASH_TABLE_INFO
            return 0;
        }
    }
    return -1;
}

template <class T>
T* HashTable<T>::GetElement(const T* pInE) {
#ifdef HASH_TABLE_INFO
    info_->m_numGet++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &(hashTable_[i * hashLen_ + hash]);
        if (pE->IsValid() && pE->Key() == pInE->Key()) {
            return pE;
        }
    }
    return NULL;
}

template <class T>
T* HashTable<T>::GetElement2(const T* pInE) {
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &(hashTable_[i * hashLen_ + hash]);
        if (pE->IsValid2() && pE->Key() == pInE->Key()) {
            return pE;
        }
    }
    return NULL;
}

template <class T>
void HashTable<T>::Reset() {
    memset(hashTable_, 0, hashLen_ * hashTime_ * sizeof (T));
#ifdef HASH_TABLE_INFO
    memset(info_, 0, sizeof(HashInfo));
#endif //HASH_TABLE_INFO
}

//return (2: have shm, 1: create 0: create new shm, -1: error)

template <class T>
int HashTable<T>::InitShm(int shmKey, int shmSize) {
#ifndef HASH_TABLE_INFO
    hashTable_ = (T *) MGetShm(shmKey, shmSize, 0666);
    if (hashTable_) {
        return 1;
    }

    hashTable_ = (T *) MGetShm(shmKey, shmSize, 0666 | IPC_CREAT);
    if (NULL == hashTable_) return -1;

    return 0;
#else //HASH_TABLE_INFO
    void* temp = MGetShm(shmKey, shmSize + sizeof (HashInfo), 0666);
    if (temp) {
        info_ = (HashInfo*) temp;
        info_->Init(shmSize);
        hashTable_ = (T *) temp + sizeof (HashInfo);
        return 1;
    }

    temp = MGetShm(shmKey, shmSize + sizeof (HashInfo), 0666 | IPC_CREAT);
    if (NULL == temp) return -1;
    info_ = (HashInfo*) temp;
    info_->Init(shmSize);
    hashTable_ = (T *) temp + sizeof (HashInfo);
    return 0;
#endif //HASH_TABLE_INFO
}

#endif


