#include <xdev/xdev-resources.hpp>
#include <gtest/gtest.h>

#include <resources/xdev-rc-json-load-test-resources.hpp>

using namespace xdev;

TEST(JSonResources, Loading) {
    auto root = XdevRcJsonLoadTestResources->getJson("test.json").get<XDict>();
    ASSERT_EQ(root["the_response"], 42.0);
}
