#pragma once

#include <utility>

namespace xdev {

  namespace detail {
    template <typename LambdaT>
    class final_lambda {
    public:
      final_lambda(LambdaT &&func)
          : func_(std::forward<LambdaT>(func))
          , cancelled_(false) {}

      final_lambda(const final_lambda &other) = delete;

      final_lambda(final_lambda &&other)
          : func_(std::forward<LambdaT>(other.func_))
          , cancelled_(other.cancelled_) {
        other.cancel();
      }

      ~final_lambda() {
        if (!cancelled_) { func_(); }
      }

      void cancel() { cancelled_ = true; }

      void call_now() {
        cancelled_ = true;
        func_();
      }

    private:
      LambdaT func_;
      bool cancelled_;
    };

    template <typename LambdaT, bool CALL_ON_FAILURE>
    class conditional_final_lambda {
    public:
      conditional_final_lambda(LambdaT &&func)
          : func_(std::forward<LambdaT>(func))
          , uncaught_exception_count_(std::uncaught_exceptions())
          , cancelled_(false) {}

      conditional_final_lambda(const conditional_final_lambda &other) = delete;

      conditional_final_lambda(conditional_final_lambda &&other) noexcept(
        std::is_nothrow_move_constructible<LambdaT>::value)
          : func_(std::forward<LambdaT>(other.func_))
          , uncaught_exception_count_(other.uncaught_exception_count_)
          , cancelled_(other.cancelled_) {
        other.cancel();
      }

      ~conditional_final_lambda() noexcept(CALL_ON_FAILURE || noexcept(std::declval<LambdaT>()())) {
        if (!cancelled_ && (is_unwinding_due_to_exception() == CALL_ON_FAILURE)) { func_(); }
      }

      void cancel() noexcept { cancelled_ = true; }

    private:
      bool is_unwinding_due_to_exception() const noexcept {
        return std::uncaught_exceptions() > uncaught_exception_count_;
      }

      LambdaT func_;
      int  uncaught_exception_count_;
      bool cancelled_;
    };
  }// namespace detail

  template <typename LambdaT>
  auto finally(LambdaT &&func) {
    return detail::final_lambda<LambdaT>{std::forward<LambdaT>(func)};
  }

  template <typename LambdaT>
  auto failure_finally(LambdaT &&func) {
    return detail::conditional_final_lambda<LambdaT, true>{std::forward<LambdaT>(func)};
  }

  template <typename LambdaT>
  auto success_finally(LambdaT &&func) {
    return detail::conditional_final_lambda<LambdaT, false>{std::forward<LambdaT>(func)};
  }

}// namespace xdev
