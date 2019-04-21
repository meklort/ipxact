////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/regular_expressions.cpp
///
/// @project    ipxact
///
/// @brief      Regular expression support routines
///
////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
///
/// @copyright Copyright (c) 2019, Evan Lojewski
/// @cond
///
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
/// 1. Redistributions of source code must retain the above copyright notice,
/// this list of conditions and the following disclaimer.
/// 2. Redistributions in binary form must reproduce the above copyright notice,
/// this list of conditions and the following disclaimer in the documentation
/// and/or other materials provided with the distribution.
/// 3. Neither the name of the <organization> nor the
/// names of its contributors may be used to endorse or promote products
/// derived from this software without specific prior written permission.
///
////////////////////////////////////////////////////////////////////////////////
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
/// @endcond
////////////////////////////////////////////////////////////////////////////////

#include <regular_expressions.hpp>

#include <fstream>
#include <sstream>
#include <cstring>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

using namespace std;

/// Maximum length of the PCRE2 error string.
#define PCRE2_MAX_ERROR_LENGTH (1024)

class RegExpData
{
public:
    RegExpData();
    ~RegExpData();

    string pattern;
    string errorString;
    vector<string> capturedTexts;
    vector<int> capturedTextPositions;
    std::string::size_type matchedLength;

    /// Regular expression object (compiled pattern).
    pcre2_code *pExpression;
};

RegExpData::RegExpData() : matchedLength(-1), pExpression(NULL)
{
}

RegExpData::~RegExpData()
{
    // Free the pattern.
    if(NULL != pExpression)
    {
        pcre2_code_free(pExpression);
        pExpression = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

RegExp::RegExp() : d(new RegExpData)
{
}

RegExp::RegExp(const string& pattern) : d(new RegExpData)
{
    SetPattern(pattern);
}

RegExp::~RegExp()
{
    delete d;
}

string RegExp::GetPattern() const
{
    return d->pattern;
}

void RegExp::SetPattern(const string& pattern)
{
    // Error number if the pattern did not compile.
    int errorNumber;

    // Error offset if the pattern did not compile.
    PCRE2_SIZE errorOffset;

    // Set the pattern.
    d->pattern = pattern;

    // Free the previous pattern.
    if(NULL != d->pExpression)
    {
        pcre2_code_free(d->pExpression);
        d->pExpression = NULL;
    }

    // Compile the pattern.
    d->pExpression = pcre2_compile((PCRE2_SPTR)pattern.c_str(),
        PCRE2_ZERO_TERMINATED, 0, &errorNumber, &errorOffset, NULL);

    // Handle any compile error.
    if(NULL == d->pExpression)
    {
        stringstream errorStringStream;

        PCRE2_UCHAR buffer[PCRE2_MAX_ERROR_LENGTH];
        memset(buffer, 0, sizeof(buffer));
        pcre2_get_error_message(errorNumber, buffer, sizeof(buffer));

        errorStringStream << "PCRE2 compilation failed at offset "
            << errorOffset << ": " << buffer;
        d->errorString = errorStringStream.str();
    }
}

string RegExp::ErrorString() const
{
    return d->errorString;
}

string RegExp::CaptureText(int captureIndex)
{
    string capturedText;

    if((int)d->capturedTexts.size() > captureIndex)
    {
        capturedText = d->capturedTexts.at(captureIndex);
    }

    return capturedText;
}

int RegExp::CapturedTextPosition(int captureIndex)
{
    int position = -1;

    if((int)d->capturedTextPositions.size() > captureIndex)
    {
        position = d->capturedTextPositions.at(captureIndex);
    }

    return position;
}

int RegExp::CaptureCount() const
{
    return d->capturedTexts.size();
}

vector<string> RegExp::CapturedTexts() const
{
    return d->capturedTexts;
}

int RegExp::MatchedLength() const
{
    return d->matchedLength;
}

int RegExp::FindMatch(const string& str, int offset, bool wantLast)
{
    // The string to match.
    const char *szString = str.c_str();

    // Match data.
    pcre2_match_data *pMatchData;

    // Get the length of the subject string.
    ssize_t stringLength;

    // Match output vector.
    PCRE2_SIZE *pOvector;

    // Number of capture texts.
    uint32_t numCaptureTexts;

    // Current capture text.
    const char *szCapture;

    // Length of a capture text.
    size_t captureLength;

    // Clear the previous results.
    d->errorString.clear();
    d->capturedTexts.clear();
    d->capturedTextPositions.clear();
    d->matchedLength = -1;

    // Check for a valid pattern.
    if(NULL == d->pExpression)
    {
        d->errorString = "No valid pattern set.";
        return -1;
    }

    // Allocate the match data.
    pMatchData = pcre2_match_data_create_from_pattern(d->pExpression, NULL);

    // Check that the match data was allocated.
    if(NULL == pMatchData)
    {
        d->errorString = "Match data allocation error.";
        return -1;
    }

    // Get the length of the subject string.
    stringLength = (ssize_t)str.length();

    do
    {
        // Check for a match.
        if(0 <= pcre2_match(d->pExpression, (PCRE2_SPTR)szString,
            stringLength, 0, 0, pMatchData, NULL))
        {
            // Clear the previous results if we found a new match.
            if(wantLast)
            {
                d->capturedTexts.clear();
                d->capturedTextPositions.clear();
            }

            // Get the ovector.
            pOvector = pcre2_get_ovector_pointer(pMatchData);

            // Determine the number of capture texts.
            numCaptureTexts = pcre2_get_ovector_count(pMatchData);

            // Process each capture text.
            for(uint32_t i = 0; i < numCaptureTexts; i++)
            {
                // Extract the capture text.
                szCapture = szString + pOvector[2 * i];

                // Calculate the length of the capture text.
                captureLength = pOvector[2 * i + 1] -
                    pOvector[2 * i];

                // Save the captured text.
                d->capturedTexts.push_back(string(szCapture, captureLength));

                // Save the position of the captured text.
                d->capturedTextPositions.push_back(pOvector[2 * i]);
            }

            // Save the length of the match.
            d->matchedLength = pOvector[1] - pOvector[0];
        }
    }
    while(wantLast && 0 <= d->matchedLength);

    // Cleanup.
    pcre2_match_data_free(pMatchData);

    // The position of the match is the position of the first capture.
    return CapturedTextPosition(0);
}

int RegExp::IndexIn(const string& str, int offset)
{
    return FindMatch(str, offset, false);
}

int RegExp::LastIndexIn(const string& str, int offset)
{
    return FindMatch(str, offset, true);
}

bool RegExp::ExactMatch(const string& str)
{
    return 0 == IndexIn(str) && d->matchedLength == str.length();
}

bool RegExp::IsEmpty() const
{
    return d->pattern.empty();
}

bool RegExp::IsValid() const
{
    return NULL != d->pExpression;
}

bool RegExp::ProcessExactFileMatches(const string& pattern, const string& path,
    MatchCallback_t callback, void *pUserData)
{
    RegExp rx(pattern);
    ifstream file;
    bool matchOK = true;

    // Attempt to open the file.
    file.open(path.c_str());

    // Make sure the pattern, callback and file are valid first.
    if(rx.IsValid() && 0 != callback && file.good())
    {
        // Try to match each line.
        for(string line; getline(file, line);)
        {
            // If this line is a match, call the callback.
            if(rx.ExactMatch(line))
            {
                (*callback)(rx.d->capturedTexts,
                    rx.d->capturedTextPositions, pUserData);
            }
        }

        // Close the file now.
        file.close();
    }
    else
    {
        matchOK = false;
    }

    return matchOK;
}

bool RegExp::ProcessExactMatches(const string& pattern, const string& str,
    MatchCallback_t callback, void *pUserData)
{
    RegExp rx(pattern);
    stringstream ss(str);
    bool matchOK = true;

    // Make sure the pattern and callback are valid first.
    if(rx.IsValid() && 0 != callback)
    {
        // Try to match each line.
        for(string line; getline(ss, line);)
        {
            // If this line is a match, call the callback.
            if(rx.ExactMatch(line))
            {
                (*callback)(rx.d->capturedTexts,
                    rx.d->capturedTextPositions, pUserData);
            }
        }
    }
    else
    {
        matchOK = false;
    }

    return matchOK;
}
