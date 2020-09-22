/**
 * @file xdev-app.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/object.hpp>
#include <xdev/exception.hpp>

#define XAPPLICATION(...)
#define XAPPLICATION_IMPL(__object_type) \
    XApplication::ptr _xdev_application; \
    XApplication::ptr _xdev_create_application() \
    { \
        if (!_xdev_application) \
        { \
            _xdev_application = XObject::Create<__object_type>(); \
        } \
        return _xdev_application; \
    }

namespace xdev {
    class XApplication: public XObjectBase
    {
    public:

        using ptr = shared_ptr<XApplication>;

        class Error : public XException
        {
        public:
            Error(const std::string& what, int rc = -1);
            inline int rc() const { return m_rc; }
        private:
            int m_rc;
        };

        virtual void run() = 0;
    protected:
    };
}
