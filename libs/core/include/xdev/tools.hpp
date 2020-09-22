/**
 * @file xdev-tools.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <chrono>
#include <functional>
#include <regex>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <variant>

#include <xdev/typetraits.hpp>

namespace xdev::tools {

#define XDEV_BENCHMARK(__FUNC_CALL) \
{ \
	auto t1 = chrono::high_resolution_clock::now(); \
	(__FUNC_CALL) \
	auto t2 = std::chrono::high_resolution_clock::now(); \
    chrono::duration<double, std::milli> ms = t2 - t1; \
    cout << "" \
		 << " took " << ms.count() << " ms\n"; \
}

template <typename Iterable, typename Lambda>
inline void fast_foreach(Iterable iterable, Lambda lambda)
{
	for_each(iterable.begin(), iterable.end(), lambda);
}

template <typename Lambda>
inline void regex_foreach(const std::string& input, const std::regex& expr, Lambda lambda)
{
    auto begin = std::sregex_iterator(input.begin(), input.end(), expr);
    auto end = std::sregex_iterator();
    for (auto ii = begin; ii != end; ++ii)
	{
		lambda(*ii);
	}
}

inline std::string regex_escape(const std::string& input)
{
    auto result = input;
    size_t offset = 0;
    while ((offset = result.find_first_of("{}[]()<>.+*^$|\\", offset)) != std::string::npos)
    {
        result.insert(offset, 1, '\\');
        offset += 2;
    }
    return result;
}

/**
 * @brief split string
 */
inline std::vector<std::string> split(const std::string &text, char sep, bool remove_empty = false)
{
    std::vector<std::string> tokens;
	size_t start = 0, end = 0;
    std::string item;
	while ((end = text.find(sep, start)) != std::string::npos) {
		item = text.substr(start, end - start);
        if (!remove_empty || (remove_empty && !item.empty()))
		{
			tokens.push_back(item);
		}
		start = end + 1;
	}
	item = text.substr(start);
    if (!remove_empty || (remove_empty && !item.empty()))
	{
		tokens.push_back(item);
	}
	return tokens;
}

/**
 * @brief Join iterable
 */
template <typename Iterable, typename SeparatorT, typename Converter = std::nullptr_t>
inline std::string join(const Iterable& iterable, const SeparatorT& sep, Converter converter = nullptr)
{
    using ConverterT = std::decay_t<decltype (converter)>;

    std::string result;

    if (iterable.begin() == iterable.end())
        return result;

    auto last = iterable.end();
    --last;
    for (auto it = iterable.begin(); it != last; ++it)
    {
        if constexpr (std::is_same<ConverterT, std::nullptr_t>::value)
            result += *it + sep;
        else result += converter(*it) + sep;
    }
    if constexpr (std::is_same<ConverterT, std::nullptr_t>::value)
        result += *last;
    else result += converter(*last);
    return result;
}

inline void to_upper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline void to_lower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

inline void replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

/**
 * @brief remove whitespaces
 * @param str
 */
inline void remove_whitespaces(std::string& str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
}

/**
 * @brief trim from start (in place)
 */
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !isspace(ch);
	}));
}


/**
* @brief trim from end (in place)
*/
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !isspace(ch);
	}).base(), s.end());
}

/**
* @brief trim both ends (in place)
*/
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}


static inline void title(std::string& s)
{
	bool active = true;
    for(auto&&c: s) {
		if (isalnum(c))
		{
			if (active)
			{
                c = static_cast<char>(toupper(c));
				active = false;
			}
			else
            {
                c = static_cast<char>(tolower(c));
			}
		}
		else
		{
			active = true;
		}
    }
}

template <typename Lambda>
void walk_directory(const std::filesystem::path& root, Lambda on_file_callback, const std::regex& match_erpr = std::regex(".*"))
{
    namespace fs = std::filesystem;
    for (const auto& p : fs::directory_iterator(root))
	{
        if (fs::is_directory(p))
		{
			walk_directory(p, on_file_callback, match_erpr);
		}
        else if (fs::is_regular_file(p) && regex_match(p.path().string(), match_erpr))
		{
			on_file_callback(p, root);
		}
	}
}

inline void hex_dump(uint8_t c, char out[2])
{
    char low_bits = c & 0x0f;
    char high_bits = static_cast<char>(c >> 4);
    if (low_bits < 10)
        out[1] = static_cast<char>(low_bits + '0');
    else out[1] = static_cast<char>(low_bits - 10 + 'a');
    if (high_bits < 10)
        out[0] = static_cast<char>(high_bits + '0');
    else out[0] = static_cast<char>(high_bits - 10 + 'a');
}

inline std::string hexlify(const char* input, size_t length)
{
    std::string buffer;
    buffer.reserve(length * 2 - 1);
    size_t ii = 0;
    for (; ii < length; ++ii)
    {
        hex_dump(static_cast<uint8_t>(input[ii]), &buffer[ii * 2]);
    }
    return buffer;
}

inline std::string hexlify(const std::string& input)
{
    return hexlify(input.c_str(), input.size());
}

template <typename Class>
struct make_shared_enabler : public Class {
    template <typename... Args>
    make_shared_enabler(Args &&... args) :
        Class(xfwd(args)...)
    {}
};

template <typename Class>
struct SharedMaker {
    template <typename... Args>
    static std::shared_ptr<Class> Make(Args &&... args)
    {
        return std::make_shared<make_shared_enabler<Class>>(xfwd(args)...);
    }
};

//
// visit tools
//

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; }; // (1)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;  // (2)

/**
 * @brief visit_2way
 * @param handler a Callable taking an lhs value and an rhs value
 * @param lhs The lhs std::variant value
 * @param rhs The rhs std::variant value
 */
constexpr auto visit_2way(auto handler, auto lhs, auto rhs) {
    return std::visit(overloaded{
       [&handler, &rhs](const auto& lhs_item) {
          return std::visit(overloaded{
             [&handler, lhs_item](const auto& rhs_item) {
                 return handler(lhs_item, rhs_item);
             },
          }, rhs);
       },
    }, lhs);
}

// TODO(me) Implement this sexy thing !!!

//template <typename FirstT, typename...Ts>
//auto visit_nway(auto handler, FirstT lhs, Ts...rhss) {
//    return std::visit(overloaded{
//       [&handler, &rhs](const auto& lhs_item) {
//          return std::visit(overloaded{
//             [&handler, lhs_item](const auto& rhs_item) {
//                 return handler(lhs_item, rhs_item);
//             },
//          }, rhs);
//       },
//    }, lhs);
//}

class finally {
public:
    finally(std::function<void()>&& handler): _handler{std::move(handler)} {}
    ~finally() {
        _handler();
    }
private:
    std::function<void()> _handler;
};

} // namespace xdev::tools
