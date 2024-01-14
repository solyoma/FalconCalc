#pragma once
// Simple wrappers for different kind of file pointers and streams to be used in templates that can work with all kinds of files.


namespace NLIBNS
{


enum FileReaderPosition { frpBegin, frpCurrent, frpEnd };
class IFileReaderBuffer
{
public:
    IFileReaderBuffer() {}
    virtual ~IFileReaderBuffer() {}

    virtual size_t Read(void *s, size_t count) = 0; // Reads count number of bytes to the memory position starting at s, and returns the actual number of bytes read.
    virtual size_t Tell() = 0; // Returns the current position in the file.
    virtual void Seek(ptrdiff_t pos, FileReaderPosition from) = 0; // Set the current file pointer to pos relative to from.
};

template<typename FTYPE>
class FileReaderBuffer : public IFileReaderBuffer
{
};

template<>
class FileReaderBuffer<FILE*> : public IFileReaderBuffer
{
public:
    FileReaderBuffer(FILE *f) : f(f)
    {
    }

    virtual size_t Read(void *s, size_t count)
    {
        return fread(s, 1, count, f);
    }

    virtual size_t Tell()
    {
        return ftell(f);
    }

    virtual void Seek(ptrdiff_t pos, FileReaderPosition from)
    {
        fseek(f, pos, from == frpBegin ? SEEK_SET : from == frpCurrent ? SEEK_CUR : SEEK_END);
    }
private:
    FILE *f;
};

template<>
class FileReaderBuffer<std::istream> : public IFileReaderBuffer
{
public:
    FileReaderBuffer(std::istream &f) : f(f)
    {
    }

    FileReaderBuffer(std::istream &&f) : f(f)
    {
    }

    virtual size_t Read(void *s, size_t count)
    {
        f.read((char*)s, count);
        return f.gcount();
    }

    virtual size_t Tell()
    {
        return f.tellg();
    }

    virtual void Seek(ptrdiff_t pos, FileReaderPosition from)
    {
        f.clear();
        f.seekg(pos, from == frpBegin ? std::ios_base::beg : from == frpCurrent ? std::ios_base::cur : std::ios_base::end);
    }
private:
    std::istream &f;
};

class FileReader
{
public:
    template<typename FTYPE>
    FileReader(FTYPE &f) : buffer(NULL)
    {
        buffer = new FileReaderBuffer<FTYPE>(f);
    }

    FileReader(FILE *f) : buffer(NULL)
    {
        buffer = new FileReaderBuffer<FILE*>(f);
    }

    ~FileReader()
    {
        delete buffer;
    }

    size_t Read(void *s, size_t count) // Reads count number of bytes to the memory position starting at s, and returns the actual number of bytes read.
    {
        return buffer->Read(s, count);
    }

    size_t Tell() // Returns the current position in the file.
    {
        return buffer->Tell();
    }

    void Seek(ptrdiff_t pos, FileReaderPosition from) // Set the current file pointer to pos relative to from.
    {
        buffer->Seek(pos, from);
    }
private:
    IFileReaderBuffer *buffer;
};


}
/* End of NLIBNS */

