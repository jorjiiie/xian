#pragma once
#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef DEBUG_ENABLED
#include <iostream>
namespace debugging {

template <typename... Args>
void print(const char *file, int line, Args... args) {
  (std::cerr << "[" << file << ":" << line << "] " << ... << args) << std::endl;
}

} // namespace debugging
#define DEBUG(...) debugging::print(__FILE__, __LINE__, __VA_ARGS__)

#define ASSERT(COND, MSG)                                                      \
  if (!(COND))                                                                 \
  DEBUG(MSG), exit(1)
#else

#define DEBUG(...) ((void)0)
#define ASSERT(COND, MSG) ((void)0)

#endif // debug_enabled

#define FAIL(MSG) std::cerr << MSG << std::endl, exit(1);
#endif
