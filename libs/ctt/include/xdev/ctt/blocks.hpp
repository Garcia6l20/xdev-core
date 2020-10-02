/** @file xdev/ctt/blocks.hpp
 * @brief Compile-time template blocks
 *
 */
#pragma once

#include <xdev/ctt/evaluator.hpp>
#include <xdev/ctt/tokenizer.hpp>
#include <xdev/str-tools.hpp>
#include <xdev/variant.hpp>

#include <ctre.hpp>

#include <spdlog/spdlog.h>

namespace xdev::ctt {

  template <ct::string input, ct::ct_tuple toks, ct::ct_tuple output = ct::tuple<>, size_t index = 0,
            bool ltrim = false>
  constexpr auto generate_blocks_impl();

  namespace blocks {
    template <bool trim_before_, bool trim_after_>
    struct base_control {
      static constexpr auto trim_before = trim_before_;
      static constexpr auto trim_after  = trim_after_;
    };

    struct for_control {

      template <auto blocks_, ct::string iter_, ct::string container_, bool trim_before_, bool trim_after_>
      struct impl_t : base_control<trim_before_, trim_after_> {
        static constexpr auto iter      = iter_;
        static constexpr auto container = container_;
        using blocks_t                  = decltype(blocks_);
        template <typename OutputT>
        static void process(const xdict &context, OutputT &output) {
          spdlog::debug(" - for '{}' in '{}'", iter.view(), container.view());
          // TODO optimize this
          xdict loop_context = context;
          auto  cont         = context.at(container.view());
          cont.visit([&]<typename ContainerT>(ContainerT &&actual) {
            if constexpr (one_of<ContainerT, xdict, xlist>) {
              for (auto &&elem : actual) {
                if constexpr (decays_to<ContainerT, xdict>) {
                  loop_context[std::string{iter.view()}] = elem.second;
                } else {
                  static_assert(decays_to<ContainerT, xlist>);
                  loop_context[std::string{iter.view()}] = elem;
                }
                ct::foreach<blocks_t>([&]<typename BlockT>() {
                  spdlog::debug(" - processing: {}", ctti::nameof<BlockT>().str());
                  BlockT::process(loop_context, output);
                });
              }
            } else {
              throw std::runtime_error(fmt::format("no iterable type: {}", ctti::nameof<ContainerT>().str()));
            }
          });
          spdlog::debug(" - endfor '{}' in '{}'", iter.view(), container.view());
        }
      };

      template <size_t index_, typename ImplT>
      struct result_t {
        using block_t                 = ImplT;
        static constexpr size_t index = index_;
      };


      template <ct::string input, typename ImplT, typename toks, size_t index>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"((-?)\s?for\s+(\w+)\s+in\s+(\w+)\s?(-?))">(ImplT::content);
        if constexpr (m) {
          constexpr auto should_trim_before = m.template get<1>().to_view().size() > 0;
          constexpr auto should_trim_after  = m.template get<4>().to_view().size() > 0;
          constexpr auto tmp1      = m.template get<2>().to_view();
          constexpr auto iter      = ct::string<tmp1.size() + 1>::from(tmp1.data());
          constexpr auto tmp2      = m.template get<3>().to_view();
          constexpr auto container = ct::string<tmp2.size() + 1>::from(tmp2.data());
          constexpr auto inner     = generate_blocks_impl<input, toks, ct::tuple<>, index + 1, should_trim_after>();
          using inner_result_t = decltype(inner);
          return result_t<inner.index, impl_t<inner.output, iter, container, should_trim_before, inner_result_t::trim_after>>{};
        }
      }
    };

    struct endfor_control {
      template <ct::string input, typename ImplT>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"((-?)\s?endfor\s?(-?))">(ImplT::content);
        if constexpr (m) {
          constexpr auto should_trim_before = m.template get<1>().to_view().size() > 0;
          constexpr auto should_trim_after  = m.template get<2>().to_view().size() > 0;
          return base_control<should_trim_before, should_trim_after>{};
        }
      }
    };

    template <auto input, typename TokImplT>
    struct render {
      static constexpr auto body =
        trim(std::string_view{input.begin() + TokImplT::start + 2, input.begin() + TokImplT::end - 1});
      static constexpr auto evaluator = evaluator_of<input, TokImplT::start + 2, TokImplT::end - 1>();
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        spdlog::debug(" - processing: '{}'", body);
        fmt::format_to(output, FMT_STRING("{}"), evaluator(context));
      }
    };

    template <auto input, size_t start, size_t end, bool ltrim_, bool rtrim_>
    struct text {
      static constexpr std::string_view trim_chars = "\r\n";
      static constexpr auto body = [] {
        if constexpr (ltrim_ and rtrim_) {
          return trim(std::string_view{input.begin() + start, input.begin() + end}, trim_chars);
        } else if constexpr (ltrim_) {
          return ltrim(std::string_view{input.begin() + start, input.begin() + end}, trim_chars);
        } else if constexpr (rtrim_) {
          return rtrim(std::string_view{input.begin() + start, input.begin() + end}, trim_chars);
        } else {
          return std::string_view{input.begin() + start, input.begin() + end};
        }
      }();
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        spdlog::debug(" - text: '{}' ({})", body, body.size());
        fmt::format_to(output, FMT_STRING("{}"), body);
      }
    };
  }// namespace blocks

  template <typename OutputT, size_t index_, bool trim_after_ = false>
  struct generate_result_t {
    static constexpr size_t index      = index_;
    static constexpr auto   output     = OutputT{};
    static constexpr auto   trim_after = trim_after_;
  };

  template <auto input, ct::ct_tuple toks, ct::ct_tuple output, std::size_t index, bool ltrim, bool rtrim>
  constexpr auto push_text() {
    using tok_t = ct::at_t<index, toks>;
    if constexpr (index == 0) {
      if constexpr (tok_t::start > 0) {
        return ct::push_t<output, blocks::text<input, 0, tok_t::start, ltrim, rtrim>>{};
      } else {
        // no previous text
        return output{};
      }
    } else {
      using prev_tok_t = ct::at_t<index - 1, toks>;
      if constexpr (tok_t::start - prev_tok_t::end > 0) {
        return ct::push_t<output, blocks::text<input, prev_tok_t::end + 1, tok_t::start, ltrim, rtrim>>{};
      } else {
        // no text between tokens
        return output{};
      }
    }
  }

  template <auto input, ct::ct_tuple toks, ct::ct_tuple output, std::size_t index, bool ltrim, bool rtrim>
  using push_text_t = decltype(push_text<input, toks, output, index, ltrim, rtrim>());

  template <ct::string input, ct::ct_tuple toks, ct::ct_tuple output, size_t index, bool ltrim>
  constexpr auto generate_blocks_impl() {
    if constexpr (index < ct::size<toks>) {
      using impl_t  = ct::at_t<index, toks>;
      using token_t = typename impl_t::token_type;
      if constexpr (decays_to<token_t, tokens::render>) {
        return generate_blocks_impl<
          input, toks, ct::push_t<push_text_t<input, toks, output, index, ltrim, false>, blocks::render<input, impl_t>>,
          index + 1>();
      } else if constexpr (decays_to<token_t, tokens::control>) {
        auto load_for    = [&] { return blocks::for_control::try_load<input, impl_t, toks, index>(); };
        auto load_endfor = [&] { return blocks::endfor_control::try_load<input, impl_t>(); };
        if constexpr (not std::is_void_v<decltype(load_for())>) {
          using result_t = decltype(load_for());
          return generate_blocks_impl<
            input, toks,
            ct::push_t<push_text_t<input, toks, output, index, ltrim, result_t::block_t::trim_before>,
                       typename result_t::block_t>,
            result_t::index + 1, result_t::block_t::trim_after>();
        } else if constexpr (not std::is_void_v<decltype(load_endfor())>) {
          using result_t = decltype(load_endfor());
          return generate_result_t<push_text_t<input, toks, output, index, ltrim, result_t::trim_before>, index,
                                   result_t::trim_after>{};
        } else {
          static_assert(always_false_v<impl_t>, "Unhandled block !");
        }
      } else {// comment
        return generate_blocks_impl<input, toks, push_text_t<input, toks, output, index, true, true>, index + 1>();
      }
    } else {
      using prev_t         = ct::at_t<index - 1, toks>;
      using final_output_t = ct::push_t<output, blocks::text<input, prev_t::end + 1, input.size(), ltrim, false>>;
      return generate_result_t<final_output_t, index>{};
    }
  }

  template <ct::string input, ct::ct_tuple TokenTupleT>
  constexpr auto generate_blocks() {
    return generate_blocks_impl<input, TokenTupleT>();
  }
}// namespace xdev::ctt
