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

#include <gtest/gtest.h>

#include "./mmapper.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace pfi::system::mmapper;

TEST(mmapper_test, open_close_empty)
{
  int r;
  {
    ofstream ofs("test.file", std::ios::out | std::ios::trunc | std::ios::binary);
  }
  mmapper m;
  r = m.open("test.file");
  EXPECT_EQ(r, -1);
  unlink("test.file");
}

TEST(mmapper_test, open_close) {
  {
    FILE *f;
    f = fopen("test.txt", "w+");
    EXPECT_TRUE(f != NULL);
    size_t s = fwrite("12345678", 1, 8, f);
    EXPECT_EQ(s, 8);
    fclose(f);
  }
  {
    int r;
    mmapper m;
    r = m.open("test.txt");
    EXPECT_EQ(r, 0);
    EXPECT_EQ(m.size(), 8);
    for (int i = 0; i < m.size(); i++)
      EXPECT_EQ(m[i], '1' + i);
    EXPECT_EQ(m.end() - m.begin(), 8);
    r = m.close();
    EXPECT_EQ(r, 0);
  }
  {
    unlink("test.txt");
  }
}

TEST(mmapper_test, file_not_found) {
  int r;
  mmapper m;
  r = m.open("file_not_found.txt");
  EXPECT_EQ(r, -1);
}
