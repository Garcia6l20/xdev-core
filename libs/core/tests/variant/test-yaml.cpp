#include <xdev/xdev.hpp>
#include <xdev/xdev-yaml.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace xdev;
using namespace std;

TEST(VariantYamlTest, String) {
    auto data = yaml::parse(R"(
"42"
)");
    ASSERT_TRUE(data.is<string>());
    ASSERT_EQ(data, "42");
}

TEST(VariantYamlTest, Double) {
    auto data = yaml::parse(R"(
42.
)");
    ASSERT_TRUE(data.is<double>());
    ASSERT_EQ(data, 42.);
}

TEST(VariantYamlTest, Integer) {
    auto data = yaml::parse(R"(
42
)");
    ASSERT_TRUE(data.is<int>());
    ASSERT_EQ(data, 42);
}


TEST(VariantYamlTest, MappingString) {
    auto data = yaml::parse(R"(
string: "42"
)");
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("string").is<string>());
    ASSERT_EQ(data.get<XDict>().at("string"), "42");
}

TEST(VariantYamlTest, MappingDouble) {
    auto data = yaml::parse(R"(
double: 42.
)");
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("double").is<double>());
    ASSERT_EQ(data.get<XDict>().at("double"), 42.0);
}

TEST(VariantYamlTest, Mapping) {
    auto data = yaml::parse(R"(
string: "42"
question: What is the answer \
to everything ?
double: 42.
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("string").is<string>());
    ASSERT_EQ(data.get<XDict>().at("string"), "42");
    ASSERT_TRUE(data.get<XDict>().at("question").is<string>());
    ASSERT_EQ(data.get<XDict>().at("question"), "What is the answer to everything ?");
    ASSERT_TRUE(data.get<XDict>().at("double").is<double>());
    ASSERT_EQ(data.get<XDict>().at("double"), 42.0);
}

TEST(VariantYamlTest, Mapping2Lvl) {
    auto data = yaml::parse(R"(
root:
    string: "42"
    double: 42.
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("root.string").is<string>());
    ASSERT_EQ(data.get<XDict>().at("root.string"), "42");
    ASSERT_TRUE(data.get<XDict>().at("root.double").is<double>());
    ASSERT_EQ(data.get<XDict>().at("root.double"), 42.0);
}

TEST(VariantYamlTest, Mapping3Lvl) {
    auto data = yaml::parse(R"(
root:
    sub:
        string: "42"
        array:
            - 1
            - 2.
            - "3"
        double: 42.
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("root.sub.string").is<string>());
    ASSERT_EQ(data.get<XDict>().at("root.sub.string"), "42");
    ASSERT_TRUE(data.get<XDict>().at("root.sub.double").is<double>());
    ASSERT_EQ(data.get<XDict>().at("root.sub.double"), 42.0);
    ASSERT_EQ(data.get<XDict>().at("root.sub.array").get<XArray>()[0], 1);
    ASSERT_EQ(data.get<XDict>().at("root.sub.array").get<XArray>()[1], 2.);
    ASSERT_EQ(data.get<XDict>().at("root.sub.array").get<XArray>()[2], "3");
}
TEST(VariantYamlTest, SequenceOfScalars) {
    auto data = yaml::parse(R"(
- "42"
- 0
- 42.
- 42
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XArray>());
    ASSERT_TRUE(data.get<XArray>()[0].is<string>());
    ASSERT_EQ(data.get<XArray>()[0], "42");
    ASSERT_TRUE(data.get<XArray>()[2].is<double>());
    ASSERT_EQ(data.get<XArray>()[2], 42.0);
    ASSERT_TRUE(data.get<XArray>()[3].is<int>());
    ASSERT_EQ(data.get<XArray>()[3], 42);
}

TEST(VariantYamlTest, MappingOfSequences) {
    auto data = yaml::parse(R"(
array:
    - "42"
    - 0
    - 42.
    - 42
inline_array: [1,2,3,4,5]
)");
    std::cout << data << std::endl;
    auto d = data.get<XDict>();
    auto array = d["array"].get<XArray>();
    ASSERT_TRUE(array[0].is<string>());
    ASSERT_EQ(array[0], "42");
    ASSERT_TRUE(array[2].is<double>());
    ASSERT_EQ(array[2], 42.0);
    ASSERT_TRUE(array[3].is<int>());
    ASSERT_EQ(array[3], 42);
    auto inline_array = d["inline_array"].get<XArray>();
    for (size_t ii = 0; ii < 4; ++ii) {
        ASSERT_TRUE(inline_array[ii].is<int>());
        ASSERT_EQ(inline_array[ii], int(ii + 1));
    }

}

TEST(VariantYamlTest, SJonSequence) {
    auto data = yaml::parse(R"(
["42", 0,
42
]
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XArray>());
    ASSERT_TRUE(data.get<XArray>()[0].is<string>());
    ASSERT_EQ(data.get<XArray>()[0], "42");
    ASSERT_TRUE(data.get<XArray>()[2].is<int>());
    ASSERT_EQ(data.get<XArray>()[2], 42);
}

TEST(VariantYamlTest, InlineSJonSequence) {
    auto data = yaml::parse(R"(["42",42])");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XArray>());
    ASSERT_TRUE(data.get<XArray>()[0].is<string>());
    ASSERT_EQ(data.get<XArray>()[0], "42");
    ASSERT_TRUE(data.get<XArray>()[1].is<int>());
    ASSERT_EQ(data.get<XArray>()[1], 42);
}

TEST(VariantYamlTest, JsonMapping2Lvl) {
    auto data = yaml::parse(R"(
root: {
    string: "42",
    double: 42.
}
)");
    std::cout << data << std::endl;
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_TRUE(data.get<XDict>().at("root.string").is<string>());
    ASSERT_EQ(data.get<XDict>().at("root.string"), "42");
    ASSERT_TRUE(data.get<XDict>().at("root.double").is<double>());
    ASSERT_EQ(data.get<XDict>().at("root.double"), 42.0);
}
