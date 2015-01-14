/*
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#pragma once
#include <istream>
#include <ostream>

// Writes the size of the written content during the lifetime of the block_writer object
class block_writer
{
    public:
        // Reserves space for the 'how much data' integer
        block_writer(std::ostream& os) : os(os)
        {
            this->prevpos = os.tellp();
            os.seekp(sizeof(std::streamsize), std::ios::cur);
        }

        // Goes back to the previous reserved space and tells it how much data was written in between
        ~block_writer()
        {
            auto blocksize = std::streamsize(os.tellp() - prevpos);
            blocksize = blocksize - sizeof(blocksize);          // blocksize should not contain the blocksize variable itself in it
            os.seekp(prevpos, std::ios::beg);
            os.write((char*)&blocksize, sizeof(blocksize));
            os.seekp(blocksize, std::ios::cur);
        }

        block_writer(const block_writer&) = delete;
        block_writer(block_writer&&) = delete;
        block_writer& operator=(const block_writer&) = delete;
        block_writer& operator=(block_writer&&) = delete;

        // Writes an empty block in the specified stream
        static void empty(std::ostream& os)
        {
            block_writer block(os);
        }

    private:
        std::ostream::pos_type prevpos;
        std::ostream& os;

};

// Reads the size of the next block to be readen and allows skipping it
class block_reader
{
    public:
        // Reads the 'how much data' integer
        block_reader(std::istream& is) : is(is)
        {
            is.read((char*)&blocksize, sizeof(blocksize));
            skipper = is.tellg() + blocksize;
        }

        // Goes to the end of the current block
        void skip()
        {
            is.seekg(skipper);
        }

        block_reader(const block_reader&) = delete;
        block_reader(block_reader&&) = delete;
        block_reader& operator=(const block_reader&) = delete;
        block_reader& operator=(block_reader&&) = delete;

        // Skips the next block in the specified stream
        static void skip(std::istream& is)
        {
            block_reader block(is);
            block.skip();
        }

    private:
        std::streamsize blocksize;
        std::istream::pos_type skipper;
        std::istream& is;
};


