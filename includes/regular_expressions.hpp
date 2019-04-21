////////////////////////////////////////////////////////////////////////////////
///
/// @file       includes/regular_expressions.hpp
///
/// @project    ipxact
///
/// @brief      regular expression support routines.
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

#ifndef REGULAR_EXPRESSIONS_H
#define REGULAR_EXPRESSIONS_H

#include <string>
#include <vector>

class RegExpData;

class RegExp
{
public:
    typedef void (*MatchCallback_t)(std::vector<std::string>& capturedTexts,
        std::vector<int>& capturedTextPositions, void *pUserData);

    RegExp();
    RegExp(const std::string& pattern);
    ~RegExp();

    /**
     * @brief Get the regular expression pattern used.
     * @returns The regular expression pattern.
     */
    std::string GetPattern() const;

    /**
     * @brief Set the regular expression pattern to use.
     * @param pattern The regular expression pattern.
     *
     * The pattern can be checked with @ref IsValid.
     */
    void SetPattern(const std::string& pattern);

    std::string ErrorString() const;
    std::string CaptureText(int captureIndex = 0);
    int CapturedTextPosition(int captureIndex = 0);
    int CaptureCount() const;
    std::vector<std::string> CapturedTexts() const;

    /**
     * @brief The length of the match or -1 if there was no match.
     * @returns The length of the match or -1 if there was no match.
     */
    int MatchedLength() const;

    int IndexIn(const std::string& str, int offset = 0);
    int LastIndexIn(const std::string& str, int offset = 0);
    bool ExactMatch(const std::string& str);

    /**
     * @brief Returns true if the pattern is empty.
     * @retval true The pattern is empty.
     * @retval true The pattern is not empty.
     */
    bool IsEmpty() const;

    /**
     * @brief Returns true if the pattern is valid.
     * @retval true The pattern is valid.
     * @retval false The pattern is not valid.
     */
    bool IsValid() const;

    static bool ProcessExactFileMatches(const std::string& pattern,
        const std::string& path, MatchCallback_t callback, void *pUserData);
    static bool ProcessExactMatches(const std::string& pattern,
        const std::string& str, MatchCallback_t callback, void *pUserData);

private:
    int FindMatch(const std::string& str, int offset, bool wantLast);

    RegExpData *d;
};

#endif // REGULAR_EXPRESSIONS_H
