/**
 * @file xdev-object-pool.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-object.hpp>

#include <map>

#pragma warning( disable : 4455) // disable user-defined literals

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
