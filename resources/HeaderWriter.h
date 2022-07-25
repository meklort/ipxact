////////////////////////////////////////////////////////////////////////////////
///
/// @file       <FILE>
///
/// @project    <PROJECT>
///
/// @brief      <DESCRIPTION>
///
////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
///
/// @copyright Copyright (c) <YEAR>, Evan Lojewski
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

/** @defgroup <GUARD>    <DESCRIPTION> */
/** @addtogroup <GUARD>
 * @{
 */
#ifndef <GUARD>
#define <GUARD>

#include <types.h>
<INCLUDES>
#ifdef CXX_SIMULATOR /* Compiling c++ simulator code - uses register wrappers */
void init_<INIT_FUNCTION>_sim(void* base, uint32_t (*read)(uint32_t val, uint32_t offset, void *args), uint32_t (*write)(uint32_t val, uint32_t offset, void *args));
void init_<INIT_FUNCTION>(void);

#include <CXXRegister.h>
typedef CXXRegister<uint8_t,  0,  8> <GUARD>_uint8_t;
typedef CXXRegister<uint16_t, 0, 16> <GUARD>_uint16_t;
typedef CXXRegister<uint32_t, 0, 32> <GUARD>_uint32_t;
#define <GUARD>_uint8_t_bitfield(__pos__, __width__)  CXXRegister<uint8_t,  __pos__, __width__>
#define <GUARD>_uint16_t_bitfield(__pos__, __width__) CXXRegister<uint16_t, __pos__, __width__>
#define <GUARD>_uint32_t_bitfield(__pos__, __width__) CXXRegister<uint32_t, __pos__, __width__>
#define register_container struct
#define <VOLATILE>
#define BITFIELD_BEGIN(__type__, __name__) struct {
#define BITFIELD_MEMBER(__type__, __name__, __offset__, __bits__) __type__##_bitfield(__offset__, __bits__) __name__;
#define BITFIELD_END(__type__, __name__) } __name__;

#else /* Firmware Data types */
typedef uint8_t  <GUARD>_uint8_t;
typedef uint16_t <GUARD>_uint16_t;
typedef uint32_t <GUARD>_uint32_t;
#define register_container union
#define <VOLATILE> volatile
#define BITFIELD_BEGIN(__type__, __name__) struct {
#define BITFIELD_MEMBER(__type__, __name__, __offset__, __bits__) __type__ __name__:__bits__;
#define BITFIELD_END(__type__, __name__) } __name__;
#endif /* !CXX_SIMULATOR */

<SERIALIZED>

#undef register_container
#undef BITFIELD_BEGIN
#undef BITFIELD_MEMBER
#undef BITFIELD_END

#ifndef CXX_SIMULATOR
_Static_assert(sizeof(<COMPONENT_TYPE>_t) == <COMPONENT_SIZE>, "sizeof(<COMPONENT_TYPE>_t) must be <COMPONENT_SIZE>");
#endif

#endif /* !<GUARD> */

/** @} */
