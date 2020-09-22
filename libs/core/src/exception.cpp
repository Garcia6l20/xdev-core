#include <xdev/exception.hpp>

#include <sstream>

using namespace xdev;

XException::XException(const XException& e):
    runtime_error(e),
    _empty(e._empty),
    m_parent(nullptr),
    m_what(e.m_what) {

}

XException::XException(XException&& e):
    runtime_error(e),
    _empty(e._empty),
    m_parent(std::move(e.m_parent)),
    m_what(move(e.m_what)) {
    e.m_parent = nullptr;
}

XException& XException::operator=(const XException& e) {
    _empty = e._empty;
    m_parent = nullptr;
    m_what = e.m_what;
    return *this;
}

XException& XException::operator=(XException&& e) {
    _empty = e._empty;
    m_parent = std::move(e.m_parent);
    e.m_parent = nullptr;
    m_what = move(e.m_what);
    return *this;
}

XException::XException() :
    runtime_error("empty"),
    _empty(true),
    m_parent(nullptr) {
    init();
}

XException::XException(const char* what) :
    runtime_error(what),
    _empty(false),
    m_parent(nullptr) {
    init();
}

XException::XException(const string& what) :
    runtime_error(what.c_str()),
    _empty(false),
    m_parent(nullptr) {
    init();
}

XException::XException(const exception& exception) :
    runtime_error(exception.what()),
    _empty(false),
    m_parent(nullptr) {
    init();
}

XException::XException(const char* what, const XException& exception) :
    runtime_error(what),
    _empty(false),
    m_parent(std::make_unique<XException>(exception))
{
    init();
}

XException::XException(const string& what, const XException& exception) :
    runtime_error(what.c_str()),
    _empty(false),
    m_parent(std::make_unique<XException>(exception)) {
    init();
}

XException::~XException() {
}

void XException::init() {
    m_what = runtime_error::what();
    if (m_parent != nullptr)
    {
        m_what += "\n  -- Caused by : ";
        m_what += m_parent->what();
    }
}

const char* XException::what() const noexcept {
    return m_what.c_str();
}

