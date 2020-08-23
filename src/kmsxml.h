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

#ifndef Killing_Me_Softly_aXing_Me_Lightly_HEADER_GUARD_
#define Killing_Me_Softly_aXing_Me_Lightly_HEADER_GUARD_

#include <iostream>
#include <string>
#include <list>
#include <regex>

namespace kmsxml {

  namespace structure {

    // weak pointer with auto locking
    template <typename T>
    class auto_weak_ptr {
    public:
      template <typename U>
      auto_weak_ptr(std::weak_ptr<U> const &wp) : wp(wp) { }
      template <typename U>
      auto_weak_ptr(std::shared_ptr<U> const &sp) : wp(sp) { }

      explicit operator bool() const { return not wp.expired(); }
      auto operator*() const { return *wp.lock(); }
      auto operator->() const { return wp.lock(); }
      void reset() { wp.reset(); }
      template <typename U>
      auto_weak_ptr<U> cast() const
        { return std::dynamic_pointer_cast<U>(wp.lock()); }
      auto_weak_ptr<T const> to_const() const { return *this; }
    private:
      std::weak_ptr<T> wp;
      template <typename U, typename V>
      friend bool operator==(auto_weak_ptr<U> const &,
                             auto_weak_ptr<V> const &);
      template <typename U>
      friend class auto_weak_ptr;
    };

    template <typename T, typename U>
    bool operator==(auto_weak_ptr<T> const &a, auto_weak_ptr<U> const &b)
      { return a.wp.lock()==b.wp.lock(); }

    // the "appending_...()" methods return "this" as a "chaining_ptr_t<>", so
    // that calls can be chained
    template <typename T>
    class chaining_ptr_t {
    public:
      chaining_ptr_t(T *ptr) : ptr(ptr) { }
      auto operator->() const { return ptr; }
    private:
      T *const ptr;
    };

  }

  using text_t=std::string;
  using name_t=std::string;

  namespace implementation {

    inline std::list<std::pair<std::string, std::string>> const
    entity_references={
      {"&",  "&amp;" }, // 1st, lest it replace "&" from the other replacements
      {"<",  "&lt;"  },
      {">",  "&gt;"  },
      {"'",  "&apos;"},
      {"\"", "&quot;"},
      {"\n", "&#xa;"} // to preserve newlines
    };
    inline std::string const additional_indent="  ";

    // encode entity references in a text
    inline text_t xml_encode_text(text_t t) {
      for (auto const &er: entity_references)
        t=std::regex_replace(t, std::regex(er.first), er.second);
      return t;
    }

  }

  /*
                      .------------.  *           .-------------.  *
                      | content_t  |<--------.    | attribute_t |<---.
                      |============|         |    |-------------|    |
                      | /output()/ |         |    | name        |    |
                      '-----,------'         |    | value       |    |
                           /_\               |    |-------------|    |
                            |                |    | output()    |    |
                            |                |    '-------------'    |
                            |                |                       |
                .-----------+---------.      '-------------.         |
                |                     |                    |         |
        .----------------.     .--------------------.    list      list
        | text_content_t |     | element_t          |      |         |
        |----------------|     |====================|      |         |
        | text           |     | output()           |<x>---'         |
        |----------------|     | static make_root() |    contents    |
        | output()       |     | append_attribute() |                |
        '----------------'     | add_element()      |<x>-------------'
                               | append_text()      |    attributes
                               '--------------------'

     the root of the tree is an element; an element contains a list of any
     number of attributes, and a list of any number of content-holding objects;
     content-holding objects can be text contents or elements

     instances of these classes can be created exclusively through the
     interface of "element_t": static "make_root()" for the root, then
     "append_attribute()", "add_element()", and "append_text()" on an
     "element_t" instance to add new children to it
  */

  class attribute_t {
  public:
    attribute_t(name_t name, text_t value) : name(name), value(value) { }
    // an attribute is output in the holding element's opening tag structure
    //
    //     {name}="{value}"
    void output(std::ostream &os) const {
      os << name << "=\"" << implementation::xml_encode_text(value) << "\"";
    }
  private:
    name_t const name;
    text_t const value;
  };

  class content_t {
  public:
    virtual ~content_t()=default;
    // output to output stream "os", with a carried indent of "indent"; if
    // "nl", output in a separate line (only applicable to text content, when
    // an element only has a child, and it is a text content)
    virtual void output(std::ostream &os, text_t indent, bool nl) const=0;
  };

  class text_content_t
    : public content_t {
  public:
    text_content_t(text_t text) : text(text) { }
    // text content is output in the holding element's body; if "nl", output in
    // a separate line
    void output(std::ostream &os, text_t indent, bool nl) const override {
      os << (nl ? indent : "") << implementation::xml_encode_text(text)
         << (nl ? "\n" : "");
    }
  private:
    text_t const text;
  };

  class element_t
    : public content_t {
  public:
    using root_t=std::shared_ptr<element_t>;
    template <typename OT=element_t>
    using node_t=structure::auto_weak_ptr<OT>;

    template <typename... A>
    static root_t make_root(A &&...a)
      { return std::shared_ptr<element_t>(new element_t(a...)); }

    template <typename... A>
    structure::chaining_ptr_t<element_t> append_attribute(A &&...a) {
      attributes.push_back(std::make_shared<attribute_t>(a...));
      return this;
    }
    template <typename... A>
    node_t<> add_element(A &&...a) {
      auto new_element=std::shared_ptr<element_t>(new element_t(a...));
      contents.push_back(new_element);
      return new_element;
    }

    structure::chaining_ptr_t<element_t> append_text(text_t text) {
      contents.push_back(
        std::shared_ptr<text_content_t>(new text_content_t(text)));
      return this;
    }

    //     <{name} {attributes...}> {children...} </{name}>
    //
    // or, when childless,
    //
    //     <{name}/>
    void output(std::ostream &os, text_t indent, bool) const override {
      os << indent << "<" << name;
      for (auto const &a: attributes)
        a->output(os << " ");
      if (contents.empty())
        os << "/>\n";
      else {
        bool single_text_content=
          contents.size()==1
          and std::dynamic_pointer_cast<text_content_t>(
                contents.front());
        bool nl_after_content=not single_text_content;
        text_t new_indent=indent+implementation::additional_indent;
        os << ">" << (nl_after_content ? "\n" : "");
        for (auto const &c: contents)
          c->output(os, new_indent, nl_after_content);
        os << (nl_after_content ? indent : "") << "</" << name << ">\n";
      }
    }
  private:
    element_t(name_t name) : name(name) { }
    name_t const name;
    std::list<std::shared_ptr<attribute_t>> attributes;
    std::list<std::shared_ptr<content_t>> contents;
  };

  std::ostream &operator<<(std::ostream &os, element_t const &e) {
    e.output(os, "", true);
    return os;
  }

}

#endif
