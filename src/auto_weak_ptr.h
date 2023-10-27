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

#ifndef Killing_Me_Softly_aXing_Me_Lightly_auto_weak_ptr_HEADER_GUARD_
#define Killing_Me_Softly_aXing_Me_Lightly_auto_weak_ptr_HEADER_GUARD_

#include <memory>

#ifdef KMSXML_TEST
namespace kmsxml_test
#else
namespace kmsxml
#endif
{

  namespace structure {

    // weak pointer with auto locking
    template <typename T>
    class auto_weak_ptr {
    public:
      auto_weak_ptr() { }
      template <typename U>
      auto_weak_ptr(std::weak_ptr<U> const &wp) : wp(wp) { }
      template <typename U>
      auto_weak_ptr(std::shared_ptr<U> const &sp) : wp(sp) { }
      template <typename U>
      operator auto_weak_ptr<U>() const { return auto_weak_ptr<U>(wp); }

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

  }

}

#endif
