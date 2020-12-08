// Copyright © 2020 Miguel González Cuadrado <mgcuadrado@gmail.com>

// This file is part of Testudo.

//     Testudo is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     Testudo is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with Testudo.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MGCUADRADO_CRC_HEADER_
#define MGCUADRADO_CRC_HEADER_

#include <string>
#include <cstdint>

namespace cyclic_redundancy_check {

  namespace implementation {

    template <typename crc_t>
    struct byte_table_t {
      crc_t data[0x100];
    };

  }

  /// crc32: CRC-32 (en.wikipedia.org/wiki/Cyclic_redundancy_check)

  using crc32_t=std::uint32_t;

  crc32_t crc32(std::string); // CRC-32 of a string

  implementation::byte_table_t<crc32_t>
  crc32_table(); // exposed only for debugging and validation

  /// crc64: CRC-64-ECMA (en.wikipedia.org/wiki/Cyclic_redundancy_check)

  using crc64_t=std::uint64_t;

  crc64_t crc64(std::string); // CRC-64-ECMA of a string

  implementation::byte_table_t<crc64_t>
  crc64_table(); // exposed only for debugging and validation

  /// crc32 and crc64

  // format:
  //
  //     * for "crc32_t", "01234567"
  //
  //     * for "crc64_t", "0123456789abcdef"

  template <typename crc_t>
  std::string to_hex(crc_t i);
  template <typename crc_t>
  crc_t hex_to(std::string s);

  /// implementation

  namespace implementation {

    template <typename crc_t>
    byte_table_t<crc_t> crc_table_n(crc_t reversed) {
      byte_table_t<crc_t> table;
      for (int i=0; i<0x100; ++i) {
        crc_t v=i;
        for (int k=0; k<8 ; ++k)
          v=(v >> 1) xor ((v bitand 0x1) * reversed);
        table.data[i]=v;
      }
      return table;
    }

    template <typename crc_t, crc_t reversed>
    crc_t crc_n(std::string s) {
      static auto table=crc_table_n<crc_t>(reversed);
      crc_t crc=compl 0;
      for (unsigned char c: s)
        crc=(crc >> 8) xor table.data[(crc bitand 0xff) xor c];
      return compl crc;
    }

    template <typename>
    struct total_digits;
    template <>
    struct total_digits<crc32_t> { static constexpr int value=8; };
    template <>
    struct total_digits<crc64_t> { static constexpr int value=16; };

  }

  template <typename crc_t>
  std::string to_hex(crc_t i) {
    static char const *digits="0123456789abcdef";
    int const total_digits=implementation::total_digits<crc_t>::value;
    char result[total_digits];
    for (int j=total_digits-1; j>=0; --j) {
      result[j]=digits[i bitand 0xf];
      i >>= 4;
    }
    return std::string(result, total_digits);
  }

  template <typename crc_t>
  crc_t hex_to(std::string s) {
    crc_t result=0;
    for (char c: s)
      result=result*16+((c<='9') ? (c-'0') : ((c-'a')+10));
    return result;
  }

  // CRC-32
  crc32_t constexpr reversed_crc32=0xedb88320;
  inline crc32_t crc32(std::string s)
    { return implementation::crc_n<crc32_t, reversed_crc32>(s); }
  inline implementation::byte_table_t<crc32_t> crc32_table()
    { return implementation::crc_table_n(reversed_crc32); }

  // CRC-64-ECMA
  crc64_t constexpr reversed_crc64=0xc96c5795d7870f42;
  inline crc64_t crc64(std::string s)
    { return implementation::crc_n<crc64_t, reversed_crc64>(s); }
  inline implementation::byte_table_t<crc64_t> crc64_table()
    { return implementation::crc_table_n(reversed_crc64); }

}

#endif
