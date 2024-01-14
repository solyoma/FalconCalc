#pragma once


namespace NLIBNS
{


    class fs_codecvt_noconv : public std::codecvt<wchar_t, char, mbstate_t>
    {
    private:
        typedef std::codecvt<wchar_t, char, mbstate_t>  base;
        typedef mbstate_t stateT;
        typedef char    externT;
        typedef wchar_t internT;
        typedef base::result    result;

        bool textmode;
    protected:
        virtual ~fs_codecvt_noconv();

        virtual bool do_always_noconv() const throw(); // cppref says int return value, must change for mingw?
        virtual int do_encoding() const throw();

        virtual result do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next) const;
        virtual int do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const;
        virtual int do_max_length() const throw();
        virtual result do_out ( stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next ) const;
        virtual result unshift ( stateT& state, externT* to, externT* to_limit, externT*& to_next ) const;
    public:
        explicit fs_codecvt_noconv(bool textmode, size_t refs = 0);
    };

    class fs_codecvt_utf8 : public std::codecvt<wchar_t, char, mbstate_t>
    {
    private:
        typedef std::codecvt<wchar_t, char, mbstate_t>  base;
        typedef mbstate_t stateT;
        typedef char    externT;
        typedef wchar_t internT;
        typedef base::result    result;

        bool textmode;
    protected:
        virtual ~fs_codecvt_utf8();

        virtual bool do_always_noconv() const throw(); // cppref says int return value, must change for mingw?
        virtual int do_encoding() const throw();

        virtual result do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next) const;
        virtual int do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const;
        virtual int do_max_length() const throw();
        virtual result do_out ( stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next ) const;
        virtual result unshift ( stateT& state, externT* to, externT* to_limit, externT*& to_next ) const;
    public:
        explicit fs_codecvt_utf8(bool textmode, size_t refs = 0);
    };

    class fs_codecvt_ansi : public std::codecvt<wchar_t, char, mbstate_t>
    {
    private:
        typedef std::codecvt<wchar_t, char, mbstate_t>  base;
        typedef mbstate_t stateT;
        typedef char    externT;
        typedef wchar_t internT;
        typedef base::result    result;

        bool textmode;
    protected:
        virtual ~fs_codecvt_ansi();

        virtual bool do_always_noconv() const throw(); // cppref says int return value, must change for mingw?
        virtual int do_encoding() const throw();

        virtual result do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next) const;
        virtual int do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const;
        virtual int do_max_length() const throw();
        virtual result do_out ( stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next ) const;
        virtual result unshift ( stateT& state, externT* to, externT* to_limit, externT*& to_next ) const;
    public:
        explicit fs_codecvt_ansi(bool textmode, size_t refs = 0);
    };

    enum FileStreamEncoding { feDefault, feANSI, feUnicode, feUTF8 };

    class FileStream : public FILESTD wfstream
    {
    private:
        typedef FILESTD wfstream    base;
        typedef std::basic_istream<wchar_t> _istream;
        typedef std::basic_ostream<wchar_t> _ostream;

        FileStreamEncoding enc;
        bool textmode;
        void inner_setencoding(FileStreamEncoding newenc, bool textmode);

#ifdef _MSC_VER
        // Stops compiler complaining about built-in warnings in ms file stream classes.
        virtual void __CLR_OR_THIS_CALL _Add_vtordisp1() {}
        virtual void __CLR_OR_THIS_CALL _Add_vtordisp2() {}
#endif

    public:
        FileStream();
        FileStream(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, FileStreamEncoding encoding = feUTF8);
        FileStream(const std::wstring &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, FileStreamEncoding encoding = feUTF8);

        void open(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, FileStreamEncoding encoding = feUTF8);
        void open(const std::wstring &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, FileStreamEncoding encoding = feUTF8);

        FileStreamEncoding encoding(); // Returns the encoding that was used when opening a file.

        bool skipbom(); // Tries to set the get pointer after the BOM bytes at the current position. The failbit is set if the encoding is not feUnicode or feUTF8. If good() is false before execution, nothing happens. The badbit can be set for reading errors. Returns true if the BOM was skipped.
        void writebom(); // Writes the BOM bytes to the stream by calling write(...) independent of the current position. The failbit is set if the encoding is not feUnicode or feUTF8. The badbit is set for the standard write errors.
    };


}
/* End of NLIBNS */

