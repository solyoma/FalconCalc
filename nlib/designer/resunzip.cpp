#include "stdafx_zoli.h"
#include "resunzip.h"
#include "zlib/zlib.h"

//---------------------------------------------


namespace NLIBNS
{


ResourceUnzipper::UnzippedData::UnzippedData() : size(0), data(NULL)
{
}

ResourceUnzipper::UnzippedData::~UnzippedData()
{
    delete[] data;
}

ResourceUnzipper::UnzippedData::UnzippedData(UnzippedData &&other) noexcept
{
    filename = std::move(other.filename);
    path = std::move(other.path);
    size = other.size;
    data = other.data;
    other.data = 0;
}

auto ResourceUnzipper::UnzippedData::operator=(UnzippedData &&other) noexcept -> UnzippedData&
{
    std::swap(filename, other.filename);
    std::swap(path, other.path);
    std::swap(size, other.size);
    std::swap(data, other.data);
    return *this;
}


//---------------------------------------------


void ResourceUnzipper::Clear()
{
    items.resize(0);
    files.resize(0);
}

bool ResourceUnzipper::Unzip(int resID)
{
    Clear();

    HRSRC data = FindResource((HMODULE)NULL, MAKEINTRESOURCE(resID), RT_RCDATA);
    if (!data)
        return false; // Error, resource not found.

    siz = SizeofResource((HMODULE)NULL, data);
    HGLOBAL res = LoadResource((HMODULE)NULL, data);
    if (res == NULL)
        return false;

    zip = (char*)LockResource(res);
    pos = zip;

    do
    {
        if (pos - zip + (int)sizeof(int) > siz || (*(int*)pos) != 0x04034b50)
            break;
        pos += sizeof(int);

        if (!NextItem())
            return false;
    } while(true);

    if (!CheckCRC()) // Not necessary as we trust the resource in the executable, but do it just to make sure.
        return false;

    return Extract();
}

bool ResourceUnzipper::NextItem()
{
    ZippedData zp;
    WORD w, namelen, extralen;
    try
    {
        Read(w); // Min version.
        Read(w); // Gen bit.
        Read(w); // Compression method.
        zp.method = w;
        Read(w); // Last mod time.
        Read(w); // Last mod date.

        Read(zp.crc32); // CRC32.
        Read(zp.compressed); // Compressed size.
        Read(zp.uncompressed); // Full uncompressed size.

        Read(namelen); // File name length.
        Read(extralen); // Extra fields length.

        // Copy the single character string file name to the wstring of zp, converting / characters to \ if found and setting the length of the path within the name.
        zp.filename.resize(namelen);
        zp.pathlen = 0;
        int namepos = 0;
        for (int ix = 0; ix < namelen; ++ix)
        {
            if (pos[ix] == '/')
            {
                zp.filename[namepos++] = L'\\';
                zp.pathlen = namepos;
            }
            else
            {
                zp.filename[namepos++] = pos[ix];
                if (pos[ix] == '\\')
                    zp.pathlen = namepos;
            }
        }
        pos += namelen;

        pos += extralen; // Skip extra data
        zp.pos = pos - zip;

        pos += zp.compressed; // Skip file to the data of the next one.

        if (zp.method == 0 || zp.method == 8)
            items.push_back(zp);
    }
    catch(...)
    {
        return false;
    }
    return true;
}


bool ResourceUnzipper::CheckCRC()
{
    for(auto it = items.begin(); it != items.end(); ++it)
    {
        ZippedData &data = *it;
        if (!data.uncompressed && !data.filename.empty() && data.filename.back() == L'\\') // folder
            continue;

        pos = zip + data.pos;;

        z_stream strm;
        strm.zalloc = NULL;
        strm.zfree = NULL;
        strm.opaque = NULL;
        strm.avail_in = 0;
        strm.next_in = NULL;
        int r = inflateInit2_(&strm, -15, ZLIB_VERSION, sizeof(strm));
        if (r != Z_OK)
            return false;

        unsigned int crc = 0;
        crc = crc32(crc, NULL, 0);

        const unsigned int CHUNK = 16384;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        try
        {
            do
            {
                strm.avail_in = Read(in, CHUNK);
                if (strm.avail_in == 0)
                    break;
                strm.next_in = in;

                /* run inflate() on input until output buffer not full */
                do
                {
                    strm.avail_out = CHUNK;
                    strm.next_out = out;

                    r = inflate(&strm, Z_NO_FLUSH);
                    switch (r) {
                    case Z_NEED_DICT:
                        r = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        throw r;
                    }

                    crc = crc32(crc,out,CHUNK - strm.avail_out);
                } while (strm.avail_out == 0);

                /* done when inflate() says it's done */
            } while (r != Z_STREAM_END);

        }
        catch(...)
        {
            inflateEnd(&strm);
            return false;
        }
        inflateEnd(&strm);

        if (crc != data.crc32)
            return false;
    }
    return true;
}

bool ResourceUnzipper::Extract()
{
    const unsigned int CHUNK = 16384;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    for (auto it = items.begin(); it != items.end(); ++it)
    {
        ZippedData &data = *it;

        if (!data.uncompressed && !data.filename.empty() && data.filename.back() == '\\') // Ignore folders for now.
            continue;

        pos = zip + data.pos;

        z_stream strm;
        strm.zalloc = NULL;
        strm.zfree = NULL;
        strm.opaque = NULL;
        strm.avail_in = 0;
        strm.next_in = NULL;
        int r = inflateInit2_(&strm, -15,ZLIB_VERSION, sizeof(strm));
        if (r != Z_OK)
            return false;

        UnzippedData udata;
        udata.filename = data.filename.substr(data.pathlen);
        udata.path = data.filename.substr(0, data.pathlen - 1);
        int rawpos = 0; // Number of uncompressed bytes in current item.
        udata.size = data.uncompressed;
        char *c = udata.data = new char[udata.size];
        files.push_back(std::move(udata));
        try
        {
            do
            {
                strm.avail_in = Read(in, CHUNK);
                if (strm.avail_in == 0)
                    break;
                strm.next_in = in;

                /* run inflate() on input until output buffer not full */
                do
                {
                    strm.avail_out = CHUNK;
                    strm.next_out = out;

                    r = inflate(&strm, Z_NO_FLUSH);
                    switch (r) {
                    case Z_NEED_DICT:
                        r = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        throw r;
                    }

                    int usize = CHUNK - strm.avail_out;
                    // Raw data to buffer.
                    if (rawpos + usize > data.uncompressed)
                        throw L"Overflow in uncompressed data.";
                    memcpy(c + rawpos, out, usize);
                    rawpos += usize;
                    //if (fwrite(out, 1, CHUNK - strm.avail_out, f2) != CHUNK - strm.avail_out)
                    //    throw "Error writing file!";
                } while (strm.avail_out == 0);

                /* done when inflate() says it's done */
            } while (r != Z_STREAM_END);

        }
        catch(...)
        {
            inflateEnd(&strm);
            return false;
        }
        inflateEnd(&strm);
    }
    return true;
}

int ResourceUnzipper::Count()
{
    return files.size();
}

const std::wstring& ResourceUnzipper::Names(int index)
{
    return files[index].filename;
}

const std::wstring& ResourceUnzipper::Paths(int index)
{
    return files[index].path;
}

const char* ResourceUnzipper::RawData(int index, int &size)
{
    size = files[index].size;
    return files[index].data;
}

void ResourceUnzipper::Lines(int index, std::vector<std::string> &lines)
{
    UnzippedData &data = files[index];

    char *pos = data.data; // Current position in the file data.
    int len = 0; // Length of the current line.
    while (pos < data.data + data.size)
    {
        while (pos + len < data.data + data.size && pos[len] != '\r' && pos[len] != '\n')
            ++len;
        lines.push_back(std::move(std::string(pos, len)));
        pos += len;
        if (pos < data.data + data.size)
        {
            if (*pos == '\r' && pos + 1 < data.data + data.size && *(pos + 1) == '\n')
                ++pos;
            ++pos;
        }
    }

}

void ResourceUnzipper::Lines8(int index, std::vector<std::wstring> &lines)
{
    const char ascii = ~((char)(1 << 7)); // Mask for ascii characters.
    const char code2ck = (char)0xE0; // Checker mask for code points that take up 2 bytes.
    const char code3ck = (char)0xF0; // Checker mask for code points that take up 3 bytes.
    const char code2 = (char)0xC0; // Mask for code points that take up 2 bytes.
    const char code3 = (char)0xE0; // Mask for code points that take up 3 bytes.
    const char codeck = (char)0xC0; // Checker mask for the latter bytes of the code point.
    const char code = (char)0x80; // Mask for the latter bytes of the code point.

    UnzippedData &data = files[index];

    char *pos = data.data; // Current position in the file data.
    // Skip BOM.
    if (data.size >= 3 && *pos == (char)0xEF && *(pos + 1) == (char)0xBB && *(pos + 2) == (char)0xBF)
        pos += 3;

    int clen = 0; // Length of the current character in bytes in UTF-8.
    int cpos = 0; // Number of bytes processed of the current UTF-8 character.
    std::wstringstream ss;

    bool lastr = false;

    wchar_t w;
    while (pos < data.data + data.size)
    {
        if (!clen) // New character.
        {
            if (((*pos) & ascii) == *pos)
            {
                clen = 1;
                w = *pos;
            }
            else if (((*pos) & code2ck) != code2 && ((*pos) & code3ck) != code3)
                throw L"Unsupported code point or error in UTF-8 format!";
            else if (((*pos) & code2ck) == code2)
            {
                clen = 2;
                w = WORD(*pos & ~code2) << (3 + 3); //110xxxxx 10yyyyyy -> 00000xxx|xxyyyyyy
            }
            else
            {
                clen = 3;
                w = WORD(*pos & ~code3) << (4 + 8); // 1110xxxx 10yyyyyy 10zzzzzz -> xxxxyyyy|yyzzzzzz
            }
        }
        else
        {
            if (((*pos) & codeck) != code)
                throw L"Invalid UTF-8 format! Expected 0x10xxxxxx.";
            if (clen == 2 || (clen == 3 && cpos == 2))
                w |= *pos & ~code;
            else if (clen == 3)
                w |= WORD(*pos & ~code) << (2 + 4);
        }

        if (++cpos == clen)
        {
            if (w != L'\n' || !lastr)
            {
                if (w == L'\r' || w == L'\n')
                {
                    lines.push_back(ss.str());
                    lastr = w == L'\r';
                    ss.clear();
                    ss.str(L"");
                }
                else
                {
                    lastr = false;
                    ss << w;
                }
            }
            else
                lastr = false;
            clen = 0;
            cpos = 0;
        }

        ++pos;
    }
    if (clen || cpos)
        throw L"Unexpected end of UTF-8 file!";
    if (ss.tellp() != (std::wstringstream::pos_type)0)
        lines.push_back(ss.str());
}


//---------------------------------------------


}
/* End of NLIBNS */

