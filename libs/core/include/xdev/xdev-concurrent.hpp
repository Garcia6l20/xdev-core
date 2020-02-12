/**
 * @file xdev-channel.hpp
 * @author Garcia Sylvain <garcia.6l20@gmail.com>
 *
 *
 **/
#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>
#include <algorithm>

#ifdef __cpp_lib_concepts
#include <concepts>
#else
#include <xdev/std-concepts.hpp>
#endif

// #define XDEV_DEBUG_CONCURRENT
#ifdef XDEV_DEBUG_CONCURRENT
#include <iostream>
#define CONCURRENT_DEBUG(...) \
    std::cout << __VA_ARGS__
#else
#define CONCURRENT_DEBUG(...)
#endif

namespace xdev::concurrent {
namespace internal {

template <typename T, typename U>
concept Channel = requires(T ch, U u) {
    {ch.pop()} -> std::same_as<U>;
    {ch.pop()} -> std::convertible_to<U>;
    {ch.push(u)};
};

template <typename T>
concept SubscriptionChannel = requires(T ch) {
    {ch.subscribe()};
    {ch.unsubscribe()};
};

template <typename T, typename mutex_type = std::mutex>
struct _channel {
    _channel() = default;
    _channel(const _channel&) = delete;
    _channel& operator=(const _channel&) = delete;
    void push(const T& value) {
        std::scoped_lock lock(_mux);
        _queue.push_back(value);
        _read_cond.notify_all();
    }
    void push(T&& value) {
        std::scoped_lock lock(_mux);
        _queue.push_back(std::forward<T>(value));
        _read_cond.notify_all();
    }
    T pop() {
        std::unique_lock lock(_mux);
        while (_queue.empty()) {
            _read_cond.wait(lock);
        }
        T item = std::move(_queue.front());
        _queue.pop_front();
        return item;
    }
    void drain() {
        std::scoped_lock lock(_mux);
        _queue.clear();
    }
protected:
    mutex_type _mux;
    std::condition_variable_any _read_cond;
    std::deque<T> _queue;
};

template <typename T, typename RetT = T, RetT def_return = RetT{}>
struct _sync_channel: _channel<std::pair<T, std::promise<RetT>>> {
    using ItemT = std::pair<T, std::promise<RetT>>;
    _sync_channel() {}
    _sync_channel(const _sync_channel&) = delete;
    _sync_channel& operator=(const _sync_channel&) = delete;
    std::future<RetT> push(const T& value) {
        std::promise<RetT> promise;
        std::future<RetT> fut = promise.get_future();
        _channel<ItemT>::push({value, std::move(promise)});
        return fut;
    }
    std::future<RetT> push(T&& value) {
        std::promise<RetT> promise;
        std::future<RetT> fut = promise.get_future();
        _channel<ItemT>::push({std::forward<T>(value), std::move(promise)});
        return fut;
    }
    T pop() {
        ItemT&& item = _channel<ItemT>::pop();
        auto promise = std::move(item.second);
        promise.set_value(def_return);
        return std::move(item.first);
    }
    T pop(std::promise<RetT>& promise) {
        ItemT&& item = _channel<ItemT>::pop();
        promise = std::move(item.second);
        return std::move(item.first);
    }
};


template <typename T>
struct _subs_channel: _channel<std::pair<T, std::vector<std::thread::id>>, std::recursive_mutex> {
    using ItemT = std::pair<T, std::vector<std::thread::id>>;
    _subs_channel() = default;
    ~_subs_channel() = default;
    _subs_channel(const _subs_channel&) = delete;
    _subs_channel& operator=(const _subs_channel&) = delete;
    void push(const T& value) {
        std::scoped_lock lock(this->_mux);
        _channel<ItemT, std::recursive_mutex>::push({value, _subs});
    }
    void push(T&& value) {
        std::scoped_lock lock(this->_mux);
        _channel<ItemT, std::recursive_mutex>::push({std::forward<T>(value), _subs});
        CONCURRENT_DEBUG("pushed " << this->_queue.back().first << " for " << _subs.size() << " subscribers" << std::endl);
    }
    T pop() {
        auto tid = std::this_thread::get_id();
        std::unique_lock lock(this->_mux);
        while (this->_queue.empty()) {
            this->_read_cond.wait(lock);
        }

        ItemT& item = this->_queue.front();
        while (std::find(item.second.begin(), item.second.end(), tid) == item.second.end()) {
            // we need to set a timeout cause the value might not be already pushed
            // but condition might be trigged by another value...
            this->_read_cond.wait_for(lock, std::chrono::milliseconds(1));
            item = this->_queue.front();
        }
        auto& subs = item.second;
        CONCURRENT_DEBUG(tid << " " <<  item.first << " subscribers: " << subs.size());
        subs.erase(std::remove(subs.begin(), subs.end(), tid), subs.end());
        CONCURRENT_DEBUG(" then " << subs.size() << std::endl);
        if (subs.empty()) {
            CONCURRENT_DEBUG(tid << " erasing " <<  item.first << " queue" << std::endl);
            T ret = std::move(item.first);
            this->_queue.erase(std::remove(this->_queue.begin(), this->_queue.end(), item), this->_queue.end());
            return std::move(ret);
        }
        return item.first;
    }
    void subscribe() {
        auto tid = std::this_thread::get_id();
        std::scoped_lock lock(this->_mux);
        _subs.push_back(tid);
        for (auto& [_, subs]: this->_queue) {
            subs.push_back(tid);
        }
        CONCURRENT_DEBUG(tid << " subscribed" << std::endl);
    }
    void unsubscribe() {
        auto tid = std::this_thread::get_id();
        std::scoped_lock lock(this->_mux);
        CONCURRENT_DEBUG(tid << " unsubscribing..." << std::endl);
        _subs.erase(std::remove(_subs.begin(), _subs.end(), tid));
        for (auto ii = this->_queue.begin(); ii < this->_queue.end(); ++ii) {
            auto& subs = ii->second;
            CONCURRENT_DEBUG(tid << " " <<  ii->first << " subscribers: " << subs.size() << std::flush);
            subs.erase(std::remove(subs.begin(), subs.end(), tid), subs.end());
            CONCURRENT_DEBUG(" then " << subs.size() << " erased" << std::endl);
            if (subs.empty()) {
                CONCURRENT_DEBUG(tid << " queue " << ii->first << " erased" << std::endl);
                this->_queue.erase(ii--);
            }
        }
    }
    std::vector<std::thread::id> _subs;
};

} // namespace internal

/**
 * Channel pattern implementation for cross threading messaging
 */
template <typename T, internal::Channel<T> ChannelT = internal::_channel<T>>
struct channel {
    channel(): _p(std::make_shared<ChannelT>()) {}
    channel(const channel& other):
        _p(other._p) {
    }
    channel& operator=(const channel& other) {
        this->_p = other;
        return *this;
    }
    channel(channel&& other):
        _p(std::move(other._p)) {
    }
    channel& operator=(channel&& other) {
        this->_p = std::move(other._p);
        return *this;
    }
    auto push(const T& value) {
        return _p->push(value);
    }
    auto push(T&& value) {
        return _p->push(std::forward<T>(value));
    }
    template <typename...Args>
    T pop(Args&&...args) {
        return std::move(_p->pop(std::forward<Args>(args)...));
    }
    void drain() {
        _p->drain();
    }
    void subscribe() requires internal::SubscriptionChannel<ChannelT> {
        _p->subscribe();
    }
    void unsubscribe() requires internal::SubscriptionChannel<ChannelT> {
        _p->unsubscribe();
    }
private:
    std::shared_ptr<ChannelT> _p;
};

template <typename T, typename RetT = T, RetT default_ret = RetT{}>
using sync_channel = channel<T, internal::_sync_channel<T, RetT, default_ret>>;

template <typename T>
using subscription_channel = channel<T, internal::_subs_channel<T>>;

template <typename T>
struct scoped_subscription {
    scoped_subscription(subscription_channel<T>& ch):
        _ch(ch) {
        _ch.subscribe();
    }
    ~scoped_subscription() {
        _ch.unsubscribe();
    }
private:
    subscription_channel<T>& _ch;
};

} // namespace xdev::concurrent
