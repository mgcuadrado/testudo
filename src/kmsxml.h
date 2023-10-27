// Copyright © 2020-2023 Miguel González Cuadrado <mgcuadrado@gmail.com>

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

// this header file implements a seriously impaired subset of XML for writing
// and reading; it's just enough for Testudo

#include "auto_weak_ptr.h"
#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <map>
#include <functional>

#ifdef KMSXML_TEST
namespace kmsxml_test
#else
namespace kmsxml
#endif
{

  namespace structure {

    // the "append_...()" methods return "shared_from_this()" as a
    // "chaining_ptr_t<>", so that calls can be chained
    template <typename T>
    class chaining_ptr_t {
    public:
      chaining_ptr_t(std::shared_ptr<T> ptr) : ptr(ptr) { }
      auto operator->() const { return ptr; }
      // operator std::shared_ptr<T>() const { return ptr; }
      // operator auto_weak_ptr<T>() const { return ptr; }
    private:
      std::shared_ptr<T> const ptr;
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
    constexpr size_t indent_width=2;
    inline text_t indent_text(size_t indent)
      { return text_t(indent*indent_width, ' '); }
    inline text_t tree_indent_text(size_t indent) {
      text_t tree_indent;
      for (size_t i=0; i<indent; ++i)
        tree_indent+="| ";
      return tree_indent;
    }

    // encode entity references in a text
    inline text_t xml_encode_text(text_t t) {
      for (auto const &[c, x]: entity_references)
        t=std::regex_replace(t, std::regex(c), x);
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
    name_t const name;
    text_t const value;
  };

  class content_t {
  public:
    using root_t=std::shared_ptr<content_t>;
    using root_const_t=std::shared_ptr<content_t const>;
    using node_t=structure::auto_weak_ptr<content_t>;
    using node_const_t=structure::auto_weak_ptr<content_t const>;
    virtual ~content_t()=default;
    // output to output stream "os", with a carried indent of "indent"; if
    // "nl", output in a separate line (only applicable to text content, when
    // an element only has a child, and it is a text content)
    virtual void output(std::ostream &os, size_t indent, bool nl) const=0;
    std::string print_tree() const {
      std::ostringstream oss;
      print_tree(oss);
      return oss.str();
    }
    void print_tree(std::ostream &os) const { print_tree(os, 0); }
    virtual void print_tree(std::ostream &os, size_t indent) const=0;
  };

  class text_content_t
    : public content_t {
  public:
    text_content_t(text_t text) : text(text) { }
    // text content is output in the holding element's body; if "nl", output in
    // a separate line
    void output(std::ostream &os, size_t indent, bool nl) const override {
      os << (nl ? implementation::indent_text(indent) : "")
         << implementation::xml_encode_text(text)
         << (nl ? "\n" : "");
    }
    using content_t::print_tree;
    void print_tree(std::ostream &os, size_t indent) const override {
      os << implementation::tree_indent_text(indent)
         << "\"" << text << "\"" << std::endl;
    }
    text_t const text;
  };

  class element_t
    : public content_t, public std::enable_shared_from_this<element_t> {
  public:
    using root_t=std::shared_ptr<element_t>;
    using root_const_t=std::shared_ptr<element_t const>;
    using node_t=structure::auto_weak_ptr<element_t>;
    using node_const_t=structure::auto_weak_ptr<element_t const>;

    template <typename... A>
    static root_t make_root(A &&...a) {
      return std::shared_ptr<element_t>(new element_t(std::forward<A>(a)...));
    }

    template <typename... A>
    structure::chaining_ptr_t<element_t> append_attribute(A &&...a) {
      attributes.push_back(
        std::make_shared<attribute_t>(std::forward<A>(a)...));
      return shared_from_this();
    }

    template <typename C>
    C adopt_child(C child){
      contents.push_back(child);
      return child;
    }
    template <typename... A>
    node_t add_element(A &&...a) {
      return
        adopt_child(
          std::shared_ptr<element_t>(new element_t(std::forward<A>(a)...)));
    }

    structure::chaining_ptr_t<element_t> append_text(text_t text) {
      contents.push_back(std::make_shared<text_content_t>(text));
      return shared_from_this();
    }

    // "output_begin()" and "output_end()" are accessible for sync'ed output
    // (i.e., outputting a tree as it's being constructed); in that situation,
    // if the element is still to be given content, the "sync" argument must be
    // true, so that the no-content syntax ("<name ... />") isn't used
    void output_begin(std::ostream &os, size_t indent, bool sync=false) const {
      os << implementation::indent_text(indent) << "<" << name;
      for (auto const &a: attributes)
        a->output(os << " ");
      if (not contents.empty() or sync)
        os << ">"
           << (single_text_content() and not sync
               ? ""
               : "\n");
      else
        os << "/>\n";
    }
    void output_end(std::ostream &os, size_t indent, bool sync=false) const {
      if (not contents.empty() or sync)
        os << (single_text_content() and not sync
               ? ""
               : implementation::indent_text(indent))
           << "</" << name << ">\n";
    }
    //     <{name} {attributes...}> {children...} </{name}>
    //
    // or, when childless,
    //
    //     <{name}/>
    void output(std::ostream &os, size_t indent, bool) const override {
      output_begin(os, indent);

      size_t new_indent=indent+1;
      for (auto const &c: contents)
        c->output(os, new_indent, not single_text_content());

      output_end(os, indent);
    }

    using content_t::print_tree;
    void print_tree(std::ostream &os, size_t indent) const override {
      os << implementation::tree_indent_text(indent) << name;
      for (auto const &a: attributes)
        os << " " << a->name << "=\"" << a->value << "\"";
      os << std::endl;
      for (auto const &c: contents)
        c->print_tree(os, indent+1);
    }

    name_t const name;
    std::list<std::shared_ptr<attribute_t>> attributes;
    std::list<std::shared_ptr<content_t>> contents;
  private:
    element_t(name_t name) : name(name) { }

    bool single_text_content() const {
      return
        contents.size()==1
        and std::dynamic_pointer_cast<text_content_t>(contents.front());
    }
  };

  using action_t=std::function<void (element_t::node_const_t)>;

  inline std::ostream &operator<<(std::ostream &os, element_t const &e) {
    e.output(os, 0, true);
    return os;
  }

  struct read_ended_unexpectedly
    : public std::runtime_error {
    using runtime_error::runtime_error;
  };

  namespace implementation {

    inline text_t xml_decode_text(text_t t) {
      std::string result;
      for (std::size_t i=0; i<t.size(); ) {
        bool entity_reference=false;
        if (t[i]=='&')
          for (auto const &[c, x]: entity_references)
            if (t.substr(i, x.size())==x) {
              entity_reference=true;
              result+=c;
              i+=x.size();
              break;
            }
        if (not entity_reference)
          result+=t[i++];
      }
      return result;
    }

    template <typename F>
    inline auto expect_char(std::istream &is, F const &f, std::string f_name) {
      if (is.eof())
        throw std::runtime_error("EOF while expecting "+f_name);
      else if (char c=char(is.get()); not f(c))
        throw std::runtime_error(std::string("unexpected '")+char(c)
                                 +"' while expecting "+f_name);
      else
        return c;
    }

    inline auto expect_char(std::istream &is, char expected) {
      return expect_char(is,
                         [expected](auto c) { return c==expected; },
                         std::string("'")+expected+"'");
    }

    inline bool initial_name_char(char c)
      { return std::isalpha(c) or (c=='_'); }
    inline bool medial_name_char(char c)
      { return initial_name_char(c) or (c=='-') or (c==':'); }

    inline std::string read_name(std::istream &is) {
      is >> std::ws;
      char c=expect_char(
               is,
               [](char c) { return (c=='/') or initial_name_char(c); },
               "final or alphabetic or underscore");;
      std::string result{char(c)};
      while (not is.eof()) {
        c=char(is.peek());
        if (std::isalnum(c) or (c=='_') or (c=='-') or (c==':'))
          result+=char(is.get());
        else
          break;
      }
      return result;
    }

    inline std::string read_text(std::istream &is) {
      is >> std::ws;
      expect_char(is, '"');
      std::string result;
      auto c=0;
      while ((not is.eof()) and ((c=is.get()) not_eq '"'))
        result+=char(c);
      return result;
    }

    using recurse_f_t=std::function<bool (element_t::node_const_t)>;

    inline content_t::root_t read_content(
        std::istream &is,
        action_t open={},
        action_t close={},
        recurse_f_t recurse={},
        bool build_tree=true) {
      is >> std::ws;
      if (is.peek() not_eq '<') {
        text_t text;
        while ((not is.eof()) and (is.peek() not_eq '<'))
          text+=char(is.get());
        return std::make_shared<text_content_t>(xml_decode_text(text));
      }
      is.get(); // '<'
      std::string name=read_name(is);
      auto e=element_t::make_root(name);

      if (name[0]=='/') {
        // "closing-name"; not a real element, caught in the "while" loop below
        expect_char(is >> std::ws, '>');
        return e;
      }

      is >> std::ws;
      while (initial_name_char(char(is.peek()))) {
        auto name=read_name(is);
        is >> std::ws;
        expect_char(is, '=');
        is >> std::ws;
        auto value=xml_decode_text(read_text(is));
        e->append_attribute(name, value);
        is >> std::ws;
      }

      switch (is.get()) {
      case '/': {
        expect_char(is >> std::ws, '>');

        if (open) open(e);
        if (close) close(e);

        return e;
      } break;
      case '>': {
        bool recurse_this=not recurse or recurse(e);
        if (recurse_this and open) open(e);

        while (true) {
          if (is.eof())
            throw read_ended_unexpectedly(
              "the \""+e->name+"\" element ended unexpectedly");
          auto child=
            recurse_this
            ? read_content(is, open, close, recurse, build_tree)
            : read_content(is);
          auto child_as_element=std::dynamic_pointer_cast<element_t>(child);
          if (child_as_element and (child_as_element->name[0]=='/')) {
            if (child_as_element->name.substr(1) not_eq e->name)
              std::runtime_error(
                "closing \""+child_as_element->name
                +"\" doesn't match opening \""+e->name+"\"");

            if (not recurse_this and open) open(e);
            if (close) close(e);

            return e;
          }
          if (build_tree or not recurse_this)
            e->adopt_child(child);
        }
      } break;
      default:
        break;
      }
      return {}; // never reached
    }

  }

  inline element_t::root_t read_element(std::istream &is) {
    auto e=
      std::dynamic_pointer_cast<element_t>(implementation::read_content(is));
    if (e or not is)
      return e;
    else
      throw std::runtime_error("couldn't read an element");
  }

  inline element_t::node_const_t as_element(content_t::node_const_t c)
    { return c.cast<element_t const>(); }

  inline element_t::node_const_t must_be_element(content_t::node_const_t c) {
    auto e=as_element(c);
    if (not e)
      throw std::runtime_error("not an element");
    return e;
  }

  inline element_t::node_const_t must_be_element(content_t::node_const_t c,
                                                 name_t name) {
    auto e=must_be_element(c);
    if (e->name not_eq name)
      throw std::runtime_error("not a \""+name+"\" element (\""+e->name
                               +"\" instead)");
    return e;
  }

  inline text_t get_text(element_t::node_const_t e) {
    text_t result;
    for (content_t::node_const_t c: e->contents)
      if (auto ct=c.cast<text_content_t const>(); ct)
        result+=ct->text;
    return result;
  }

  inline bool has_attribute(element_t::node_const_t e, name_t name) {
    for (auto const &a: e->attributes)
      if (a->name==name)
        return true;
    return false;
  }

  inline text_t get_attribute(element_t::node_const_t e, name_t name) {
    for (auto const &a: e->attributes)
      if (a->name==name)
        return a->value;
    throw std::runtime_error("no \""+name+"\" attribute in \""+e->name
                             +"\" element");
  }

  inline element_t::node_const_t get_child_or_null(element_t::node_const_t e,
                                                   name_t name) {
    for (content_t::node_const_t c: e->contents)
      if (auto ce=c.cast<element_t const >(); ce and ce->name==name)
        return ce;
    return {};
  }

  inline element_t::node_const_t get_child(element_t::node_const_t e,
                                           name_t name) {
    if (auto ce=get_child_or_null(e, name))
      return ce;
    else
      throw std::runtime_error("no \""+name+"\" child in \""+e->name
                               +"\" element");
  }

  struct traverse_t {
    action_t open={};
    action_t close={};
    bool recurse=true;
  };
  using traverse_map_t=std::map<name_t, traverse_t>;

  inline void process(element_t::node_const_t e,
                      traverse_map_t const &ocm) {
    if (auto ei=ocm.find(e->name); ei not_eq ocm.end()) {
      auto traverse=ei->second;

      if (traverse.open)
        traverse.open(e);

      if (traverse.recurse)
        for (auto const &c: e->contents)
          if (auto ce=as_element(c))
            process(ce, ocm);

      if (traverse.close)
        traverse.close(e);
    }
    else
      throw std::runtime_error("no action for \""+e->name+"\" element");
  }

  inline content_t::root_t interpret_element(std::istream &is,
                                             traverse_map_t const &ocm) {
    auto const get_traverse=
      [&ocm] (element_t::node_const_t e) {
        if (auto ei=ocm.find(e->name); ei not_eq ocm.end())
          return ei->second;
        else
          throw std::runtime_error("no action for \""+e->name+"\" element");
      };
    return implementation::read_content(
      is,
      [&ocm, &get_traverse] (element_t::node_const_t e) {
        if (auto t=get_traverse(e); t.open)
          t.open(e);
      },
      [&ocm, &get_traverse] (element_t::node_const_t e) {
        if (auto t=get_traverse(e); t.close)
          t.close(e);
      },
      [&ocm, &get_traverse] (element_t::node_const_t e) {
        return get_traverse(e).recurse;
      },
      false);
  }
}

#endif
