#pragma once

#include <format>
#include <iostream>

namespace xdev {

    struct XLogCategory {
        const std::string id;
    };

    struct XLogger {
        const XLogCategory cat;

        template <typename S, typename... Args>
        inline auto& debug(const S& format_str, Args&&... args) const {
            std::cout << std::format<std::string, const std::string&, Args &&...>(
                std::string("[{}.debug]: ") + format_str, cat.id, std::forward<Args>(args)...) << std::endl;
            return std::cout;
        }

        template <typename S, typename... Args>
        inline auto& info(const S& format_str, Args&&... args) const {
            std::cout << std::format<std::string, const std::string&, Args &&...>(
                std::string("[{}.info]: ") + format_str, cat.id, std::forward<Args>(args)...) << std::endl;
            return std::cout;
        }

        template <typename S, typename... Args>
        inline auto& warning(const S& format_str, Args&&... args) const {
            std::cout << std::format<std::string, const std::string&, Args &&...>(
                std::string("[{}.warning]: ") + format_str, cat.id, std::forward<Args>(args)...) << std::endl;
            return std::cout;
        }

        template <typename S, typename... Args>
        inline auto& error(const S& format_str, Args&&... args) const {
            std::cout << std::format<std::string, const std::string&, Args &&...>(
                std::string("[{}.error]: ") + format_str, cat.id, std::forward<Args>(args)...) << std::endl;
            return std::cout;
        }

        template <typename S, typename... Args>
        inline auto& critical(const S& format_str, Args&&... args) const {
            std::cout << std::format<std::string, const std::string&, Args &&...>(
                std::string("[{}.critical]: ") + format_str, cat.id, std::forward<Args>(args)...) << std::endl;
            return std::cout;
        }
    };
}
