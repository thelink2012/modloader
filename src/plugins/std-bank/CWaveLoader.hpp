/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef CWAVELOADER_HPP
#define	CWAVELOADER_HPP

#include <cstdint>
#include <cstdio>
#include <windows.h>

class CWavePCM
{
    public:
        struct SubChunkHead
        {
            char        id[4];
            uint32_t    size;
        };
        
        struct Header
        {
            char            RIFF[4];
            uint32_t        chunk_size;
            char            WAVE[4];
            SubChunkHead    subchunk1;
            uint16_t        audio_format;
            uint16_t        num_channels;
            uint16_t        sample_rate;
            uint32_t        byte_rate;
            uint16_t        block_align;
            uint16_t        bps;            // bits per sample
            SubChunkHead    subchunk2;
        };
        
        
        static_assert(sizeof(Header) == 44, "__");
        
    private:
        HANDLE hFile;
        Header header;
        
    public:
        CWavePCM() : hFile(0)
        {}
        
        CWavePCM(const char* filename) : hFile(0)
        {
            this->Open(filename);
        }
        
        ~CWavePCM()
        {
            this->Close();
        }
        
        bool IsOpen()
        {
            return(hFile != 0 && hFile != INVALID_HANDLE_VALUE);
        }
        
        bool Open(const char* filename, bool bReadHeader = true)
        {
            this->Close();
            
            this->hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,
                                      0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

            if(hFile != INVALID_HANDLE_VALUE)
            {
                if(bReadHeader)
                {
                    if(this->ReadHeader())
                        return true;
                }
                else
                    return true;
            }
            
            this->Close();
            return false;
        }
        
        void Close()
        {
            if(IsOpen()) CloseHandle(hFile);
        }
        
        uint16_t GetNumChannels()
        { return header.num_channels; }
   
        uint16_t GetSampleRate()
        { return header.sample_rate; }
        
        uint32_t GetByteRate()
        { return header.byte_rate; }
       
        uint16_t GetSampleSize()
        { return header.block_align; }
        
        uint16_t GetBPS()
        { return header.bps; }
        
        uint32_t GetSoundBufferSize()
        { return header.subchunk2.size; }

        
        
        bool ReadHeader()
        {
            DWORD size;
            if(ReadFile(hFile, &this->header, sizeof(this->header), &size, nullptr) && size == sizeof(this->header))
            {
                if(!strncmp(header.RIFF, "RIFF", 4) && !strncmp(header.WAVE, "WAVE", 4)
                && !strncmp(header.subchunk1.id, "fmt ", 4) && !strncmp(header.subchunk2.id, "data", 4))
                {
                    if(header.subchunk1.size == 16 && header.audio_format == 1) // PCM format
                        return true;
                }
            }
            return false;
        }
        
        bool ReadSoundBuffer(void* pOutBuf)
        {
            if(ReadFile(hFile, pOutBuf, GetSoundBufferSize(), nullptr, nullptr))
                return true;
            return false;
        }
        
        bool ReadSoundBuffer(void* pOutBuf, size_t bufsize)
        {
            OVERLAPPED ov = {0};
            ov.Offset = sizeof(Header);
            
            if(ReadFile(hFile, pOutBuf, bufsize, nullptr, &ov))
                return true;
            return false;
        }
};


#endif
