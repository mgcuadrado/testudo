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

#ifndef MGCUADRADO_PATTERN_NAMED_CREATE_HEADER_
#define MGCUADRADO_PATTERN_NAMED_CREATE_HEADER_

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <any>
#include <string>
#include <stdexcept>

namespace pattern {

  // the class "NamedCreator<>" implements an abstract factory for a single
  // kind of created object, with specified parameters to the object
  // constructor, and the possibility of adding more parameters for specific
  // object subclasses (these additional parameters are packed into an
  // "additional_t" instance)

  // "additional_t" is simply a "vector" of "any" instance, augmented with some
  // new capabilities: a method to check its size (that throws if the size
  // isn't correct), and a method to access a member by its index that, instead
  // of returning an "any" instance, returns a value that can be implicitly
  // converted to the type of the variable it's assigned to
  struct additional_t
    : public std::vector<std::any> {
    using vector::vector;
    struct any_convertible
      : public std::any {
      template <typename A>
      any_convertible(A &&a) : any(std::forward<A>(a)) { }
      template <typename T>
      operator T() const {
        if (type() not_eq typeid(T))
          throw std::invalid_argument(
            std::string("error trying to cast an \"any\" with type \"")
            +type().name()+"\" to type \""+typeid(T).name()+"\"");
        return std::any_cast<T>(*this);
      }
    };
    any_convertible operator[](size_t i) const
      { return vector::operator[](i); }
    void check_size(size_t s) const {
      if (size() not_eq s)
        throw std::invalid_argument(
          "wrong number of additional arguments: "+std::to_string(size())
          +" (should be"+std::to_string(s)+")");
    }
  };

  // "NamedCreator<>" is templatised on the base type it creates "T" and the
  // common constructor parameter types "P..."; creators (concrete factories)
  // are functions that accept "P..." parameters, plus possibly additional
  // parameters that are packed into an "additional_t" instance, and return a
  // shared pointer to a newly created object of declared type "T"
  template <typename T, typename... P>
  class NamedCreator {
  public:
    using creator_t=
      std::function<std::shared_ptr<T> (P const &...p,
                                        additional_t const &pa)>;
    struct creator_caller_t {
      creator_caller_t(creator_t const &creator) : creator(creator) { }
      template <typename... PA>
      std::shared_ptr<T> operator()(P const &... p, PA &&...pa) const
        { return creator(p..., additional_t{std::any(pa)...}); }
    private:
      creator_t const creator;
    };

    // get the creator registered under the given name; the returned value
    // isn't actually the original creator that was registered, but a wrapper
    // around it that copes with additional parameters, packing them into an
    // "additional_t" instance
    template <typename... PA>
    creator_caller_t operator()(std::string name) const {
      auto it=creators.find(name);
      if (it==creators.end())
        throw std::out_of_range("unknown creator name \""+name+"\"");
      return it->second;
    }

  private:
    // the creator registration is done through the "register_creator<>" class
    void register_creator_function(std::string name, creator_t creator)
      { creators[name]=creator; }
    std::map<std::string, creator_t> creators;
    template <typename CONST>
    friend struct register_creator;
  };

  template <typename CT>
  struct register_creator {
    template <typename T, typename...P>
    register_creator(NamedCreator<T, P...> &named_creator,
                     std::string name) {
      if constexpr (std::is_constructible_v<
                      CT, P const &..., additional_t const &>)
        // the constructor specifies a final "additional_t" parameter
        named_creator.register_creator_function(
          name,
          [](P const &...p, additional_t const &pa)
            { return std::make_shared<CT>(p..., pa); });
      else
        // the constructor accepts just the common parameters
        named_creator.register_creator_function(
          name,
          [](P const &...p, additional_t const &pa) {
            pa.check_size(0);
            return std::make_shared<CT>(p...);
          });
    }
  };

  // macro to ease the creation of a named creator; it's usage is like this:
  //
  //     define_named_creator(animal_named_creator,
  //                          AnimalNamedCreator,
  //                          Animal, string, unsigned);
  //
  // which defines a type alias named "AnimalNamedCreator" for
  // "NamedCreator<Animal, string, unsigned>", and then an instance for that
  // named creator type named "animal_named_creator", wrapped into an
  // anti-initialisation-order fiasco device (which means you have to use
  // "animal_named_creator()" when referring to it); it also defines an
  // overload of "animal_named_creator()" so you can use
  //
  //     animal_named_creator("mammal")
  //
  // as a shortcut, instead of
  //
  //     animal_named_creator()("mammal")
#define define_named_creator(n_c_name, n_c_type, ...)                   \
  using n_c_type=pattern::NamedCreator<__VA_ARGS__>;                    \
  inline auto &n_c_name() {                                             \
    static n_c_type result;                                             \
    return result;                                                      \
  }                                                                     \
  inline auto n_c_name(std::string name)                                \
    { return n_c_name()(name); }                                        \
  struct never_define_this_struct_it_just_eats_a_semicolon

}

#endif
