#include <xdev/xdev.hpp>
#include <gtest/gtest.h>
#include <iomanip>

using namespace xdev;

TEST(Hexlify, Dump) {
    char originals [255];
    char dumped [sizeof(originals) * 2];
    for (size_t ii = 0; ii < sizeof(originals) - 1; ++ii) {
        originals[ii] = char(ii);
        tools::hex_dump(char(ii), &dumped[ii * 2]);
        std::cout << originals[ii] << " -> " << std::string_view(&dumped[ii * 2], 2) << std::endl;
    }
}
