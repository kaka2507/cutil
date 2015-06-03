/* 
 * File:   DoubleKeyHashTable.h
 * Author: baodn
 * 
 * Created on Jul 30, 2013, 2:24 PM
 */

#ifndef _DOUBLE_KEY_HASH_TABLE_H_
#define _DOUBLE_KEY_HASH_TABLE_H_

#include <math.h>
#include <stdint.h>
#include <sys/shm.h>
#include "klogger.h"
#include "ShmCommon.h"


#ifdef HASH_TABLE_INFO

struct DoubleKeyHashInfo {
    uint32_t m_capacity;
    uint32_t m_numInsert;
    uint32_t m_numInsertFail1;
    uint32_t m_numInsertFail2;
    uint32_t m_numRemove;
    uint32_t m_numRemoveFail1;
    uint32_t m_numRemoveFail2;
    uint32_t m_numGet;
    uint32_t m_numGetFail;
    uint32_t m_numGet2;
    uint32_t m_numGet2Fail;

    void Init(uint32_t _capacity) {
        m_capacity = _capacity;
        m_numInsert = 0;
        m_numInsertFail1 = 0;
        m_numInsertFail2 = 0;
        m_numRemove = 0;
        m_numRemoveFail1 = 0;
        m_numRemoveFail2 = 0;
        m_numGet = 0;
        m_numGetFail = 0;
        m_numGet2 = 0;
        m_numGet2Fail = 0;
    }

    void Dump()
    {
    	printf("capacity: %u\n", m_capacity);
    	printf("m_numInsert: %u\n", m_numInsert);
    	printf("m_numInsertFail1: %u\n", m_numInsertFail1);
    	printf("m_numInsertFail2: %u\n", m_numInsertFail2);
    	printf("m_numRemove: %u\n", m_numRemove);
    	printf("m_numRemoveFail1: %u\n", m_numRemoveFail1);
    	printf("m_numRemoveFail2: %u\n", m_numRemoveFail2);
    	printf("m_numGet: %u\n", m_numGet);
    	printf("m_numGetFail: %u\n", m_numGetFail);
    	printf("m_numGet2: %u\n", m_numGet2);
    	printf("m_numGet2Fail: %u\n", m_numGet2Fail);
    }
};
#endif //HASH_TABLE_INFO

template<class T>
class DoubleKeyHashTable {
public:

    DoubleKeyHashTable()
    : hashTime_(0)
    , hashLen_(0)
    , hashPrimers_(NULL)
    , hashTable_(NULL)
    , hashTable2_(NULL) {
    }

    ~DoubleKeyHashTable() {
        if (NULL != hashPrimers_) {
            delete [] hashPrimers_;
        }
    }
    //return (1: have shm, 0: create new shm, -1: error)

    int Init(int hashLen, int hashTime, int shmKey) {
	   printf("call DoubleKeyhashTable init with %d %d %d\n", hashLen, hashTime, shmKey);
        hashLen_ = hashLen;
        hashTime_ = hashTime;

        InitHashPrime();
        return InitShm(shmKey, hashLen_ * hashTime_ * sizeof (T));
    }

    //this function only use to test. If you want to use it, you should edit to prevent leak memory
    void Init(int hashLen, int hashTime) {
        hashLen_ = hashLen;
        hashTime_ = hashTime;
        hashTable_ = new T[hashTime_ * hashLen_];
        hashTable2_ = new T[hashTime_ * hashLen_];
#ifdef HASH_TABLE_INFO
    	info_ = new DoubleKeyHashInfo();
    	info_->Init(hashTime_ * hashLen_);
#endif //HASH_TABLE_INFO

        InitHashPrime();
    }

    int InsertElement(const T* pInE); //When insert a element to hash, you should call GetElement and GetElement2 to check whether it exist
    int RemoveElement(const T* pInE);
    const T* GetElement(const T* pInE);
    const T* GetElement2(const T* pInE);
    bool Begin();
    T* Next();
    bool IsEnd();

    void Reset(); //dangerous function. It will clear all hash and hash' function.

#ifdef HASH_TABLE_INFO
    DoubleKeyHashInfo* info_;
#endif //HASH_TABLE_INFO
private:
    void InitHashPrime();
    int InitShm(int shmKey, int shmSize);
    int hashTime_;
    int hashLen_;
    int *hashPrimers_;
    T *hashTable_;
    T *hashTable2_;
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
void DoubleKeyHashTable<T>::InitHashPrime() {
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
bool DoubleKeyHashTable<T>::Begin() {
    travelIdx = 0;
    if (hashTime_ <= 0 || hashLen_ <= 0)
        return false;
    return true;
}

template <class T>
T* DoubleKeyHashTable<T>::Next() {
    return &hashTable_[travelIdx++];
}

template <class T>
bool DoubleKeyHashTable<T>::IsEnd() {
    return (travelIdx >= hashTime_ * hashLen_);
}

template <class T>
int DoubleKeyHashTable<T>::InsertElement(const T* pInE) {
	if(!pInE->IsValid() || !pInE->IsValid2())
		return -3;
#ifdef HASH_TABLE_INFO
    info_->m_numInsert++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &hashTable_[i * hashLen_ + hash];
        if (!pE->IsValid()) {
        	for(int j = 0; j < hashTime_; j++)
        	{
        		int hash2 = pInE->Key2() % hashPrimers_[j];
        		T *pE2 = &hashTable2_[j * hashLen_ + hash2];
        		if(!pE2->IsValid2())
        		{
        			*pE = *pInE;
        			*pE2 = *pInE;
        			return 0;
        		}
        	}
#ifdef HASH_TABLE_INFO
    		info_->m_numInsertFail2++;
#endif //HASH_TABLE_INFO
        	DBG_LOG(WARNING, "DoubleKeyHashTable: InsertElement can not find empty space in hashTable2_");
            return -2;
        }
    }
#ifdef HASH_TABLE_INFO
    info_->m_numInsertFail1++;
#endif //HASH_TABLE_INFO
    DBG_LOG(WARNING, "DoubleKeyHashTable: InsertElement can not find empty space in hashTable_");
    return -1;
}

template <class T>
int DoubleKeyHashTable<T>::RemoveElement(const T* pInE) {
	if(!pInE->IsValid() || !pInE->IsValid2())
		return -3;
#ifdef HASH_TABLE_INFO
    info_->m_numRemove++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key() % hashPrimers_[i];
        T *pE = &hashTable_[i * hashLen_ + hash];
        if (pE->IsValid() && pE->Key() == pInE->Key() && pE->Key2() == pInE->Key2()) {
        	for(int j = 0; j < hashTime_; j++)
        	{
        		int hash2 = pInE->Key2() % hashPrimers_[j];
        		T *pE2 = &hashTable2_[j*hashLen_ + hash2];
        		if(pE2->IsValid2() && pE2->Key2() == pInE->Key2() && pE2->Key() == pInE->Key())
        		{
        			pE->Clear();
        			pE2->Clear();
        			return 0;
        		}
        	}
#ifdef HASH_TABLE_INFO
    		info_->m_numRemoveFail2++;
#endif //HASH_TABLE_INFO
        	DBG_LOG(WARNING, "DoubleKeyHashTable: RemoveElement does not exist in hashTable2_");
            return -2;
        }
    }
#ifdef HASH_TABLE_INFO
   	info_->m_numRemoveFail1++;
#endif //HASH_TABLE_INFO
    DBG_LOG(WARNING, "DoubleKeyHashTable: RemoveElement can not exist in hashTable_");
    return -1;
}

template <class T>
const T* DoubleKeyHashTable<T>::GetElement(const T* pInE) {
	if(!pInE->IsValid())
		return NULL;
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
#ifdef HASH_TABLE_INFO
    info_->m_numGetFail++;
#endif //HASH_TABLE_INFO
    return NULL;
}

template <class T>
const T* DoubleKeyHashTable<T>::GetElement2(const T* pInE) {
	if(!pInE->IsValid2())
		return NULL;
#ifdef HASH_TABLE_INFO
    info_->m_numGet2++;
#endif //HASH_TABLE_INFO
    for (int i = 0; i < hashTime_; i++) {
        int hash = pInE->Key2() % hashPrimers_[i];
        T *pE = &(hashTable2_[i * hashLen_ + hash]);
        if (pE->IsValid2() && pE->Key2() == pInE->Key2()) {
            return pE;
        }
    }
#ifdef HASH_TABLE_INFO
    info_->m_numGet2Fail++;
#endif //HASH_TABLE_INFO
    return NULL;
}

template <class T>
void DoubleKeyHashTable<T>::Reset() {
	if(hashTable_)
	{
    	memset(hashTable_, 0, hashLen_ * hashTime_ * sizeof (T));
    }
    if(hashTable2_)
    {
    	memset(hashTable2_, 0, hashLen_ * hashTime_ * sizeof (T));	
    }
#ifdef HASH_TABLE_INFO
    if(info_)
    {
    	memset(info_, 0, sizeof(DoubleKeyHashInfo));
    }
#endif //HASH_TABLE_INFO
}

//return (2: have shm, 1: create 0: create new shm, -1: error)

template <class T>
int DoubleKeyHashTable<T>::InitShm(int shmKey, int shmSize) {
#ifndef HASH_TABLE_INFO
    char* alloc = (char*) MGetShm(shmKey, shmSize*2, 0666);
    if (alloc) {
	hashTable_ = (T*) alloc;
    	hashTable2_ = (T*)(alloc + shmSize);
        return 1;
    }

    alloc = (char*) MGetShm(shmKey, shmSize*2, 0666 | IPC_CREAT);
    if (NULL == alloc) {
	return -1;
    }
    hashTable_ = (T*) alloc; 
    hashTable2_ = (T*)(alloc + shmSize);
    return 0;
#else //HASH_TABLE_INFO
    char* temp = (char*) MGetShm(shmKey, shmSize*2 + sizeof (DoubleKeyHashInfo), 0666);
    if (temp) {
        info_ = (DoubleKeyHashInfo*) temp;
        hashTable_ = (T*) (temp + sizeof (DoubleKeyHashInfo));
        hashTable2_ = (T*) (temp + shmSize + sizeof (DoubleKeyHashInfo));
        return 1;
    }

    temp = (char*) MGetShm(shmKey, shmSize*2 + sizeof (DoubleKeyHashInfo), 0666 | IPC_CREAT);
    if (NULL == temp) {
	return -1;
    }
    info_ = (DoubleKeyHashInfo*) temp;
    info_->Init(shmSize/sizeof(T));
    hashTable_ = (T*) (temp + sizeof (DoubleKeyHashInfo));
    hashTable2_ = (T*) (temp + shmSize + sizeof (DoubleKeyHashInfo));
    return 0;
#endif //HASH_TABLE_INFO
}

#endif


