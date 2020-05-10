//
//  AudioFile.h
//  HPSFolddownMacApp
//
//  Created by Richard Zhang on 12/7/14.
//  Copyright (c) 2016 Hear360, Co. All rights reserved.
//

#ifndef __HPSFolddownMacApp__AudioFile__
#define __HPSFolddownMacApp__AudioFile__

#include <iostream>

class AudioFile
{
private:
    const char CHUNK1_ID[4] = {'f','m','t',' '};
    const char CHUNK2_ID[4] = {'d','a','t','a'};

public:
    unsigned char   pChunkId[4];    //4 Bytes
    unsigned int    chunkSize;      //4 Bytes
    unsigned char   pFormat[4];        //4 Bytes

    unsigned char   pSubChunk1Id[4];//4 Bytes
    unsigned int    subChunk1Size;  //4 Bytes
    unsigned short  audioFormat;    //2 Bytes
    unsigned short  numChannels;    //2 Bytes
    unsigned int    sampleRate;     //4 Bytes
    unsigned int    byteRate;       //4 Bytes
    unsigned short  blockAlign;     //2 Bytes
    unsigned short  bitsPerSample;  //2 Bytes

    unsigned char   pSubChunk2Id[4];//4 Bytes
    unsigned int    subChunk2Size;  //4 Bytes
    unsigned char*  pContent;       //subChunk2Size

    AudioFile();
    ~AudioFile();

    AudioFile* CopyExceptHeader();
    void ClearFile();
    bool LoadWavFile(const char* pPath);
    bool SaveWavFile(const char* pPath);
    unsigned int GetTotalFrames();
    void SetMeta(unsigned int samplerate, unsigned int bitdepth, unsigned int channels, unsigned int frames);
    void SetContent(unsigned char* content, unsigned int chunks);
    void PrintInfo();
    void Deinterleave(float** outputBus);
    void Interleave(float** inputBus, unsigned int initialDelayFrames);
};


#endif /* defined(__HPSFolddownMacApp__AudioFile__) */
