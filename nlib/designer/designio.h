#pragma once

//---------------------------------------------


namespace NLIBNS
{


    // Class to write indentation to streams, to format c++ and object serialization.
    class Indentation
    {
    private:
        int level;

        bool usetab;
        int size;

        std::string str() const;
        std::wstring wstr() const;
    public:
        Indentation(bool usetab, int size);
        //explicit Indentation(int initlevel);
        Indentation(const Indentation &);

        Indentation& operator=(const Indentation &);
        Indentation& operator=(int);

        bool operator==(int);
        bool operator==(const Indentation &);

        Indentation& operator++();
        Indentation operator++(int);

        Indentation& operator--();
        Indentation operator--(int);

        friend std::wiostream& operator<<(std::wiostream&, const Indentation&);
        friend std::iostream& operator<<(std::iostream&, const Indentation&);
        friend std::wostream& operator<<(std::wostream&, const Indentation&);
        friend std::ostream& operator<<(std::ostream&, const Indentation&);
    };

    std::wiostream& operator<<(std::wiostream&, const Indentation&); // Writes indentation characters to the stream, but only if the stream is empty or the last character was a newline character.
    std::iostream& operator<<(std::iostream&, const Indentation&); // Writes indentation characters to the stream, but only if the stream is empty or the last character was a newline character.
    std::wostream& operator<<(std::wostream&, const Indentation&); // Writes indentation characters to the stream.
    std::ostream& operator<<(std::ostream&, const Indentation&); // Writes indentation characters to the stream.


    // Class to read object serialization token from project or clipboard. Reads a single token, keeping track of the current line in the stream.
    // Valid tokens are:
    //   unquoted words: /([a-zA-Z0-9_.-]*)/ or namespace strings: /((?:[a-zA-Z0-9_.-]+)(?:::[a-zA-Z0-9_.-]+)+/ depending on read mode 
    //   strings: /"([^\"]*(?:\\)*(?:\")*)*"/ (that is, between quotes. \ is the escape character for \ or ") The quote characters from the edges are dropped.
    //   template arguments: same as strings but between <> instead of quotes, and there is no escape character.
    //   Spaces, tabs and newline characters are skipped outside quotes and <>. Anything else is a single character token.

    // Type of last read token.
    enum ReadTokenType { rttNone, rttUnquoted, rttString, rttTemplateArg, rttToken };

    // Mode for the next token reading.
    //  rtmUnspecified: read a single token
    //  rtmSingleNamespace: read a token that can be made up of a namespace string directly followed by double colon and another string. If double colon is not found, the first read token is retrieved.
    //  rtmFullNamespace: Similar to rtmSingleNamespace, but reads any number of namespaces together as a single token.
    enum ReadTokenMode { rtmUnspecified, rtmSingleNamespace, rtmFullNamespace };

    class TokenStream
    {
    private:
        int linenum;
        int colpos;
        ReadTokenType type;
        std::wstring token; // Last read string from stream by read().
        ReadTokenType peektype;
        std::wstringstream peektoken; // Token last read by peek().
        bool peeked; // True when the last reading call was peek(). In that case the next call to read() will clear the peeked value and replace token with peektoken.

        ReadTokenMode pmode; // Mode of the last token reading or peeking. If peeking was used first, the mode must be the same for the next peek or read operation or an exception is thrown.
        int nmscpcnt; // The number of unquoted namespace tokens already read when accepting namespaces.

        std::wistream &stream;

        int nextchar(wchar_t c[3], ReadTokenType &rtype, ReadTokenMode mode); // Extracts at most 3 characters from the stream. Returns the number of characters extracted.

        friend class TokenE;
    public:
        TokenStream(std::wistream &stream);
        TokenStream(const TokenStream& other);

        bool empty(); // Returns true if no token was read or the next call to operator>> failed.
        bool read(ReadTokenMode mode = rtmUnspecified); // Reads the next token and returns whether a new token was found.
        std::wstring peek(ReadTokenMode mode = rtmUnspecified);
        std::wstring peek(ReadTokenType &ptype, ReadTokenMode mode = rtmUnspecified);
        void unread(); // Only valid after a call to read. Sets peeked to true like it only peeked the last value but not read it.
        ReadTokenType tokentype();
        const std::wstring& toString(); // Last read token.
        bool toInt(int&); // Last read token in int format, if applicable. Returns false otherwise.
        std::wstring position_str();
        bool operator==(const std::wstring &);
        bool operator!=(const std::wstring &);
        bool operator==(ReadTokenType rtype);
        bool operator!=(ReadTokenType rtype);

        int linenumber();
        int column();
    };

    class TokenE : Exception
    {
    private:
        typedef Exception   base;
        TokenStream token;
    public:
        TokenE(const std::wstring &text, const TokenStream &token) : base(text), token(token) {}
        TokenE(const wchar_t* ctext, const TokenStream &token) : base(ctext), token(token) {}

        virtual std::wstring what();
    };

    // Class for working with source files that must be read and modified by the designer. All getter and seeker functions skip comments, quoted strings and numbers.
    class SourceStream
    {
    public:
        // Position to be returned if needed. The members have the same meaning as the values of the same name in the class.
        struct Position
        {
            int linepref;
            int chpref;

            Position() {}
            Position(const Position &other) : linepref(other.linepref), chpref(other.chpref) {}
            Position(int linepref, int chpref) : linepref(linepref), chpref(chpref) {}

            bool operator==(const Position &other) { return linepref == other.linepref && chpref == other.chpref; }
            bool operator<(const Position &other) { return linepref < other.linepref || (linepref == other.linepref && chpref < other.chpref); }
            bool operator>(const Position &other) { return linepref > other.linepref || (linepref == other.linepref && chpref > other.chpref); }
            bool operator<=(const Position &other) { return linepref < other.linepref || (linepref == other.linepref && chpref <= other.chpref); }
            bool operator>=(const Position &other) { return linepref > other.linepref || (linepref == other.linepref && chpref >= other.chpref); }
        };
        typedef std::pair<Position, Position> Size;
    private:
        // The source stream being read line by line.
        std::wistream &stream;

        std::wstring line; // The current line being examined.
        // Last reading positions.
        int linenum; // Line number.
        int chnum; // Character number (or column number, as it is labeled in some programs) that marks the beginning of the current word on the current line.
        int wlen; // Length of the current word without its prefix in previous lines. The end of the word on the current line is chnum + wlen.
        int blevel; // Number of opening brackets passed without their closing pair. This value is increased every time an opening curly bracket is found and decreased for closing brackets.
        std::wstring prefix; // Beginning of the current word in previous lines if the lines were broken up with the '\' character.
        int linepref; // Line where the current word started. This can be different from linenum if there was a prefix present.
        int chpref; // Character where the current word started on line marked with linepref.

        bool fetchagain; // Unget called. The next 'get' will not read just restore the stream state to be like how it was before the unget. The next unget should do nothing.

        // Values storing the stream state before the unget. These values are std::swap-ed with the original values by an unget, and assigned before a get.
        std::wstring prevline;
        int prevlinenum;
        int prevchnum;
        int prevwlen;
        int prevblevel;
        std::wstring prevprefix;
        int prevlinepref;
        int prevchpref;

        void nextline(); // Reads the next line from the stream and updates the position.
        void skipspace(); // Skips to the next non-space character in the line or if a comment block found, to the end of the block even if it is on some other line.
        wchar_t nextchar(); // Gets the next character in the current position or 0 if line end or stream end was reached. This can also go to the next line if the previous one ended in '\'. The function also updates the prefix string for new lines and the wlen as well.
        bool emptyforward() const; // Returns whether the line contains only whitespace after the current position.

        // Record for updating the stream.
        struct UpdateRec
        {
            bool remove;
            Position from;
            std::wstring str; // For insertions.
            Position to; // Only used when remove is true.
            int priority;

            UpdateRec(const Position &from, const Position &to, int priority) : remove(true), from(from), to(to), priority(priority) {}
            UpdateRec(const Position &from, const std::wstring &str, int priority) : remove(false), from(from), str(str), priority(priority) {}
        };

        std::list<UpdateRec> record;
    public:
        SourceStream(std::wistream &stream);

        void rewind(); // Go to the front of the stream.
        void fforward(); // Go to the end of the stream. The stream will be set to eof.

        bool skip_to(const std:: wstring &str, int level = -1); // Reads the lines in the stream until the given str is found at the bracket level of level. If level is negative, any level is accepted
        bool eof() const; // End of file (stream) is reached. Nothing more to get.
        bool get(bool skipdirectives = true); // Returns the next word in the stream. Set skipdirectives to true if compiler directives must be ignored by get() calls.
        void unget(); // Puts the last got word back to the stream, which means the next call to get will do nothing.
        void append(); // Appends the result of the last get to the previous one. Using after an unget, a change of level, or before reading anything causes undefined behavior.
        std::wstring last(); // Returns the word last returned by get.
        int level(); // Returns the bracket level at the current position. When the opening bracket is returned by get, level is already increased.

        // Functions for recording updates to the stream. The updates are not committed until update is called. Record insertion works in FIFO order, unless the priority is defined. Lower priority values are inserted before higher ones and printed above.
        void record_update(const std::wstring &str); // Mark the word at the current position to be updated to str.
        void record_update(const Position &from, const Position &to, const std::wstring &str); // Inserts str at from and removes everything between from-to.
        void record_update(const Size &pos, const std::wstring &str); // Inserts str at from and removes everything between from-to.
        void record_insert_lines(std::wistream &stream, bool after, int priority = 100); // Mark position before or after the current word to insert lines in stream.
        void record_insert_lines(std::wistream &stream, const Position &pos, int priority = 100); // Mark position before or after the current word to insert lines in stream.
        void record_remove(const Position &pos); // Mark the characters between pos and the current position to be removed. The current position is before the last read word.

        void record_remove(const Position &from, const Position &to); // Remove characters between the two position.
        void record_remove(const Size &pos); // Remove characters between the two position.
        void record_insert(const Position &at, const std::wstring &str, int priority = 100); // Insert string at the passed position.

        void update(std::wstringstream &s); // Creates a copy of the stream set in the constructor with all recorded updates commited to the passed stream. Make sure this is not the same stream that was passed in the constructor. After calling this function, further use of the stream is undefined behavior.

        Position position(); // Returns the position of the current word, with its starting position, current position and current length.
        Position position_after(); // Returns the position after the current word.
        Size wordposition(); // Returns a pair of position and position_after.
        void skip_after(bool skipnextempty); // Sets the value of position_after to the start of the next line after the current word, in case the current word was the last in its line. White space can be after the word but not comments to count as the last. Set skipnextempty to skip the next line too if it's empty or only contains whitespace.
    };


}
/* End of NLIBNS */


