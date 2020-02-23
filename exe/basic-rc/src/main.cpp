#include <xdev/xdev-basic-rc.hpp>

#include <lyra/lyra.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

#include <iostream>

using namespace xdev;
using namespace xdev::rc;

void help()
{
    cout << "Usage: xdev-basic-rc PROJECT_NAME RESOURCE_PATH" << std::endl;
	cout << "    Build application resources." << std::endl;
}

int main(int argc, char** argv) {

    bool help = false;
    bool verbose = false;
    string project_name;
    filesystem::path resource_path;
    auto cli = lyra::help(help) |
            lyra::opt(verbose, "verbose")["-v"]["--verbose"]("Let me talk about me !") |
            lyra::arg(project_name, "project name")("The name of the project") |
            lyra::arg(resource_path, "resource path")("Path to the resources to compile");

    auto res = cli.parse({argc, argv});
    if (!res) {
         fmt::print(fg(fmt::color::red), "error: {}\n", res.errorMessage());
         return -1;
    }

    if (help) {
        std::cout << cli << '\n';
        return 0;
    }

    try {
        XBasicResourceCompiler(project_name, resource_path).compile();
        return 0;
    } catch (const XBasicResourceCompiler::CompileError& error) {
        fmt::print(fg(fmt::color::red), "Compilation error: {}\n", error.what());
        return -2;
    }
}
