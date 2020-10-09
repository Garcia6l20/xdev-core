/** @file xdev/ctt/parser.hpp
 *
 */
#pragma once

#include <xdev/ct/string.hpp>
#include <xdev/ct/tuple.hpp>
#include <xdev/ctt/evaluator.hpp>
#include <xdev/variant.hpp>

#include <ctre.hpp>

namespace xdev::ctt {

  template <ct::string input>
  struct parser {

    //
    // tokenizer part
    //

    template <size_t start_pos, size_t end_pos, typename TokenT>
    struct impl {
      /** @brief Actual matched token type
       */
      using token_type              = TokenT;
      static constexpr auto start   = start_pos;
      static constexpr auto end     = end_pos;
      static constexpr auto content = std::string_view{input.begin() + start_pos + 2, input.begin() + end_pos - 1};
    };
    struct control {
      static constexpr std::string_view start_tok = "{%";
      static constexpr std::string_view end_tok   = "%}";
    };

    struct render {
      static constexpr std::string_view start_tok = "{{";
      static constexpr std::string_view end_tok   = "}}";
    };

    struct comment {
      static constexpr std::string_view start_tok = "{#";
      static constexpr std::string_view end_tok   = "#}";
    };

    using tokens_t = ct::tuple<control, render, comment>;

    template <size_t pos>
    static constexpr auto match_start() {
      if constexpr (pos < input.size() - 1) {
        return ct::foreach<tokens_t>([]<typename ElemT>() {
          if constexpr (input[pos] == ElemT::start_tok[0] and input[pos + 1] == ElemT::start_tok[1]) { return pos; }
        });
      }
    }
    template <size_t pos, size_t start>
    static constexpr auto match_end() {
      return ct::foreach<tokens_t>([]<typename ElemT>() {
        if constexpr (input[pos] == ElemT::end_tok[0] and input[pos + 1] == ElemT::end_tok[1]) {
          return impl<start, pos + 1, ElemT>{};
        }
      });
    }

    template <size_t pos>
    static constexpr auto next_start() {
      if constexpr (pos < input.size()) {
        auto match = [] { return match_start<pos>(); };
        if constexpr (std::is_void_v<decltype(match())>) {
          return next_start<pos + 1>();
        } else {
          // matched !
          return match();
        }
      }
    }

    template <size_t pos, size_t start>
    static constexpr auto next_end() {
      if constexpr (pos < input.size()) {
        auto match = [] { return match_end<pos, start>(); };
        if constexpr (std::is_void_v<decltype(match())>) {
          return next_end<pos + 1, start>();
        } else {
          // matched !
          return match();
        }
      }
    }

    template <size_t pos>
    static constexpr auto next_token() {
      if constexpr (pos < input.size() - 2) {
        auto start_pos = [] { return next_start<pos>(); };
        if constexpr (not std::is_void_v<decltype(start_pos())>) {
          // we have a token to find !
          constexpr auto start        = next_start<pos>();
          auto           get_next_end = [] { return next_end<start + 1, start>(); };
          static_assert(not std::is_void_v<decltype(get_next_end())>, "unterminated token");
          auto end = get_next_end();
          return end;
        }
      }
    }

    template <size_t pos = 0, ct::ct_tuple result>
    static constexpr auto parse_tokens_impl() {
      if constexpr (pos < input.size() - 1) {
        auto next   = [] { return next_token<pos>(); };
        using NextT = decltype(next());
        if constexpr (std::is_void_v<NextT>) {
          return result{};
        } else {
          // token available here
          constexpr auto tok = next();
          return parse_tokens_impl<tok.end, ct::push_t<result, decltype(tok)>>();
        }
      } else {
        return result{};
      }
    }

    static constexpr auto parse_tokens() { return parse_tokens_impl<0, ct::tuple<>>(); }

    using token_list = std::decay_t<decltype(parse_tokens())>;

    //
    // blocks part
    //

    template <bool trim_before_, bool trim_after_>
    struct base_control_block {
      static constexpr auto trim_before = trim_before_;
      static constexpr auto trim_after  = trim_after_;
    };

    struct for_control_block {

      template <auto blocks_, ct::string iter_, ct::string container_, bool trim_before_, bool trim_after_>
      struct impl_t : base_control_block<trim_before_, trim_after_> {
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


      template <typename ImplT, typename toks, size_t index>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"((-?)\s?for\s+(\w+)\s+in\s+(\w+)\s?(-?))">(ImplT::content);
        if constexpr (m) {
          constexpr auto should_trim_before = m.template get<1>().to_view().size() > 0;
          constexpr auto should_trim_after  = m.template get<4>().to_view().size() > 0;
          static_assert(not should_trim_before);
          static_assert(not should_trim_after);
          constexpr auto tmp1      = m.template get<2>().to_view();
          constexpr auto iter      = ct::string<tmp1.size() + 1>::from(tmp1.data());
          constexpr auto tmp2      = m.template get<3>().to_view();
          constexpr auto container = ct::string<tmp2.size() + 1>::from(tmp2.data());
          constexpr auto inner     = generate_blocks_impl<toks, ct::tuple<>, index + 1, should_trim_after>();
          return result_t<inner.index, impl_t<inner.output, iter, container, should_trim_before, should_trim_after>>{};
        }
      }
    };

    struct endfor_control_block {
      template <typename ImplT>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"((-?)\s?endfor\s?(-?))">(ImplT::content);
        if constexpr (m) {
          constexpr auto should_trim_before = m.template get<1>().to_view().size() > 0;
          constexpr auto should_trim_after  = m.template get<2>().to_view().size() > 0;
          return base_control_block<should_trim_before, should_trim_after>{};
        }
      }
    };

    template <typename TokImplT>
    struct render_block {
      static constexpr auto body =
        trim(std::string_view{input.begin() + TokImplT::start + 2, input.begin() + TokImplT::end - 1});
      static constexpr auto evaluator = evaluator_of<input, TokImplT::start + 2, TokImplT::end - 1>();
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        spdlog::debug(" - processing: '{}'", body);
        output += evaluator(context).toString();
      }
    };

    template <size_t start, size_t end, bool ltrim_, bool rtrim_>
    struct text_block {
      static constexpr auto body = [] {
        if constexpr (ltrim_ and rtrim_) {
          return trim(std::string_view{input.begin() + start, input.begin() + end});
        } else if constexpr (ltrim_) {
          return ltrim(std::string_view{input.begin() + start, input.begin() + end});
        } else if constexpr (rtrim_) {
          return rtrim(std::string_view{input.begin() + start, input.begin() + end});
        } else {
          return std::string_view{input.begin() + start, input.begin() + end};
        }
      }();
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        spdlog::debug(" - text: '{}'", body);
        output += body;
      }
    };

    //
    // generator part
    //

    template <typename OutputT, size_t index_, bool trim_after_ = false>
    struct generate_result_t {
      static constexpr size_t index      = index_;
      static constexpr auto   output     = OutputT{};
      static constexpr auto   trim_after = trim_after_;
    };

    template <ct::ct_tuple toks, ct::ct_tuple output, std::size_t index, bool ltrim, bool rtrim>
    static constexpr auto push_text() {
      using tok_t = ct::at_t<index, toks>;
      if constexpr (index == 0) {
        if constexpr (tok_t::start > 0) {
          return ct::push_t<output, text_block<0, tok_t::start, ltrim, rtrim>>{};
        } else {
          // no previous text
          return output{};
        }
      } else {
        using prev_tok_t = ct::at_t<index - 1, toks>;
        if constexpr (tok_t::start - prev_tok_t::end > 1) {
          return ct::push_t<output, text_block<prev_tok_t::end + 1, tok_t::start - 1, ltrim, rtrim>>{};
        } else {
          // no text between tokens
          return output{};
        }
      }
    }

    template <ct::ct_tuple toks, ct::ct_tuple output, std::size_t index, bool ltrim, bool rtrim>
    using push_text_t = decltype(push_text<toks, output, index, ltrim, rtrim>());

    template <ct::ct_tuple toks, ct::ct_tuple output, size_t index, bool ltrim>
    static constexpr auto generate_blocks_impl() {
      if constexpr (index < ct::size<toks>) {
        using impl_t  = ct::at_t<index, toks>;
        using token_t = typename impl_t::token_type;
        if constexpr (decays_to<token_t, render>) {
          return generate_blocks_impl<
            toks, ct::push_t<push_text_t<toks, output, index, ltrim, false>, render_block<impl_t>>, index + 1>();
        } else if constexpr (decays_to<token_t, control>) {
          constexpr auto load_for    = [&] { return typename for_control_block::template try_load<impl_t, toks, index>(); };
          auto load_endfor = [&] { return typename endfor_control_block::template try_load<impl_t>(); };
          if constexpr (not std::is_void_v<decltype(load_for())>) {
            using result_t = decltype(load_for());
            return generate_blocks_impl<
              toks,
              ct::push_t<push_text_t<toks, output, index, ltrim, result_t::block_t::trim_before>,
                         typename result_t::block_t>,
              result_t::index + 1, result_t::block_t::trim_after>();
          } else if constexpr (not std::is_void_v<decltype(load_endfor())>) {
            using result_t = decltype(load_endfor());
            return generate_result_t<push_text_t<toks, output, index, ltrim, result_t::trim_before>, index,
                                     result_t::trim_after>{};
          } else {
            static_assert(always_false_v<impl_t>, "Unhandled block !");
          }
        } else {// comment
          return generate_blocks_impl<toks, push_text_t<toks, output, index, true, true>, index + 1>();
        }
      } else {
        using prev_t         = ct::at_t<index - 1, toks>;
        using final_output_t = ct::push_t<output, text_block<prev_t::end + 1, input.size(), ltrim, false>>;
        return generate_result_t<final_output_t, index>{};
      }
    }

    static constexpr auto generate_blocks() { return generate_blocks_impl<token_list, ct::tuple<>, 0, false>(); }
    using block_list = std::decay_t<decltype(generate_blocks())>;
  };
}// namespace xdev::ctt
