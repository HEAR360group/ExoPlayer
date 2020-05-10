//
//  Util.h
//  HPSFD
//
//  Created by Richard Zhang on 5/4/16.
//  Copyright (c) 2016 Hear360, Co. All rights reserved.
//

#ifndef HPSFD_Util_h
#define HPSFD_Util_h

namespace Util
{
    static void CheckError(int status, const char* errorStr, bool quitOnError = false)
    {
        if(status != 0)
        {
            printf("%s: %d", errorStr, status);
            
//            if(quitOnError)
//                exit(-1);
        }
    }
    
    const int MAX_INT_24BIT = 1 << 23;
    const int MAX_INT_16BIT = 1 << 15;
    
    static bool safeFread(void* pDest, size_t size, size_t count, FILE* pStream)
    {
        if(fread(pDest, size, count, pStream) != count)
            return false;
        return true;
    }
    
    static bool safeFwrite(void* pSource, size_t size, size_t count, FILE* pStream)
    {
        if(fwrite(pSource, size, count, pStream) != count)
            return false;
        return true;
    }
    
    static bool safeFseek(FILE* pStream, long offset, int whence)
    {
        if(fseek(pStream, offset, whence) != 0)
            return false;
        return true;
    }
    
    static bool compareChars(const char* pStr1, const char* pStr2, int count)
    {
        for(int i = 0; i < count; i++)
            if(pStr1[i] != pStr2[i])
                return false;
        return true;
    }
    
    inline static int IntFrom24bitData(const unsigned char* pData)
    {
        unsigned int mask = ((pData[2] & 0x80) != 0) ? 0xFF000000 : 0;
        unsigned int intValue = mask | ((unsigned int)pData[2] << 16) | ((unsigned int)pData[1] << 8) | (unsigned int)pData[0];
        return ((int)intValue);
    }
    
    inline static short ShortFrom16bitData(const unsigned char* pData)
    {
        unsigned int mask = ((pData[1] & 0x80) != 0) ? 0xFFFF0000 : 0;
        unsigned int intValue = mask | ((unsigned int)pData[1] << 8) | (unsigned int)pData[0];
        return ((short)intValue);
    }
    
    inline static float floatFrom24bitData(const unsigned char* pData)
    {
        unsigned int mask = ((pData[2] & 0x80) != 0) ? 0xFF000000 : 0;
        unsigned int intValue = mask | ((unsigned int)pData[2] << 16) | ((unsigned int)pData[1] << 8) | (unsigned int)pData[0];
        return ((float)(int)intValue) / MAX_INT_24BIT;
    }
    
    inline static float floatFrom16bitData(const unsigned char* pData)
    {
        unsigned int mask = ((pData[1] & 0x80) != 0) ? 0xFFFF0000 : 0;
        unsigned int intValue = mask | ((unsigned int)pData[1] << 8) | (unsigned int)pData[0];
        return (float)((int)intValue) / MAX_INT_16BIT;
    }
    
    inline static void floatTo24bitData(float data, unsigned char* pData)
    {
        int intVal = (int)(data * MAX_INT_24BIT);
        pData[0] = (unsigned char)(intVal & 0x000000FF);
        pData[1] = (unsigned char)((intVal >> 8) & 0x000000FF);
        pData[2] = (unsigned char)((intVal >> 16) & 0x000000FF);
    }
    
    inline static void floatTo16bitData(float data, unsigned char* pData)
    {
        int intVal = (int)(data * MAX_INT_16BIT);
        
        pData[0] = intVal & 0x000000FF;
        pData[1] = (intVal >> 8) & 0x000000FF;
    }
    
    static unsigned int ChannelCountFromHex(unsigned int hex)
    {
        unsigned int result = 0;
        unsigned int flag = 0x1;
        
        for(unsigned int i = 0; i < 32; i++)
        {
            if((flag & hex) != 0)
            {
                result++;
            }
            
            flag = flag << 1;
        }
        
        return result;
    }
}

#endif
