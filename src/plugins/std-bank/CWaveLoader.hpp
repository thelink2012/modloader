/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef CWAVELOADER_HPP
#define	CWAVELOADER_HPP

#include <cstdint>
#include <cstdio>

/*
 *  CWavePCM 
 *      Reads WAVE files with PCM format
 *      Built to be used on little-endian machines!
 */
class CWavePCM
{
    public:
        struct SubChunkHead // SubChunk header
        {
            char        id[4];
            uint32_t    size;
        };
        
        struct RIFFHeader   // RIFF header
        {
            char            RIFF[4];
            uint32_t        chunk_size;
            char            WAVE[4];
        };
        
        struct FMTHeader    // WAVE FORMAT header
        {
            SubChunkHead    head;
            uint16_t        audio_format;
            uint16_t        num_channels;
            uint32_t        sample_rate;
            uint32_t        byte_rate;
            uint16_t        block_align;
            uint16_t        bps;            // bits per sample
        };
        
        struct DATAHeader   // WAVE DATA header
        {
            SubChunkHead    head;
        };
        
        
    private:
        FILE*       f;                  // File handle
        FMTHeader   fmt;                // FMTHeader
        DATAHeader  data;               // DATAHeader
        uint32_t    fmt_offset;         // Offset for FMTHeader on the file
        uint32_t    data_offset;        // Offset for DATAHeader on the file
        
    public:
        
        // Construct
        CWavePCM() : f(0), fmt_offset(0), data_offset(0)
        {}
        
        // Construct with open
        CWavePCM(const char* filename) : f(0), fmt_offset(0), data_offset(0)
        {
            this->Peek(filename);
        }
        
        // Destruct, closing the file handle
        ~CWavePCM()
        {
            this->Close();
        }
        
        // Checks if has wave information in the structure
        bool HasChunks()
        {
            return(fmt_offset && data_offset);
        }
        
        // Checks if the wave is open
        bool IsOpen()
        {
            return(f != 0);
        }
        
        // Close the file
        void Close()
        {
            if(IsOpen())
            {
                fclose(f);
                f = 0;
            }
        }
        
        // Open the wave file for reading
        bool Open(const char* filename)
        {
            this->Close();  // Close previous file
            
            if(this->f = fopen(filename, "rb"))
            {
                return (this->ReadHeader() && this->CheckPCM());
            }
            
            this->Close();  // Failed, close
            return false;
        }
        
        // Open the wave file, read it's header and basic chunks and closes it
        bool Peek(const char* filename)
        {
            if(this->Open(filename))
            {
                this->Close();
                return true;
            }
            return false;
        }
        
        
        
        /* --------- Chunk Reading  --------- */
        
        
        // Checks if subchunk @chunk is of type @chunkname
        bool IsChunk(const SubChunkHead& chunk, const char* chunkname)
        {
            return !strncmp(chunk.id, chunkname, 4);
        }
        
        // Copies subchunk header @head to chunk structure @chunk
        template<class T>
        void CopyHead(const SubChunkHead& head, T& chunk)
        {
            chunk.head = head;
        }

        // Skips the chunk @head
        // File pointer must be after @head
        // Returns the @head offset or 0 on failure
        uint32_t ReadChunk(const SubChunkHead& head)
        {
            size_t offset = ftell(f) - sizeof(head);
            if(fseek(f, head.size, SEEK_CUR)) return 0;
            return offset;
        }
        
        // Reads the chunk @head to @chunk
        // File pointer must be after @head
        // Returns the @head offset or 0 on failure
        template<class T>
        uint32_t ReadChunk(const SubChunkHead& head, T& chunk)
        {
            size_t chunk_size = sizeof(chunk) - sizeof(head);
            if(head.size >= chunk_size) // Check if head size suffices chunk size
            {
                size_t offset = ftell(f) - sizeof(head);    // Find head offset
                size_t read_size = head.size > chunk_size? chunk_size : head.size;  // Find ammount of bytes to read
                
                // Finally, read the chunk
                if(fread((char*)(&chunk) + sizeof(chunk.head), read_size, 1, f))
                {
                    // If there's more data on this chunk, skip it
                    if(head.size <= read_size || !fseek(f, head.size - read_size, SEEK_CUR))
                        return offset;
                }
            }
            return 0;
        }
        
        // Reads the RIFF header
        bool ReadHeader()
        {
            RIFFHeader header;
            SubChunkHead sub;
            size_t offset;
            this->data_offset = this->fmt_offset = 0;
            
            // Read the RIFF header
            if(fread(&header, sizeof(header), 1, f))
            {
                // Check if it's WAVE format
                if(!strncmp(header.RIFF, "RIFF", 4) && !strncmp(header.WAVE, "WAVE", 4))
                {
                    // Keep reading chunks until the end of the WAVE
                    while(ftell(f) < (header.chunk_size - 8))
                    {
                        // We don't need to continue iterating if we already found data and fmt
                        if(this->data_offset && this->fmt_offset)
                            break;
                        
                        // Read chunk
                        if(fread(&sub, sizeof(sub), 1, f))
                        {
                            if(IsChunk(sub, "data"))
                            {
                                // Data chunk, we need to take care of it
                                CopyHead(sub, this->data);
                                this->data_offset = offset = ReadChunk(sub);
                            }
                            else if(IsChunk(sub, "fmt "))
                            {
                                // Fmt chunk, we need to take care of it
                                CopyHead(sub, this->fmt);
                                this->fmt_offset = offset = ReadChunk(sub, this->fmt);
                            }
                            else
                            {
                                // Another chunk, ignore
                                offset = ReadChunk(sub);
                            }
                            
                            // Couldn't read the chunk? Ow, that's bad
                            if(offset == 0) break;
                        }
                    }
                    
                    // Check if we found data and fmt
                    if(this->data_offset && this->fmt_offset)
                        return true;
                }
            }
            
            return false;
        }
        
        
        
        /* Information getter */
        
        // Related to format
        uint16_t GetNumChannels()
        { return fmt.num_channels; }
   
        uint16_t GetSampleRate()
        { return fmt.sample_rate; }
        
        uint32_t GetByteRate()
        { return fmt.byte_rate; }
       
        uint16_t GetSampleSize()
        { return fmt.block_align; }
        
        uint16_t GetBPS()
        { return fmt.bps; }
        
        bool CheckPCM() // Checks if format is PCM
        {
            return fmt.audio_format == 1;
        }
        
        
        // Related to data
        uint32_t GetSoundBufferOffset()
        { return data_offset + sizeof(data); }
        
        uint32_t GetSoundBufferSize()
        { return data.head.size; }
        
        void* GetSoundBuffer(void* out)
        {
            // No need to implement such a thing, we won't use it
            return nullptr;
        }
        
        
};


#endif
