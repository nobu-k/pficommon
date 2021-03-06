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

#include "base.h"

#include <vector>

namespace pfi{
namespace data{
namespace serialization{

template <class Archive, class T, class Allocator>
void serialize(Archive &ar, std::vector<T, Allocator> &v)
{
  uint32_t size=static_cast<uint32_t>(v.size());
  ar & size;

  v.resize(size);
  for (uint32_t i=0;i<size;i++)
    ar & v[i];
}

class array_type : public type_rep {
public:
  array_type(pfi::lang::shared_ptr<type_rep> type): type(type){}

  void traverse(pfi::lang::function<void(type_rep*)> f){
    f(this);
    type->traverse(f);
  }

  void print(std::ostream &os){
    os<<"array<";
    type->print(os);
    os<<">";
  }

  pfi::lang::shared_ptr<type_rep> elem_type(){ return type; }

private:
  pfi::lang::shared_ptr<type_rep> type;
};

template <class T, class Allocator>
void serialize(reflection &ref, std::vector<T, Allocator> &)
{
  ref.add("", pfi::lang::shared_ptr<type_rep>(new array_type(get_type<T>())));
}

} // serializatin
} // data
} // pfi
