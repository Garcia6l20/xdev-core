/**
 * @file xdev-object-pool.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/object.hpp>

#include <map>

#ifdef _WIN32
#pragma warning( disable : 4455) // disable user-defined literals
#endif

#define XPOOLREGISTER(__name, ...) \
    extern XObjectPool __name ## Pool; \
    static XObject::ptr operator "" ## __name (const char* name, size_t s) \
    { \
        return __name ## Pool[name]; \
    }

namespace xdev {

class XObjectPool: public map<string, XObjectBase::ptr> {

};

} // namespace xdev
