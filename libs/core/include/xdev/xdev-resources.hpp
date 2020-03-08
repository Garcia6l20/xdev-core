/**
 * @file xdev-resources.hpp
 */
#pragma once

#include <xdev/xdev-core.hpp>
#include <xdev/xdev-tools.hpp>
#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-typetraits.hpp>
#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-base64.hpp>

#include <map>
#include <vector>
#include <fstream>
#include <iterator>
#include <any>

namespace xdev {

	class XResources: public tools::SharedMaker<XResources>
    {
	public:
        using ptr = shared_ptr<XResources>;

        struct resource_data_t {
            const uint8_t* data;
            const size_t len;
        };

        template <typename AnyT, typename = enable_if_t<!is_same_v<resource_data_t, AnyT>>>
        void add(const string& key, const AnyT& data)
        {
            m_resources.insert_or_assign(key, data);
        }

        void add(const string& key, const resource_data_t& data)
        {
            m_resources.insert_or_assign(key, string(data.data, data.data + data.len));
        }

        void add(const string& key, const filesystem::path& path)
        {
            ifstream in(path.string());
            add(key, string(istreambuf_iterator<char>(in), istreambuf_iterator<char>()));
        }

        string getString(const string& key) const
		{
            const std::any& data = m_resources.at(key);
            return std::any_cast<string>(data);
		}
        xvar getJson(const string& key) const {
            return xvar::FromJSON(getString(key));
        }
        xvar getYaml(const string& key) const {
            return xvar::FromYAML(getString(key));
        }
        const any& get(const string& key) const
        {
            return m_resources.at(key);
        }
        const any& operator[](const string& key) const
        {
            return m_resources.at(key);
		}
		bool has(const std::string& key) const
		{
			return m_resources.find(key) != m_resources.end();
		}
	private:
        XResources() {}
        map<std::string, any> m_resources;
        friend struct tools::make_shared_enabler<XResources>;
    };
} // namespace xdev
