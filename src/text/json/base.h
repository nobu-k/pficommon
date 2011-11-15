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

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <map>

#include "../../lang/shared_ptr.h"
#include "../../data/string/ustring.h"

#include <msgpack.hpp>

namespace pfi{
namespace text{
namespace json{

class json_value;

class json{
public:
  typedef std::map<std::string, json>::iterator iterator;
  typedef std::map<std::string, json>::const_iterator const_iterator;

  json();
  json(json_value* p): val(p) {}

  json &operator[](const std::string &name);
  json &operator[](size_t ix);

  const json &operator[](const std::string &name) const;
  const json &operator[](size_t ix) const;

  size_t count(const std::string &name) const;

  void add(const std::string &name, const json &v);
  void add(const json &v);

  size_t size() const;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  json_value *get() const { return val.get(); }

  void print(std::ostream &os, bool escape) const;
  void pretty(std::ostream &os, bool escape) const;

  static const bool is_read = false;

  template<typename Packer>
  void msgpack_pack(Packer &pk) const;
  void msgpack_unpack(const msgpack::object &o) {
    val.reset(inner_unpack(o));
  }

private:
  template<typename T, typename Packer>
  bool inner_pack(Packer &pk) const;
  json_value *inner_unpack(const msgpack::object &o);

private:
  pfi::lang::shared_ptr<json_value> val;
};

class json_value{
public:
  virtual ~json_value(){}

  virtual void print(std::ostream &os, bool escape) const = 0;
  virtual void pretty(std::ostream &os, int level, bool escape) const {
    print(os, escape);
  }
};

class json_array : public json_value{
public:
  json_array(){}

  size_t size() const{
    return dat.size();
  }

  json &operator[](size_t ix){
    return dat[ix];
  }

  const json &operator[](size_t ix) const {
    return dat[ix];
  }

  void add(const json &j){
    dat.push_back(j);
  }

  void print(std::ostream &os, bool escape) const {
    bool fst=true;
    os<<'[';
    for (size_t i=0; i<dat.size(); i++){
      if (fst) fst=false;
      else os<<',';
      dat[i].print(os, escape);
    }
    os<<']';
  }

  void pretty(std::ostream &os, int level, bool escape) const {
    bool fst=true;
    os<<'[';
    for (size_t i=0; i<dat.size(); i++){
      if (fst) fst=false, os<<std::endl;
      else os<<','<<std::endl;
      for (int j=0; j<(level+1)*2; j++) os<<' ';
      dat[i].get()->pretty(os, level+1, escape);
    }
    os<<std::endl;
    for (int i=0; i<level*2; i++) os<<' ';
    os<<']';
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack_array(dat.size());
    for (size_t i=0; i<dat.size(); i++){
      dat[i].msgpack_pack(pk);
    }
  }

private:
  std::vector<json> dat;
};

class json_number : public json_value{
};

class json_integer : public json_value{
public:
  json_integer(int64_t n) : dat(n){}

  int64_t get() const { return dat; }

  void print(std::ostream &os, bool escape) const {
    os<<dat;
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack(dat);
  }

private:
  int64_t dat;
};

class json_float : public json_value{
public:
  json_float(double d) : dat(d){}

  double get() const { return dat; }

  void print(std::ostream &os, bool escape) const {
    std::streamsize prec=os.precision();
    os<<std::setprecision(12)
      <<dat
      <<std::setprecision(prec);
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack(dat);
  }

private:
  double dat;
};

class json_string : public json_value{
public:
  json_string(const std::string &s): dat(s) {}

  const std::string &get() const { return dat; }

  void print(std::ostream &os, bool escape) const {
    print(os, dat, escape);
  }

  static void print(std::ostream &os, const std::string &dat, bool escape){
    os<<'\"';
    if (escape){
      const char *p=dat.c_str();
      while(*p){
	pfi::data::string::uchar uc=pfi::data::string::chars_to_uchar(p);
	print_char(os, uc);
      }
    }
    else{
      const char *p=dat.c_str();
      while(*p){
	print_char_without_escape(os, *p++);
      }
    }
    os<<'\"';
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack_raw(dat.size());
    pk.pack_raw_body(dat.c_str(), dat.size());
  }

private:

  static void print_char(std::ostream &os, pfi::data::string::uchar u){
    static const char escs[]={
      0x22, 0x5C, 0x2F, 0x08, 0x0C, 0x0A, 0x0D, 0x09
    };
    static const char cnvs[]={
      '\"', '\\', '/', 'b', 'f', 'n', 'r', 't'
    };

    static const int cnt=sizeof(escs)/sizeof(escs[0]);

    if (u<=0x7F){
      char c=static_cast<char>(u);
      for (int i=0; i<cnt; i++){
	if (escs[i]==c){
	  os<<'\\'<<cnvs[i];
	  return;
	}
      }
      if (iscntrl(c)){
        os<<"\\u00"<<tohex((c>>4)&0xf)<<tohex(c&0xf);
        return;
      }
      os<<c;
    }
    else{
      os<<"\\u"
	<<tohex((u>>12)&0xf)
	<<tohex((u>>8)&0xf)
	<<tohex((u>>4)&0xf)
	<<tohex((u>>0)&0xf);
    }
  }

  static void print_char_without_escape(std::ostream &os, char c){
    static const char escs[]={
      0x22, 0x5C, 0x2F, 0x08, 0x0C, 0x0A, 0x0D, 0x09
    };
    static const char cnvs[]={
      '\"', '\\', '/', 'b', 'f', 'n', 'r', 't'
    };

    static const int cnt=sizeof(escs)/sizeof(escs[0]);

    for (int i=0; i<cnt; i++){
      if (escs[i]==c){
	os<<'\\'<<cnvs[i];
	return;
      }
    }
    if (iscntrl(c)){
      os<<"\\u00"<<tohex((c>>4)&0xf)<<tohex(c&0xf);
      return;
    }
    os<<c;
  }

  static char tohex(int c){
    if (c<=9) return '0'+c;
    return 'A'+c-10;
  }

  std::string dat;
};

class json_object : public json_value{
public:
  typedef std::map<std::string, json>::iterator iterator;
  typedef std::map<std::string, json>::const_iterator const_iterator;
  
  json_object(){}

  void add(const std::string &name, const json &j){
    member[name]=j;
  }

  json &operator[](const std::string &name){
    return member[name];
  }

  const json &operator[](const std::string &name) const {
    std::map<std::string, json>::const_iterator p=member.find(name);
    if (p==member.end())
      throw std::out_of_range("json_object::operator[]");
    return p->second;
  }

  size_t count(const std::string &name) const {
    return member.count(name);
  }

  iterator begin(){ return member.begin(); }
  const_iterator begin() const { return member.begin(); }

  iterator end(){ return member.end(); }
  const_iterator end() const { return member.end(); }

  void print(std::ostream &os, bool escape) const {
    bool fst=true;
    os<<'{';
    for (std::map<std::string, json>::const_iterator it=member.begin();
	 it!=member.end(); it++){
      if (fst) fst=false;
      else os<<',';
      json_string::print(os, it->first, escape);
      os<<':';
      it->second.print(os, escape);
    }
    os<<'}';
  }

  void pretty(std::ostream &os, int level, bool escape) const {
    bool fst=true;
    os<<'{';
    for (std::map<std::string, json>::const_iterator it=member.begin();
	 it!=member.end(); it++){
      if (fst) fst=false, os<<std::endl;
      else os<<','<<std::endl;
      for (int i=0; i<(level+1)*2; i++) os<<' ';
      json_string::print(os, it->first, escape);
      os<<": ";
      it->second.get()->pretty(os, level+1, escape);
    }
    os<<std::endl;
    for (int i=0; i<level*2; i++) os<<' ';
    os<<'}';
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack_map(member.size());
    for (const_iterator it = member.begin(), end = member.end(); it != end; ++it){
      pk.pack_raw(it->first.size());
      pk.pack_raw_body(it->first.c_str(), it->first.size());
      it->second.msgpack_pack(pk);
    }
  }

private:
  std::map<std::string, json> member;
};

class json_bool : public json_value{
public:
  json_bool(bool b): dat(b) {}

  bool get() const { return dat; }

  void print(std::ostream &os, bool escape) const {
    os<<(dat?"true":"false");
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack(dat);
  }

private:
  bool dat;
};

class json_null : public json_value{
public:
  json_null(){}

  void print(std::ostream &os, bool escape) const {
    os<<"null";
  }

  template<typename Packer>
  void msgpack_pack(Packer &pk) const {
    pk.pack_nil();
  }
};

template <class T>
inline bool is(const json &j)
{
  return dynamic_cast<const T*>(j.get())!=NULL;
}

inline json::json(): val(new json_null())
{
}

inline json &json::operator[](const std::string &name)
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  return p[name];
}

inline json &json::operator[](size_t ix)
{
  json_array &p=dynamic_cast<json_array&>(*val.get());
  return p[ix];
}

inline const json &json::operator[](const std::string &name) const
{
  const json_object &p=dynamic_cast<const json_object&>(*val.get());
  return p[name];
}

inline const json &json::operator[](size_t ix) const
{
  const json_array &p=dynamic_cast<const json_array&>(*val.get());
  return p[ix];
}

inline size_t json::count(const std::string &name) const
{
  const json_object &p=dynamic_cast<const json_object&>(*val.get());
  return p.count(name);
}

inline void json::add(const std::string &name, const json &v)
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  p[name]=v;
}

inline void json::add(const json &v)
{
  json_array &p=dynamic_cast<json_array&>(*val.get());
  p.add(v);
}

inline size_t json::size() const
{
  json_array &p=dynamic_cast<json_array&>(*val.get());
  return p.size();
}

inline json::iterator json::begin()
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  return p.begin();
}

inline json::const_iterator json::begin() const
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  return p.begin();
}

inline json::iterator json::end()
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  return p.end();
}

inline json::const_iterator json::end() const
{
  json_object &p=dynamic_cast<json_object&>(*val.get());
  return p.end();
}

inline void json::print(std::ostream &os, bool escape) const 
{
  val->print(os, escape);
}

inline void json::pretty(std::ostream &os, bool escape) const
{
  val->pretty(os, 0, escape);
  os<<std::endl;
}

template <class T>
class pretty_tag{
public:
  pretty_tag(const T &dat): dat(dat) {}
  const T &dat;
};

template <class T>
pretty_tag<T> pretty(const T &v)
{
  return pretty_tag<T>(v);
}

template <class T>
class without_escape_tag{
public:
  without_escape_tag(const T &dat): dat(dat) {}
  const T &dat;
};

template <class T>
without_escape_tag<T> without_escape(const T &v)
{
  return without_escape_tag<T>(v);
}

template <class T>
inline void gen_print(std::ostream &os, const T &js, bool pretty, bool escape)
{
  if (pretty)
    js.pretty(os, escape);
  else
    js.print(os, escape);
}

template <class T>
inline void gen_print(std::ostream &os, const pretty_tag<T> &js, bool pretty, bool escape)
{
  gen_print(os, js.dat, true, escape);
}


template <class T>
inline void gen_print(std::ostream &os, const without_escape_tag<T> &js, bool pretty, bool escape)
{
  gen_print(os, js.dat, pretty, false);
}

inline std::ostream &operator<<(std::ostream &os, const json &j)
{
  gen_print(os, j, false, true);
  return os;
}

template <class T>
inline std::ostream &operator<<(std::ostream &os, const pretty_tag<T> &j)
{
  gen_print(os, j, false, true);
  return os;
}

template <class T>
inline std::ostream &operator<<(std::ostream &os, const without_escape_tag<T> &j)
{
  gen_print(os, j, false, true);
  return os;
}

template<typename Packer>
void json::msgpack_pack(Packer &pk) const {
  inner_pack<json_bool>(pk) ||
  inner_pack<json_integer>(pk) ||
  inner_pack<json_float>(pk) ||
  inner_pack<json_array>(pk) ||
  inner_pack<json_string>(pk) ||
  inner_pack<json_object>(pk) ||
  inner_pack<json_null>(pk);
}

template<typename T, typename Packer>
inline bool json::inner_pack(Packer &pk) const {
  if (!is<T>(*this)) return false;
  static_cast<const T*>(get())->msgpack_pack(pk);
  return true;
}

inline json_value *json::inner_unpack(const msgpack::object &o) {
  switch (o.type){
  case msgpack::type::NIL: return new json_null();
  case msgpack::type::BOOLEAN: return new json_bool(o.via.boolean);
  case msgpack::type::POSITIVE_INTEGER: return new json_integer(o.via.i64);
  case msgpack::type::NEGATIVE_INTEGER: return new json_integer(o.via.u64);
  case msgpack::type::DOUBLE: return new json_float(o.via.dec);
  case msgpack::type::RAW: return new json_string(std::string(o.via.raw.ptr, o.via.raw.size));
  case msgpack::type::ARRAY: {
    json_array *array=new json_array();
    try{
      for (size_t i=0; i<o.via.array.size; i++){
        array->add(json(inner_unpack(o.via.array.ptr[i])));
      }
      return array;
    }
    catch(...){
      delete array;
      throw;
    }
  }
  case msgpack::type::MAP: {
    json_object *obj=new json_object();
    try{
      for (size_t i=0; i<o.via.map.size; i++){
        const msgpack::object_kv &kv=o.via.map.ptr[i];
        if (kv.key.type!=msgpack::type::RAW) continue; // FIXME: currently, key must be a string value
        obj->add(std::string(kv.key.via.raw.ptr, kv.key.via.raw.size),
                 json(inner_unpack(kv.val)));
      }
      return obj;
    }
    catch(...){
      delete obj;
      throw;
    }
  }
  default:
    return new json_null(); // FIXME: should throw an exception?
  }
}

} // json
} // text
} // pfi
