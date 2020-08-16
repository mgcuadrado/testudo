#ifndef TESTUDO_TRY_CATCH_HEADER_
#define TESTUDO_TRY_CATCH_HEADER_

#include <iostream>
#include <exception>

namespace testudo {

#define oxys_testudo_eat_a_semicolon static_cast<void>(0)
#define oxys_testudo_decl_eat_a_semicolon                                \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

#define begintrycatch try { oxys_testudo_eat_a_semicolon
#define endtrycatch } \
  catch (std::exception const &excp)                                         \
    { std::cerr << std::endl << "Exception: " << excp.what() << std::endl; } \
  catch (char const *mess)                                                   \
    { std::cerr << std::endl << "Error: " << mess << std::endl; }            \
  catch (...)                                                                \
    { std::cerr << std::endl << "Uncatched error" << std::endl; throw; }     \
    oxys_testudo_eat_a_semicolon

}

#endif
