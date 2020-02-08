#include <xdev/xdev-app.hpp>

using namespace xdev;

XApplication::Error::Error(const std::string & what, int rc):
    XException(what),
    m_rc(rc)
{
}
