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

namespace xdev::ctt {

  template <auto input, ct::ct_tuple toks, ct::ct_tuple output, std::size_t index>
  constexpr auto push_text();

  template <auto input, ct::ct_tuple toks, ct::ct_tuple output, std::size_t index>
  using push_text_t = decltype(push_text<input, toks, output, index>());

  template <ct::string input, ct::ct_tuple toks, ct::ct_tuple output = ct::tuple<>, size_t index = 0>
  constexpr auto generate_blocks_impl();

  namespace blocks {
    struct for_control {
      template <ct::string iter_, ct::string container_>
      struct data_t {
        static constexpr auto iter      = iter_;
        static constexpr auto container = container_;
      };

      template <auto blocks_, auto data_>
      struct impl_t {
        using blocks_t             = decltype(blocks_);
        static constexpr auto data = data_;
        template <typename OutputT>
        static void process(const xdict &context, OutputT &output) {
          fmt::print(" - for '{}' in '{}'\n", data.iter.view(), data.container.view());
          // TODO optimize this
          xdict loop_context = context;
          auto cont = context.at(data.container.view());
          cont.visit([&]<typename ContainerT>(ContainerT &&actual){
            if constexpr (one_of<ContainerT, xdict, xlist>) {
              for (auto &&elem : actual) {
                if constexpr (decays_to<ContainerT, xdict>) {
                  loop_context[std::string{data.iter.view()}] = elem.second;
                } else {
                  static_assert(decays_to<ContainerT, xlist>);
                  loop_context[std::string{data.iter.view()}] = elem;
                }
                ct::foreach<blocks_t>([&]<typename BlockT>() {
                  fmt::print(" - processing: {}\n", ctti::nameof<BlockT>().str());
                  BlockT::process(loop_context, output);
                });
              }
            } else {
              throw std::runtime_error(fmt::format("no iterable type: {}", ctti::nameof<ContainerT>().str()));
            }
          });
          fmt::print(" - endfor '{}' in '{}'\n", data.iter.view(), data.container.view());
        }
      };

      template <size_t index_, typename ImplT>
      struct result_t {
        using block_t                 = ImplT;
        static constexpr size_t index = index_;
      };

      template <ct::string input, typename toks, size_t index_, data_t data>
      static constexpr auto load() {
        // generate inner blocks
        constexpr auto inner = generate_blocks_impl<input, toks, ct::tuple<>, index_>();
        return result_t<inner.index, impl_t<inner.output, data>>{};
      }

      template <ct::string input, typename ImplT, typename toks, size_t index>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"(\s?for\s+(\w+)\s+in\s+(\w+)\s?)">(ImplT::content);
        if constexpr (m) {
          constexpr auto tmp1      = m.template get<1>().to_view();
          constexpr auto iter      = ct::string<tmp1.size() + 1>::from(tmp1.data());
          constexpr auto tmp2      = m.template get<2>().to_view();
          constexpr auto container = ct::string<tmp2.size() + 1>::from(tmp2.data());
          constexpr auto data      = data_t<iter, container>{};
          return load<input, toks, index + 1, data>();
        }
      }
    };

    struct endfor_control {
      template <ct::string input, typename ImplT>
      static constexpr auto try_load() {
        constexpr auto m = ctre::match<R"(\s?endfor\s?)">(ImplT::content);
        if constexpr (m) { return true; }
      }
    };

    template <auto input, typename TokImplT>
    struct render {
      static constexpr auto body =
        trim(std::string_view{input.begin() + TokImplT::start + 2, input.begin() + TokImplT::end - 1});
      static constexpr auto evaluator = evaluator_of<input, TokImplT::start + 2, TokImplT::end - 1>();
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        fmt::print(" - processing: '{}'\n", body);
        output += evaluator(context).toString();
      }
    };

    template <auto input, size_t start, size_t end>
    struct text {
      static constexpr auto body = std::string_view{input.begin() + start, input.begin() + end};
      template <typename OutputT>
      static void process(const xdict &context, OutputT &output) {
        fmt::print(" - text: '{}'\n", body);
        output += body;
      }
    };
  }// namespace blocks

  template <typename OutputT, size_t index_>
  struct generate_result_t {
    static constexpr size_t index  = index_;
    static constexpr auto   output = OutputT{};
  };

  template <auto input, ct::ct_tuple toks, ct::ct_tuple output, std::size_t index>
  constexpr auto push_text() {
    using tok_t = ct::at_t<index, toks>;
    if constexpr (index == 0) {
      if constexpr (tok_t::start > 0) {
        return ct::push_t<output, blocks::text<input, 0, tok_t::start>>{};
      } else {
        // no previous text
        return output{};
      }
    } else {
      using prev_tok_t = ct::at_t<index - 1, toks>;
      if constexpr (tok_t::start - prev_tok_t::end > 1) {
        return ct::push_t<output, blocks::text<input, prev_tok_t::end + 1, tok_t::start - 1>>{};
      } else {
        // no text between tokens
        return output{};
      }
    }
  }

  template <ct::string input, ct::ct_tuple toks, ct::ct_tuple output, size_t index>
  constexpr auto generate_blocks_impl() {
    if constexpr (index < ct::size<toks>) {
      using text_pusher_t = push_text_t<input, toks, output, index>;
      using impl_t        = ct::at_t<index, toks>;
      using token_t       = typename impl_t::token_type;
      if constexpr (decays_to<token_t, tokens::render>) {
        return generate_blocks_impl<input, toks, ct::push_t<text_pusher_t, blocks::render<input, impl_t>>, index + 1>();
      } else if constexpr (decays_to<token_t, tokens::control>) {
        auto load_for    = [&] { return blocks::for_control::try_load<input, impl_t, toks, index>(); };
        auto load_endfor = [&] { return blocks::endfor_control::try_load<input, impl_t>(); };
        if constexpr (not std::is_void_v<decltype(load_for())>) {
          using result_t = decltype(load_for());
          return generate_blocks_impl<input, toks, ct::push_t<text_pusher_t, typename result_t::block_t>,
                                      result_t::index + 1>();
        } else if constexpr (not std::is_void_v<decltype(load_endfor())>) {
          return generate_result_t<text_pusher_t, index>{};
        } else {
          static_assert(always_false_v<impl_t>, "Unhandled block !");
        }
      } else {// comment
        return generate_blocks_impl<input, toks, text_pusher_t, index + 1>();
      }
    } else {
      using prev_t         = ct::at_t<index - 1, toks>;
      using final_output_t = ct::push_t<output, blocks::text<input, prev_t::end + 1, input.size()>>;
      return generate_result_t<final_output_t, index>{};
    }
  }

  template <ct::string input, ct::ct_tuple TokenTupleT>
  constexpr auto generate_blocks() {
    return generate_blocks_impl<input, TokenTupleT>();
  }
}// namespace xdev::ctt
