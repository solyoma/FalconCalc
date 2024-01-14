#pragma once

//---------------------------------------------


namespace NLIBNS
{


/* Class for unzipping a resource from the designer's executable to memory. */
class ResourceUnzipper
{
private:
    /* Properties of separate files when listing files to unzip from zipped resource. */
    struct ZippedData {
        unsigned int crc32;
        int pos; /* Position in zipped resource. */
        int compressed; /* Compressed size of data. */
        int uncompressed; /* Uncompressed size when data is unzipped. */
        std::wstring filename; /* Name of data. */
        int pathlen; /* Length of path in file name. Same as the position after the last '\' character. */
        byte method; /* Compression method. We can only handle 0 (no compression) and 8 (deflated). */
    };

    /* Unzipped data item. */
    struct UnzippedData
    {
        std::wstring filename; /* Name of data in zipped format. */
        std::wstring path; /* Relative path of data. */

        int size; /* Size of the binary data in data. */
        char *data; /* Raw data. */

        UnzippedData(); /* Zeroes the memory for the data and size members. */
        ~UnzippedData(); /* Frees the memory in data if it was allocated. */
        UnzippedData(UnzippedData &&other) noexcept; /* Moves the data from one UnzippedData to the new one, zeroing the old value. */
        UnzippedData& operator=(UnzippedData &&other) noexcept; /* Swaps two unzipped data objects. */
    };


    std::vector<ZippedData> items; /* Vector of data describing the items in the zipped data. */
    std::vector<UnzippedData> files; /* Vector of unzipped file data. */

    int siz; /* Size of buffer read from resource. */
    char *zip; /* Zipped data loaded from resource. */
    char *pos; /* Current position in the zipped data. */

    bool NextItem(); /* Reads the descriptor at the top of the zip file for the next item and adds it to the items vector. */
    void Clear(); /* Frees memory if any was allocated by a previous call to Unzip. */

    bool CheckCRC();
    bool Extract(); // Extracts data to the files vector.

    template<typename T> /* Read a variable of type T from pos and step by sizeof(T). */
    void Read(T& val)
    {
        if (pos - zip + (int)sizeof(T) > siz)
            throw L"Buffer overrun!";
        val = *(T*)pos;
        pos += sizeof(T);
    }

    template<typename T>
    int Read(T *val, int valsiz) /* Read to buffer of size valsiz. Returns the number of bytes read. The position in pos is increased by at most valsiz, or less if the zip buffer is not large enough. */
    {
        valsiz = min(valsiz, siz - (pos - zip));
        memcpy(val, pos, valsiz);
        pos += valsiz;
        return valsiz;
    }
public:
    bool Unzip(int resID); /* Unzippes data by ID of resource in ablakok.rc of the designer. Returns false on error. */

    int Count(); /* Number of items in the unzipped data. */
    const std::wstring& Names(int index); /* Returns the file name for the element at index. */
    const std::wstring& Paths(int index); /* Returns the relative file path for the element at index. */
    const char* RawData(int index, int &size); /* Returns the raw bytes for the file at index, setting size to the size of the data. */
    void Lines(int index, std::vector<std::string> &lines); /* Returns the data of the file at index, filling the lines vector with the lines in the text file. */
    void Lines8(int index, std::vector<std::wstring> &lines); /* Returns the data of the file at index, filling the lines vector with the lines in the UTF-8 text file. */
};


}
/* End of NLIBNS */


