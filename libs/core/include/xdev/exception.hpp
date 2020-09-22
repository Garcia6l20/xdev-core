/**
 * @file xdev-exception.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/core.hpp>

namespace xdev {
    class XDEV_CORE_EXPORT XException: public runtime_error
    {
    public:
        explicit XException();
        explicit XException(const char* what);
        explicit XException(const string& what);
        XException(const std::exception& exception);
        explicit XException(const char* what, const XException& exception);
        explicit XException(const string& what, const XException& exception);
        XException(const XException&);
        XException(XException&&);
        XException& operator=(const XException&);
        XException& operator=(XException&&);
        virtual ~XException();
        virtual const char* what() const noexcept;
        bool empty() const { return _empty; }
    private:
        void init();
        bool _empty;
        std::unique_ptr<XException> m_parent;
        string m_what;
	};
}

namespace std {
inline ostream& operator<<(ostream& st, const xdev::XException& e) {
    st << e.what();
    return st;
}
}
