#include <xdev/xdev-object.hpp>

using namespace xdev;

//
// XObject
//

XObjectBase::XObjectBase():
    _metaData {
      .objectName = "undefined"
    }
{
}

size_t XObjectBase::_init() {
    auto instance_num = _get_static_class().operator++();
    _metaData.objectName = staticClass().name() + "#" + to_string(instance_num);
    return instance_num;
}

XObjectBase::~XObjectBase()
{
}

XStaticClass::~XStaticClass() {}
