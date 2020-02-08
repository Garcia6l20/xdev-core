#include <gtest/gtest.h>
#include <xdev/xdev-logger.hpp>

using namespace xdev;

static const auto TestCat = XLogCategory{"test.logger"};
static const auto TestLogger = XLogger{TestCat};

TEST(Logger, Nominal) {
    TestLogger.debug("{} + {} = {}", 40, 2, 42);
    TestLogger.info("{} + {} = {}", 40, 2, 42);
    TestLogger.warning("{} + {} = {}", 40, 2, 42);
    TestLogger.error("{} + {} = {}", 40, 2, 42);
    TestLogger.critical("{} + {} = {}", 40, 2, 42);
    std::cout << std::format("{} + {} = {}\n", 40, 2, 42);
}
