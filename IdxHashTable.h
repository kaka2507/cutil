/* 
 * File:   IdxHashTable.h
 * Author: baodn
 * 
 * Created on April 5, 2013, 9:56 AM
 */

#ifndef IDX_HASH_TABLE_H
#define IDX_HASH_TABLE_H
#include <sys/shm.h>

inline void *MGetShm(int key, unsigned int size, int flags)
{
	int shmId;
	long ret;

	if ((shmId = shmget(key, size, flags)) == -1)
		return NULL;

	if ((ret = (long)shmat(shmId, NULL, 0)) == -1)
		return NULL;
	return (void *)ret;
}

template<class T>
class IdxHashTable
{
public:
	IdxHashTable()
	{
		m_size = 0;
		m_hashTable = NULL;
		m_useShm = false;
		m_travelIdx = 0;
	}

	~IdxHashTable()
	{
		m_size = 0;
		if(!m_useShm && (m_hashTable != NULL))
			delete [] m_hashTable;
	}

	int Init(uint32_t _size)
	{
		m_useShm = false;
		m_size = _size;
		m_hashTable = new T[m_size];
		return 0;
	}

	/*
	** Init a shared memory area with size = _size and key = _shmKey
	** Return
	**		1: If the shared memory with key is existed
	**		0: If you created a new shared memory location
	**		-1: Don't exist and can not creat
	*/
	int Init(uint32_t _size, int _shmKey)
	{
		m_useShm = true;
		m_size = _size;
		m_hashTable = (T *)MGetShm(_shmKey, _size, 0666);
		if (m_hashTable)
		{
			return 1;
		}

		m_hashTable = (T *)MGetShm(_shmKey, _size, 0666 | IPC_CREAT);
		if (NULL == m_hashTable) return -1;

		return 0;
	}

	/*
	** Return the location of object in shm or array
	*/
	int64_t Insert(T* object)
	{
		for(int64_t i = 0; i < m_size; i++)
		{
			T* tempObj = m_hashTable[i];
			if(!tempObj.IsValid() || tempObj.Key() == object.Key())
			{
				*tempObj = *object;
				return i;
			}
		}
		return -1;
	}

	int Remove(int64_t localtion)
	{
		if(location >= 0 && location < m_size)
		{
			m_hashTable[location].Clear();
			return 0;
		}
		else
		{
			return 1;
		}
	}

	int Remove(T* object)
	{
		for(int64_t i = 0; i < m_size; i++)
		{
			T* tempObj = m_hashTable[i];
			if(tempObj.IsValid() && tempObj.Key() == object.Key())
			{
				tempObj.Clear();
				return 0;
			}
		}
		return -1;
	}

	T* GetElement(T* object)
	{
		for(int64_t i = 0; i < m_size; i++)
		{
			T* tempObj = m_hashTable[i];
			if(tempObj.IsValid() && tempObj.Key() == object.Key())
			{
				return tempObj;
			}
		}
		return NULL;
	}

	T* GetElement(int64_t location)
	{
		if(location >= 0 && location < m_size)
		{
			return m_hashTable[location];
		}
		else
		{
			return NULL;
		}
	}

	bool Begin()
	{
		m_travelIdx = 0;
		if(m_size == 0)
			return false;
		return true;
	}

    T* Next()
    {
    	return &m_hashTable[m_travelIdx++];
    }

    bool IsEnd()
    {
    	return (m_travelIdx >= m_size);
    }

private:
	uint32_t m_size;
	bool m_useShm;
	T* m_hashTable;
	uint32_t m_travelIdx;
};

 #endif //IDX_HASH_TABLE_H