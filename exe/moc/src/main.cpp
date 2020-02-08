#include <xdev/xdev-moc.hpp>

#include <iostream>

using namespace xdev;

void help()
{
    cout << "Usage: xdev-moc PROJECT_NAME HEADER_FILE ..." << std::endl;
}

int main(int argc, char** argv)
{
    if (argc <= 2)
	{
		help();
		return -1;
	}

    string project = argv[1];
    XMetaObjectCompiler compiler(project, {&argv[2], &argv[2] + argc - 2});
	try
	{
		compiler.compile();
	}
	catch (const XMetaObjectCompiler::CompileError& error) {
		cerr << error.what() << endl;
		return -1;
	}
	return 0;
}
