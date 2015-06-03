/* 
 * File:   UInt32KeyMap.h
 * Author: baodn
 * 
 * Created on March 7, 2013, 2:24 PM
 */

#ifndef UInt32KeyMap_H
#define	UInt32KeyMap_H

#include <iostream>
#include <stdint.h>

#define DEBUG 0
#if DEBUG
#define DEBUG_LOG(X) {std::cout << X << std::endl;}
#else
#define DEBUG_LOG(X)
#endif

template<typename ValueType>
class UInt32KeyMap {

	struct MapObject
	{
		bool isStored;
		ValueType value;
		MapObject()
		{
			isStored = false;
		}

		void Store(ValueType object)
		{
			isStored = true;
			value = object;
		}

		ValueType& Get()
		{
			return value;
		}

		ValueType& Remove()
		{
			isStored = false;
			return value;
		}

		bool IsStored()
		{
			return isStored;
		}
	};

	template<typename BucketType>
	struct Bucket
	{
		BucketType* object;
		uint16_t ref_count;
		Bucket()
		{
			ref_count = 0;
			object = NULL;
		}

		bool IsOpen()
		{
			return (object != NULL);
		}

		void Open()
		{
			DEBUG_LOG("Bucket: OpenBucket");
			object = new BucketType[256];
		}

		void OpenBucket(uint8_t idx)
		{
			if(object)
			{
				object[idx].Open();
				ref_count++;
			}
			else
				DEBUG_LOG("Bucket: you must call Open before OpenBucket");
		}

		void Store(uint8_t idx, BucketType bucket)
		{
			if(object)
			{
				object[idx] = bucket;
				ref_count++;
			}
			else
				DEBUG_LOG("Bucket: you must call Open before OpenBucket");
		}

		ValueType& Remove(uint8_t idx)
		{
			ref_count--;
			return object[idx].Remove();
		}

		void Remove()
		{}

		bool ShoudClose()
		{
			return (ref_count == 0);
		}

		void Close()
		{
			DEBUG_LOG("Bucket Close");
			if (object)
			{
				delete[] object;
				object = NULL;
				ref_count = 0;
			}
			else
				DEBUG_LOG("Bucket: the bucket isn't still opened.");
		}
		
		void CloseBucket(uint8_t idx)
		{
			if(object)
			{
				object[idx].Close();
				ref_count--;
			}
			else
				DEBUG_LOG("Bucket: the bucket isn't still opened.");
		}

		BucketType* Get(uint8_t idx)
		{
			if(idx >= 0 && idx < 256 && object)
				return &object[idx];
			else
			{
				DEBUG_LOG("Bucket: idx = %i must be from 0 to 255" << idx);
				return NULL;
			}
		}
	};

private:
    Bucket<Bucket<Bucket<Bucket<MapObject>>>> m_dictionary;
    uint32_t m_size;
public:
    UInt32KeyMap()
    {
    	m_dictionary.Open();
    	m_size = 0;
    }

    ~UInt32KeyMap()
    {
    	DEBUG_LOG("UInt32KeyMap destructor");
    	for(uint16_t i = 0; i < 256; i++)
		{
			if(m_dictionary.Get(i)->IsOpen())
			{
				DEBUG_LOG("UInt32KeyMap destructor 1");
				for(uint16_t j = 0; j < 256; j++)
				{
					if(m_dictionary.Get(i)->Get(j)->IsOpen())
					{
						DEBUG_LOG("UInt32KeyMap destructor 2");
						for(uint16_t k = 0; k < 256; k++)
						{
							if(m_dictionary.Get(i)->Get(j)->Get(k)->IsOpen())
							{
								DEBUG_LOG("UInt32KeyMap destructor 3");
								m_dictionary.Get(i)->Get(j)->CloseBucket(k);
							}
						}
						m_dictionary.Get(i)->CloseBucket(j);
					}
				}
				m_dictionary.CloseBucket(i);
			}
		}
		m_dictionary.Close();
    }

    uint32_t Size()
    {
    	return m_size;
    }

    ValueType* Find(uint32_t key)
    {
	    uint8_t temp[4];
		temp[0] = (uint8_t) (key >> 24) & 0xFF;
		temp[1] = (uint8_t) (key >> 16) & 0xFF;
		temp[2] = (uint8_t) (key >> 8) & 0xFF;
		temp[3] = (uint8_t) key & 0xFF;
		if(m_dictionary.Get(temp[0])->IsOpen())
		{
			if(m_dictionary.Get(temp[0])->Get(temp[1])->IsOpen())
			{
				if(m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->IsOpen())
				{
					if(m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->Get(temp[3])->IsStored())
						return &m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->Get(temp[3])->Get();
				}
			}
		}
		return NULL;	
    }

    bool Insert(uint32_t key, ValueType object)
    {
	    uint8_t temp[4];
		temp[0] = (uint8_t) (key >> 24) & 0xFF;
		temp[1] = (uint8_t) (key >> 16) & 0xFF;
		temp[2] = (uint8_t) (key >> 8) & 0xFF;
		temp[3] = (uint8_t) key & 0xFF;
		if(!m_dictionary.Get(temp[0])->IsOpen())
			m_dictionary.OpenBucket(temp[0]);
		if(!m_dictionary.Get(temp[0])->Get(temp[1])->IsOpen())
			m_dictionary.Get(temp[0])->OpenBucket(temp[1]);
		if(!m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->IsOpen())
			m_dictionary.Get(temp[0])->Get(temp[1])->OpenBucket(temp[2]);
		MapObject mObj;
		mObj.Store(object);
		m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->Store(temp[3], mObj);
		m_size++;
    }

    ValueType* Erase(uint32_t key)
    {
	    uint8_t temp[4];
		temp[0] = (uint8_t) (key >> 24) & 0xFF;
		temp[1] = (uint8_t) (key >> 16) & 0xFF;
		temp[2] = (uint8_t) (key >> 8) & 0xFF;
		temp[3] = (uint8_t) key & 0xFF;
		ValueType* tmp = NULL;
		if(m_dictionary.Get(temp[0])->IsOpen())
		{
			if(m_dictionary.Get(temp[0])->Get(temp[1])->IsOpen())
			{
				if(m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->IsOpen())
				{
					if(m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->Get(temp[3])->IsStored())
					{
						tmp = &m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->Remove(temp[3]);
						m_size--;
					}
					if(m_dictionary.Get(temp[0])->Get(temp[1])->Get(temp[2])->ShoudClose())
						m_dictionary.Get(temp[0])->Get(temp[1])->CloseBucket(temp[2]);
				}
				if(m_dictionary.Get(temp[0])->Get(temp[1])->ShoudClose())
					m_dictionary.Get(temp[0])->CloseBucket(temp[1]);
			}
			if(m_dictionary.Get(temp[0])->ShoudClose())
				m_dictionary.CloseBucket(temp[0]);
		}
		return tmp;
    }

    void CleanMemory()
    {
    	for(uint16_t i = 0; i < 256; i++)
    	{
    		if(m_dictionary.Get(i)->IsOpen())
			{
				for(uint16_t j = 0; j < 256; j++)
				{
					if(m_dictionary.Get(i)->Get(j)->IsOpen())
					{
						for(uint16_t k = 0; k < 256; k++)
						{
							if(m_dictionary.Get(i)->Get(j)->Get(k)->IsOpen() && m_dictionary.Get(i)->Get(j)->Get(k)->ShoudClose())
							{
								m_dictionary.Get(i)->Get(j)->CloseBucket(k);
							}
						}
						if(m_dictionary.Get(i)->Get(j)->ShoudClose())
							m_dictionary.Get(i)->CloseBucket(j);
					}
				}
				if(m_dictionary.Get(i)->ShoudClose())
					m_dictionary.CloseBucket(i);
			}
    	}
    	m_size = 0;
    }

    void ClearAll()
    {
	    for(uint16_t i = 0; i < 256; i++)
		{
			if(m_dictionary.Get(i)->IsOpen())
			{
				for(uint16_t j = 0; j < 256; j++)
				{
					if(m_dictionary.Get(i)->Get(j)->IsOpen())
					{
						for(uint16_t k = 0; k < 256; k++)
						{
							if(m_dictionary.Get(i)->Get(j)->Get(k)->IsOpen())
							{
								m_dictionary.Get(i)->Get(j)->CloseBucket(k);
							}
						}
						m_dictionary.Get(i)->CloseBucket(j);
					}
				}
				m_dictionary.CloseBucket(i);
			}
		}
		m_size = 0;
    }
};

#endif	/* UInt32KeyMap_H */