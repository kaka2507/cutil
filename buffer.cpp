#include "buffer.h"

CVBuffer::CVBuffer(void)
{
	memset(m_Buffer, 0, MAX_CVBUFFER_SIZE);
	Clear();
}

CVBuffer::~CVBuffer(void)
{
}

bool CVBuffer::setBuffer(const char* pBuffer, const unsigned int uSize)
{
	if( pBuffer == NULL || uSize == 0)return false;

	if(uSize>MAX_CVBUFFER_SIZE)return false;

	memcpy(m_Buffer,pBuffer,uSize);

	m_uOffset=uSize;
	m_uGetOffset = 0;

	return true;
}

std::string CVBuffer::Dump() {
	static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A','B','C','D','E','F'};
	std::string str;
	for (int i = 0; i < m_uOffset; ++i) {
		const char ch = m_Buffer[i];
		str.append(&hex[(ch  & 0xF0) >> 4], 1);
		str.append(&hex[ch & 0xF], 1);
	}
	return str;
}

bool CVBuffer::setSize(unsigned int uSize)
{
	m_uOffset=uSize;
	m_uGetOffset = 0;

	return true;
}

bool CVBuffer::storageData(unsigned short usLen, CVBuffer& buf)
{    
    Clear();

    *this << usLen;
    m_uOffset=usLen + sizeof(unsigned short);
    m_uGetOffset = 0;

    buf.Get(m_Buffer + sizeof(unsigned short), (unsigned int)usLen);

    return true;    
}

bool CVBuffer::storageData(unsigned char ucLen, CVBuffer& buf)
{
    Clear();

    *this << ucLen;
    m_uOffset=ucLen + sizeof(unsigned char);
    m_uGetOffset = 0;

    buf.Get(m_Buffer + sizeof(unsigned char), (unsigned int)ucLen);

    return true;    
}


bool CVBuffer::Append(const char* pData, const unsigned int uLength)
{
	if(pData == NULL || uLength == 0)return false;

	unsigned int uSize = uLength;

	if(m_uOffset+uSize>MAX_CVBUFFER_SIZE)return false;
	
	memcpy(m_Buffer+m_uOffset,pData,uSize);

	m_uOffset+=uSize;

	return true;
}


bool CVBuffer::Get(char* pData, const unsigned int uLength)
{
	if( pData == NULL || uLength == 0)return false;

	unsigned int uSize = uLength;

	if(m_uGetOffset+uSize>m_uOffset)return false;

	memcpy(pData,m_Buffer+m_uGetOffset,uSize);

	m_uGetOffset+=uSize;

	return true;
}

bool CVBuffer::Copy(char* pData,unsigned int uLength)
{
    if( pData == NULL || uLength == 0)return false;

    unsigned int uSize = uLength;

    if(m_uGetOffset+uSize>m_uOffset)return false;

    memcpy(pData,m_Buffer+m_uGetOffset,uSize);

    return true;
}

const unsigned int CVBuffer::getSize() const 
{
	return m_uOffset;
}

const char* CVBuffer::getBuffer() const
{
	return m_Buffer;
}

char* CVBuffer::getBegin()
{
    return m_Buffer;
}

void CVBuffer::Clear()
{
	m_uOffset=0;
	m_uGetOffset = 0;
	//memset(m_Buffer,0,MAX_CVBUFFER_SIZE);
}
