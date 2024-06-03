#!/bin/bash
find . -type f -exec grep -Hn 'L"' {} \; 2>/dev/null | awk -F: '
{
    filename = $1
    line_number = $2
    # Reconstruct the rest of the line after the filename and line number
    line = $0
    sub(/^[^:]*:[0-9]+:/, "", line)
    while (match(line, /L"[^"]*"/)) {
        wide_string = substr(line, RSTART+2, RLENGTH-3)
        print filename ":" line_number ":" wide_string
        line = substr(line, RSTART + RLENGTH)
    }
}'
