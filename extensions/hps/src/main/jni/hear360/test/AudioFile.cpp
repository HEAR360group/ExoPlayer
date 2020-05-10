//
//  AudioFile.cpp
//  HPSFolddownMacApp
//
//  Created by Richard Zhang on 12/7/14.
//  Copyright (c) 2016 Hear360, Co. All rights reserved.
//

#include <hear360/test/AudioFile.h>
#include <stdexcept>
#include <cstring>
#include "Util.h"
#include <errno.h>

AudioFile::AudioFile()
{
    memset(pChunkId, 0, 4);
    chunkSize = 0;
    memset(pFormat, 0, 4);

    memset(pSubChunk1Id, 0, 4);
    subChunk1Size = 0;
    audioFormat = 0;
    numChannels = 0;
    sampleRate = 0;
    byteRate = 0;
    blockAlign = 0;
    bitsPerSample = 0;

    memset(pSubChunk2Id, 0, 4);
    subChunk2Size = 0;
    pContent = NULL;
}

AudioFile::~AudioFile()
{
    if(pContent != NULL)
        delete[] pContent;
}

AudioFile* AudioFile::CopyExceptHeader()
{
    AudioFile* copiedFile = new AudioFile();

    memcpy(&copiedFile->pChunkId, &pChunkId, 4);
    copiedFile->chunkSize = chunkSize;
    memcpy(&copiedFile->pFormat, &pFormat, 4);
    memcpy(&copiedFile->pSubChunk1Id, &pSubChunk1Id, 4);
    copiedFile->subChunk1Size = subChunk1Size;
    copiedFile->audioFormat = audioFormat;
    copiedFile->numChannels = numChannels;
    copiedFile->sampleRate = sampleRate;
    copiedFile->byteRate = byteRate;
    copiedFile->blockAlign = blockAlign;
    copiedFile->bitsPerSample = bitsPerSample;
    memcpy(&copiedFile->pSubChunk2Id, &pSubChunk2Id, 4);
    copiedFile->subChunk2Size = subChunk2Size;

    return copiedFile;
}

void AudioFile::ClearFile()
{
    memset(pChunkId, 0, 4);
    chunkSize = 0;
    memset(pFormat, 0, 4);

    memset(pSubChunk1Id, 0, 4);
    subChunk1Size = 0;
    audioFormat = 0;
    numChannels = 0;
    sampleRate = 0;
    byteRate = 0;
    blockAlign = 0;
    bitsPerSample = 0;

    memset(pSubChunk2Id, 0, 4);
    subChunk2Size = 0;

    if(pContent != NULL)
        delete[] pContent;
    pContent = NULL;
}

void AudioFile::SetMeta(unsigned int samplerate, unsigned int bitdepth, unsigned int channels, unsigned int frames)
{
    pChunkId[0] = 'R';
    pChunkId[1] = 'I';
    pChunkId[2] = 'F';
    pChunkId[3] = 'F';

    pFormat[0] = 'W';
    pFormat[1] = 'A';
    pFormat[2] = 'V';
    pFormat[3] = 'E';

    pSubChunk1Id[0] = 'f';
    pSubChunk1Id[1] = 'm';
    pSubChunk1Id[2] = 't';
    pSubChunk1Id[3] = ' ';

    pSubChunk2Id[0] = 'd';
    pSubChunk2Id[1] = 'a';
    pSubChunk2Id[2] = 't';
    pSubChunk2Id[3] = 'a';

    subChunk1Size = 16;
    audioFormat = 1;
    numChannels = channels;

    sampleRate = samplerate;
    bitsPerSample = bitdepth;
    blockAlign = bitdepth / 8 * channels;
    byteRate = samplerate * blockAlign;
    subChunk2Size = frames * 2 * channels;
    chunkSize = subChunk2Size + 36;
}

void AudioFile::SetContent(unsigned char* content, unsigned int chunks)
{
    subChunk2Size = chunks;
    for(int i = 0; i < chunks; i++) {
        pContent[i] = content[i];
    }
}

bool AudioFile::LoadWavFile(const char* pPath)
{
    FILE* pfile = NULL;
    try
    {
        try
        {
            pfile = fopen(pPath, "rb");
            if(pfile == NULL)
                throw std::runtime_error("\n\nfopen");

            //Chunk descriptor
            if(!Util::safeFread(pChunkId, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFread pChunkId");

            if(!Util::safeFread(&chunkSize, 1, 4, pfile))
                throw std::runtime_error("\n\nsafeFread chunkSize");

            if(!Util::safeFread(pFormat, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFread pFormat");

            //Skip junks chunks between descriptor and FMT
            char chunkId[4];
            unsigned int chunkSize = 0;

            while(true) {
                if(!Util::safeFread(chunkId, 1, 4, pfile))
                    throw std::runtime_error("\n\nsafeFread chunkId");

                if(!Util::safeFread(&chunkSize, 4, 1, pfile))
                    throw std::runtime_error("\n\nsafeFread chunkSize");

                if(Util::compareChars(chunkId, CHUNK1_ID, 4))
                    break;

                if(!Util::safeFseek(pfile, chunkSize, SEEK_CUR))
                    throw std::runtime_error("\n\nsafeFseek chunkSize");
            }

            //FMT subchunk
            memcpy(pSubChunk1Id, chunkId, 4);
            subChunk1Size = chunkSize;

            if(!Util::safeFread(&audioFormat, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFread audioFormat");

            if(!Util::safeFread(&numChannels, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFread numChannels");

            if(!Util::safeFread(&sampleRate, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFread sampleRate");

            if(!Util::safeFread(&byteRate, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFread byteRate");

            if(!Util::safeFread(&blockAlign, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFread blockAlign");

            if(!Util::safeFread(&bitsPerSample, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFread bitsPerSample");

            //Skip junks in FMT subchunk
            fseek(pfile, subChunk1Size - 16, SEEK_CUR);

            //Skip junks chunks between FMT and data
            while(true) {
                if(!Util::safeFread(chunkId, 1, 4, pfile))
                    throw std::runtime_error("\n\nsafeFread chunkId");

                if(!Util::safeFread(&chunkSize, 4, 1, pfile))
                    throw std::runtime_error("\n\nsafeFread chunkSize");

                if(Util::compareChars(chunkId, CHUNK2_ID, 4))
                    break;

                if(!Util::safeFseek(pfile, chunkSize, SEEK_CUR))
                    throw std::runtime_error("\n\nsafeFseek chunkSize");
            }

            //Data subchunk
            memcpy(pSubChunk2Id, chunkId, 4);
            subChunk2Size = chunkSize;

            //Print the audio file info
            PrintInfo();

            //Format check
            /*
            //Assert the audio file is stereo
            if(numChannels != 2)
                throw std::runtime_error("\n\nsafeFread numChannels");
             */

            //Assert sampleRate is 44100 or 48000
//            if(sampleRate != 44100 && sampleRate != 48000 && sampleRate != 88200 && sampleRate != 96000)
//                throw std::runtime_error("\n\nsafeFread sampleRate");

            //Assert bitsPerSample is 24 or 16
            if(bitsPerSample != 24 && bitsPerSample != 16)
                throw std::runtime_error("\n\nsafeFread bitsPerSample");

            //Read the content
            pContent = new unsigned char[subChunk2Size];
            if(!Util::safeFread(pContent, 1, subChunk2Size, pfile))
                throw std::runtime_error("\n\nsafeFread pContent");

            //Change chunk sizes for future saving files
            chunkSize = subChunk2Size + 36;
            subChunk1Size = 16;
        }
        catch (std::runtime_error& error)
        {
            if(pfile != NULL)
                if(fclose(pfile) != 0)
                    throw std::runtime_error("\n\nfclose");

            throw error;
        }

        if(pfile != NULL)
            if(fclose(pfile) != 0)
                throw std::runtime_error("\n\nfclose");
    }
    catch(std::runtime_error& error)
    {
        ClearFile();

        printf("%s\n", error.what());

        return false;
    }

    return true;
}

bool AudioFile::SaveWavFile(const char* pPath)
{
    FILE* pfile = NULL;
    try
    {
        try
        {
            pfile = fopen(pPath, "wb");
            if(pfile == NULL)
                throw std::runtime_error(std::strerror(errno));

            //Chunk descriptor
            if(!Util::safeFwrite(pChunkId, 4, 1, pfile))
                throw std::runtime_error(std::strerror(errno));

            if(!Util::safeFwrite(&chunkSize, 1, 4, pfile))
                throw std::runtime_error(std::strerror(errno));

            if(!Util::safeFwrite(pFormat, 4, 1, pfile))
                throw std::runtime_error(std::strerror(errno));

            //FMT subchunk
            if(!Util::safeFwrite(pSubChunk1Id, 1, 4, pfile))
                throw std::runtime_error("\n\nsafeFwrite pSubChunk1Id");

            if(!Util::safeFwrite(&subChunk1Size, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite subChunk1Size");

            if(!Util::safeFwrite(&audioFormat, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite audioFormat");

            if(!Util::safeFwrite(&numChannels, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite numChannels");

            if(!Util::safeFwrite(&sampleRate, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite sampleRate");

            if(!Util::safeFwrite(&byteRate, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite byteRate");

            if(!Util::safeFwrite(&blockAlign, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite blockAlign");

            if(!Util::safeFwrite(&bitsPerSample, 2, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite bitsPerSample");

            if(!Util::safeFwrite(pSubChunk2Id, 1, 4, pfile))
                throw std::runtime_error("\n\nsafeFwrite pSubChunk2Id");

            if(!Util::safeFwrite(&subChunk2Size, 4, 1, pfile))
                throw std::runtime_error("\n\nsafeFwrite subChunk2Size");

            PrintInfo();

            //Write the content
            if(!Util::safeFwrite(pContent, 1, subChunk2Size, pfile))
                throw std::runtime_error("\n\nsafeFwrite pContent");
        }
        catch (std::runtime_error& error)
        {
            if(pfile != NULL)
                if(fclose(pfile) != 0)
                    throw std::runtime_error("\n\nfclose");

            throw error;
        }

        if(pfile != NULL)
            if(fclose(pfile) != 0)
                throw std::runtime_error("\n\nfclose");
    }
    catch(std::runtime_error& error)
    {
        printf("%s\n", error.what());

        return false;
    }

    return true;
}

unsigned int AudioFile::GetTotalFrames()
{
    return (subChunk2Size / blockAlign);
}



void AudioFile::PrintInfo()
{
//    if(pChunkId != NULL)
        printf("pChunkId: %c%c%c%c\n", pChunkId[0], pChunkId[1], pChunkId[2], pChunkId[3]);
//    else
//        printf("pChunkId: NULL");

    printf("chunkSize %i\n", chunkSize);

//    if(pFormat != NULL)
        printf("pFormat: %c%c%c%c\n", pFormat[0], pFormat[1], pFormat[2], pFormat[3]);
//    else
//        printf("pFormat: NULL");

//    if(pSubChunk1Id != NULL)
        printf("pSubChunk1Id: %c%c%c%c\n", pSubChunk1Id[0], pSubChunk1Id[1], pSubChunk1Id[2], pSubChunk1Id[3]);
//    else
//        printf("pSubChunk1Id: NULL");

    printf("subChunk1Size: %i\n", subChunk1Size);
    printf("audioFormat: %i\n", audioFormat);
    printf("numChannels: %i\n", numChannels);
    printf("sampleRate: %i\n", sampleRate);
    printf("byteRate: %i\n", byteRate);
    printf("blockAlign: %i\n", blockAlign);
    printf("bitsPerSample: %i\n", bitsPerSample);

//    if(pSubChunk2Id != NULL)
        printf("pSubChunk2Id: %c%c%c%c\n", pSubChunk2Id[0], pSubChunk2Id[1], pSubChunk2Id[2], pSubChunk2Id[3]);
//    else
//        printf("pSubChunk2Id: NULL");

    printf("subChunk2Size %i\n", subChunk2Size);
}

void AudioFile::Deinterleave(float** outputBus)
{
    unsigned int samplesPerChannel = GetTotalFrames();

    //24bit
    if(bitsPerSample == 24)
    {
        for(unsigned i = 0; i < samplesPerChannel; i++)
        {
            for(unsigned j = 0; j < numChannels; j++)
            {
                outputBus[j][i] = Util::floatFrom24bitData(pContent + (i * numChannels + j) * 3);
            }
        }
    }
        //16bit
    else
    {
        for(unsigned i = 0; i < samplesPerChannel; i++)
        {
            for(unsigned j = 0; j < numChannels; j++)
            {
                outputBus[j][i] = Util::floatFrom16bitData(pContent + (i * numChannels + j) * 2);
            }
        }
    }
}

void AudioFile::Interleave(float** inputBus, unsigned int initialDelayFrames)
{
    unsigned int samplesPerChannel = GetTotalFrames();

    //24bit
    if(bitsPerSample == 24)
    {
        for(unsigned i = 0; i < samplesPerChannel; i++)
        {
            for(unsigned j = 0; j < numChannels; j++)
            {
                Util::floatTo24bitData(inputBus[j][i + initialDelayFrames], pContent + (i * numChannels + j) * 3);
            }
        }
    }
        //16bit
    else
    {
        for(unsigned i = 0; i < samplesPerChannel; i++)
        {
            for(unsigned j = 0; j < numChannels; j++)
            {
                Util::floatTo16bitData(inputBus[j][i + initialDelayFrames], pContent + (i * numChannels + j) * 2);
            }
        }
    }
}
