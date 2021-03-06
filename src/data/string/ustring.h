// Copyright (c)2008-2011, Preferred Infrastructure Inc.
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Preferred Infrastructure nor the names of other
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <string>

namespace pfi {
namespace data {
namespace string {

  typedef unsigned int uchar;

  class ustring : public std::basic_string<uchar> {
  public:
    ustring(){}
    ustring(const ustring &s, size_type pos = 0, size_type n = npos)
      : std::basic_string<uchar>(s, pos, npos) {}
    ustring(const uchar *p)
      : std::basic_string<uchar>(p) {}
    ustring(const uchar *p, size_type n)
      : std::basic_string<uchar>(p, n) {}
    ustring(size_type n, uchar c)
      : std::basic_string<uchar>(n, c) {}

    template <class InputIterator>
    ustring(InputIterator first, InputIterator last)
      : std::basic_string<uchar>(first, last) {}

    ustring(const std::basic_string<uchar> &str)
      : std::basic_string<uchar>(str) {}

    operator std::basic_string<uchar>() const {
      return std::basic_string<uchar>(*this);
    }
  };
  
  // string <-> ustring conversion
  ustring string_to_ustring(const char* p);
  ustring string_to_ustring(const std::string& s);
  std::string ustring_to_string(const ustring& us);

  // get first character of string
  uchar string_to_uchar(const char* p);
  uchar string_to_uchar(const std::string& s);
  std::string uchar_to_string(uchar uc);

  // char[] -> uchar conversion
  template <class InputIterator>
  uchar chars_to_uchar(InputIterator &in){
    unsigned int cur = 0;
    bool start = true;
    while(*in) {
      if((*in&0xC0) != 0x80){
	if(!start){
	  return cur;
	}else{
	  start = false;
	}
	if((*in&0xFC) == 0xFC){
	  cur = *in & 0x01;
	}else if((*in&0xF8) == 0xF8){
	  cur = *in & 0x03;
	}else if((*in&0xF0) == 0xF0){
	  cur = *in & 0x07;
	}else if((*in&0xE0) == 0xE0){
	  cur = *in & 0x0F;
	}else if((*in&0xC0) == 0xC0){
	  cur = *in & 0x1F;
	}else{
	  cur = *in;
	}
      }else{
	cur = cur * 64 + (*in&0x3F);
      }
      ++in;
    }
    if (!start) return cur;
    return 0;
  }

  // uchar -> char[] conversion
  template <class OutputIterator>
  void uchar_to_chars(uchar c, OutputIterator &out){
    char b;
    if(0 <= c && c <= 0x007f){
      b = c;
      *out++ = b;
    }else if(0x0080 <= c && c <= 0x07ff){
      b = c >> 6;
      *out++ = (b | 0xc0);
      b = c & 0x3f;
      *out++ = b | 0x80;
    }else if(0x0800 <= c && c <= 0xffff){
      b = c >> 12;
      *out++ = (b | 0xe0);
      b = (c >> 6) & 0x3f;
      *out++ = b | 0x80;
      b = c & 0x3f;
      *out++ = b | 0x80;
    }else if(0x10000 <= c && c <= 0x10ffff){
      b = c >> 18;
      *out++ = (b | 0xf0);
      b = (c >> 12) & 0x3f;
      *out++ = b | 0x80;
      b = (c >> 6) & 0x3f;
      *out++ = b | 0x80;
      b = c & 0x3f;
      *out++ = b | 0x80;
    }
  }

  // utility
  bool is_basic_latin(uchar uc);
  bool is_alnum(uchar uc);
  bool is_hiragana(uchar uc);
  bool is_katakana(uchar uc);
  bool is_kanji(uchar uc);
  bool is_hankaku(uchar uc);

  std::string sanitize_utf8(const std::string& s);
  ustring hankaku_to_zenkaku(const ustring& us);
  ustring zenkaku_latin_to_basic_latin(const ustring& us);
  ustring basic_latin_to_zenkaku_latin(const ustring& us);
  uchar zenkaku_latin_to_basic_latin(uchar uc);
  uchar basic_latin_to_zenkaku_latin(uchar uc);

  std::ostream& operator<<(std::ostream& out, const ustring &str);
  std::istream& operator>>(std::istream& in , ustring &str);

} // string
} // data
} // pfi
