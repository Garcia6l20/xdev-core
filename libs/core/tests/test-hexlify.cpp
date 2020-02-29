#include <gtest/gtest.h>
#include <xdev/xdev.hpp>
#include <iomanip>

using namespace xdev;

TEST(Hexlify, Dump) {
    uint8_t originals [255];
    char dumped [sizeof(originals) * 2];
    for (size_t ii = 0; ii < sizeof(originals) - 1; ++ii) {
        originals[ii] = uint8_t(ii);
        tools::hex_dump(uint8_t(ii), &dumped[ii * 2]);
        std::cout << originals[ii] << " -> " << std::string_view(&dumped[ii * 2], 2) << std::endl;
    }
}
