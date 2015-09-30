/**
 * Copyright (C) 2013-2015 Niklas Rosenstein
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include <c4d.h>
#include <lib_csv.h>
#include <cctype>  // isspace
#include <cstdlib> // strtoll

#if defined(_MSC_VER)
    #define strtoll _strtoi64
#endif

/**
 * Wrapper for the std `isspace` function which will yield an assertion
 * error when passing a unicode character.
 */
static Bool IsSpace(LONG chr) {
    if (chr < 0 || chr > 255) return FALSE;
    else return isspace(chr);
}


CSVReader::CSVReader(CHAR delimiter, Bool stripWhitespace)
: m_isOpened(FALSE), m_delimiter(delimiter), m_stripWhitespace(stripWhitespace),
  m_error(CSVError_None), m_encoding(STRINGENCODING_UTF8) {
}

CSVReader::~CSVReader() {
}

Bool CSVReader::GetRow(CSVRow& destRow) {
    if (!IsOpened() || AtEnd() || m_error != CSVError_None) {
        return FALSE;
    }

    c4d_misc::BaseArray<CHAR> charArray;
    CHAR current;
    do {
        // Read the next character from the input file. If the method returns
        // FALSE, the end of the file has been reached.
        if (!m_file->ReadChar(&current)) {
            m_atEnd = TRUE;
            break;
        }

        // If the current character is a new line, processing should stop.
        if (current == '\n') {
            break;
        }

        // Skip null-characters, we don't want them in a CSV file. :(
        if (current == 0) {
            continue;
        }

        // If the current character is a delimiter, append the current value
        // to the destination row.
        if (current == m_delimiter) {
            if (destRow.Append(CharArrayToString(charArray)) == NULL) {
                m_error = CSVError_Memory;
                break;
            }
            charArray.Flush();
        }
        // Otherwise, we'll just add the current character to the string.
        else if (charArray.Append(current) == NULL) {
            m_error = CSVError_Memory;
            break;
        }
    } while (TRUE);

    if (m_error != CSVError_None) {
        return FALSE;
    }

    // If there are any elements in the current string at this point, we
    // add it to the destination row as well.
    String lastString = CharArrayToString(charArray);
    if (lastString.Content()) {
        if (destRow.Append(lastString) == NULL) {
            m_error = CSVError_Memory;
        }
    }

    m_line++;
    return m_error == CSVError_None;
}

String CSVReader::CharArrayToString(const c4d_misc::BaseArray<CHAR>& arr) {
    String string;
    string.SetCString(arr.GetFirst(), arr.GetCount(), m_encoding);
    if (m_stripWhitespace) string = StringStripWhitespace(string);
    return string;
}


Bool BaseCSVTable::Init(const Filename& filename, Bool forceUpdate, Bool* didReload) {
    if (!forceUpdate && !CheckReload(filename)) return TRUE;
    Bool fExist = GeFExist(filename);
    if (!m_loaded && !fExist) return TRUE;
    if (didReload) *didReload = TRUE;
    m_loaded = FALSE;

    // Instruct the sub-class to flush all its stored information.
    FlushData();
    m_header.Reset();

    m_fileError = FILEERROR_NONE;
    m_error = LoadDataStart(filename);
    if (m_error != CSVError_None || !fExist) {
        LoadDataEnd(m_error);
        return m_error == CSVError_None;
    }

    // Open the CSV File and handle possible errors.
    CSVReader reader(m_delimiter);
    if (!reader.Open(filename)) {
        m_error = reader.GetError();
        m_fileError = reader.GetFileError();
        return FALSE;
    }

    // Read in the header of the CSV File if we were instructed to do so.
    if (m_hasHeader) {
        if (!reader.GetRow(m_header)) {
            LoadDataEnd(m_error);
            return TRUE;
        }
        m_error = ProcessHeader(m_header);
    }

    // Read in each row.
    CSVRow tempRow;
    while (m_error == CSVError_None && !reader.AtEnd() && reader.GetRow(tempRow)) {
        m_error = reader.GetError();
        if (m_error == CSVError_None) {
            // Store the current row.
            m_error = StoreRow(tempRow);
            tempRow.Flush();
        }
    }

    LoadDataEnd(m_error);
    m_loaded = m_error == CSVError_None;
    return m_loaded;
}

Bool BaseCSVTable::CheckReload(const Filename& filename) {
    LocalFileTime fileTime;
    GeGetFileTime(filename, GE_FILETIME_MODIFIED, &fileTime);

    Bool exists = GeFExist(filename);
    Bool differs = filename != m_filename;
    Bool modified = exists && fileTime > m_fileTime;

    m_filename = filename;
    m_fileTime = fileTime;

    // Update if the filenames are different or if it was modified. Also update if
    // if the file does not exist since this will cause flusing of the stored data.
    Bool reload = differs || !exists || modified;
    /** /
    // DEBUG:
    if (reload) {
        #define BTOS(x) String((x) ? "true" : "false")
        GePrint(__FUNCTION__ ": Reloading(differs: " + BTOS(differs) + ", exists: " + BTOS(exists) + ", modified: " + BTOS(modified));
    }
    /**/

    return reload;
}


String StringStripWhitespace(const String& ref) {
    LONG start, end;
    LONG count = ref.GetLength();

    for (start=0; start < count; start++) {
        if (!IsSpace(ref[start])) break;
    }

    for (end=count - 1; end >= start; end--) {
        if (!IsSpace(ref[end])) break;
    }

    return ref.SubStr(start, end - start + 1);
}

LONG StringToLong(const String& str) {
    CHAR* cstr = str.GetCStringCopy();
    if (!cstr) {
        return 0;
    }
    long long int value = strtoll(cstr, NULL, 10);
    GeFree(cstr);

    return value;
}

Real StringToReal(const String& str) {
    CHAR* cstr = str.GetCStringCopy();
    if (!cstr) {
        return 0;
    }
    Real value = strtod(cstr, NULL);
    GeFree(cstr);

    return value;
}


