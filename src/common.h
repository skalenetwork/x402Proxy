#pragma once

#include <cassert>
#include <string>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    constexpr bool exceptions_enabled = true;
#else
constexpr bool exceptions_enabled = false;
#endif

static_assert(exceptions_enabled, "Exceptions must be enabled!");

#define CHECK_STATE( _EXPRESSION_ )                                                        \
    if ( !( _EXPRESSION_ ) ) {                                                             \
        auto __msg__ = std::string( "State check failed::" ) + #_EXPRESSION_ + " " +       \
                       std::string( __FILE__ ) + ": " + std::to_string( __LINE__ );        \
        throw std::logic_error( __msg__ +  ":" + __FUNCTION__ );                           \
    }


#define CHECK_STATE2( _EXPRESSION_, _MSG_ )                                                              \
    if ( !( _EXPRESSION_ ) ) {                                                                           \
        auto __msg__ = std::string( "Check failed::" ) + #_EXPRESSION_ + " " + std::string( __FILE__ ) + \
                       ":" + std::to_string( __LINE__ );                                                      \
        throw std::logic_error( __msg__ + ": " + _MSG_ + ":" +  __FUNCTION__ );                          \
    }

inline std::string stripSpaces(std::string _s) {
    _s.erase(std::remove_if(_s.begin(), _s.end(), ::isspace), _s.end());
    return _s;
}

#include <glog/logging.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <stdexcept>