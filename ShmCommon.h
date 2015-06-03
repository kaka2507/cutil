/* 
 * File:   ShmCommon.h
 * Author: baodn
 * 
 * Created on June 6, 2013, 2:02 PM
 */
 
#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <sys/shm.h>
#include <string.h>

#define PERMISSION_FLAG 0600

inline void* MGetShm(int key, unsigned int size, int flags)
{
	int shmId;
	long ret;

	if ((shmId = shmget(key, size, flags)) == -1)
		return NULL;

	if ((ret = (long)shmat(shmId, NULL, 0)) == -1)
		return NULL;
	return (void *)ret;
}

inline int InitSharedMem(void* &data, int shm_key, uint32_t shm_size)
{
	int ret = -1;
	data = (void*)MGetShm(shm_key, shm_size, PERMISSION_FLAG);
	if (data)
	{
		ret = 1;
	}
	if(ret < 0)
	{
		data = (void*)MGetShm(shm_key, shm_size, PERMISSION_FLAG | IPC_CREAT);
		if(data)
		{
			ret = 0;
			memset(data, 0, shm_size);
		}
	}
	return ret;
}

#endif //SHM_COMMON_H
