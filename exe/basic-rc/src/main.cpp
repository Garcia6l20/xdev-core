#include <xdev/xdev-basic-rc.hpp>

#include <iostream>

using namespace xdev;
using namespace xdev::rc;

void help()
{
    cout << "Usage: xdev-basic-rc PROJECT_NAME RESOURCE_PATH" << std::endl;
	cout << "    Build application resources." << std::endl;
}

int main(int argc, char** argv) try
{
    if (argc < 3)
	{
        std::cerr << "argument(s) missing..." << std::endl;
		help();
		return -1;
	}

	string name = argv[1];
    std::cout << "project: " << name << std::endl;

	filesystem::path path = argv[2];
    std::cout << "resource path: " << path << std::endl;

	XBasicResourceCompiler(name, path).compile();

	return 0;
}
catch (const XBasicResourceCompiler::CompileError& error) {
    cerr << "Compilation error: " << error.what() << endl;
    return -1;
}
