#include <xdev/xdev-basic-rc.hpp>
#include <xdev/xdev-core.hpp>
#include <xdev/xdev-tools.hpp>
#include <xdev/xdev-base64.hpp>

#include <iostream>
#include <fstream>

using namespace xdev;
using namespace xdev::rc;

namespace fs = xdev::filesystem;

XBasicResourceCompiler::XBasicResourceCompiler(const string& name, const fs::path& path):
    m_name(name),
    m_path(path)
{
}

void XBasicResourceCompiler::processFile(const fs::path& path, const string& root)
{
	string input_file = path.string();
    string basename = path.filename().string();

    if (!root.empty()) {
        fs::create_directories(root);
        basename = root + "/" + basename;
    }

	string resource_file = basename + ".xrc";
	string variable_name = regex_replace(basename, regex(R"([-\./(\\)])"), "_");
	ifstream input(path.string());

	cout << "-- generating: " << resource_file << endl;
	ofstream output(resource_file);
	output << "// key: " << basename << endl;
    output << "const uint8_t " << variable_name << "_data[] = {";

    char tmp_dump[2];
    for_each(istreambuf_iterator<char>(input), istreambuf_iterator<char>(), [&output, &tmp_dump](char c)
    {
        tools::hex_dump(static_cast<uint8_t>(c), tmp_dump);
        output << "0x";
        output.write(tmp_dump, sizeof(tmp_dump));
        output << ",";
    });
    output << "};" << endl;

    output << "const size_t " << variable_name << "_len = sizeof(" << variable_name << "_data);" << endl;
    output << "const xdev::XResources::resource_data_t " << variable_name << " = {" << endl;
    output << "    " << variable_name << "_data, " << variable_name << "_len" << endl;
	output << "};" << endl;

	rc_map mapping;
	mapping.key = basename;
	mapping.resource_path = resource_file;
	mapping.resource_name = variable_name;
	m_rcMaps.push_back(mapping);
}

#include <algorithm>
void XBasicResourceCompiler::createResourceFiles() {
	string cpp_filename = m_name + "-resources.cpp";
	string h_filename = m_name + "-resources.hpp";
	string base_name = regex_replace(m_name, regex(R"([-\.])"), "_");
	string header_lock_name = base_name;
	tools::to_upper(header_lock_name);
	tools::title(base_name);
	base_name.erase(std::remove(base_name.begin(), base_name.end(), '_'), base_name.end());

	ofstream h_output(h_filename);
	h_output << "#pragma once" << endl;
	h_output << endl;
	h_output << "#include <xdev/xdev-resources.hpp>" << endl;
	h_output << endl;
	h_output << "extern const xdev::XResources::ptr " << base_name << "Resources;" << endl;
	h_output << endl;

	ofstream cpp_output(cpp_filename);
	cpp_output << "#include <" << h_filename << ">" << endl;
	cpp_output << endl;
	cpp_output << "using namespace xdev;" << endl;
	cpp_output << endl;
	for (const rc_map& item : m_rcMaps)
	{
		cpp_output << "#include <" << item.resource_path << ">" << endl;
	}
	cpp_output << endl;
	cpp_output << "static XResources::ptr _load_resources()" << endl;
	cpp_output << "{" << endl;
	cpp_output << "    XResources::ptr resources = XResources::Make();" << endl;
	for (const rc_map& item : m_rcMaps)
    {
		cpp_output << "    resources->add(\"" << item.key << "\", " << item.resource_name << ");" << endl;
	}
	cpp_output << "    return resources;" << endl;
	cpp_output << "}" << endl;
	cpp_output << endl;
	cpp_output << "const XResources::ptr " << base_name << "Resources = _load_resources();" << endl;
}

void XBasicResourceCompiler::compile() {
	if (!fs::exists(m_path))
	{
		throw CompileError(std::string() + "Error: path does not exist: " + m_path.string());
	}

	cout << "Generating ressources for: " << m_name << std::endl;

	tools::walk_directory(m_path, [&](const fs::path& path, const fs::path& root) {
        auto relative = root.string();
        relative.erase(relative.find(m_path.string()), m_path.string().size());
        relative = regex_replace(relative, regex(R"(\\)"), "/");
        if (relative[0] == '/')
            relative.erase(0, 1);
		processFile(path, relative);
	});

	createResourceFiles();
}
