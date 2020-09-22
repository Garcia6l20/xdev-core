#include <xdev/concurrent.hpp>
#include <xdev/tools.hpp>
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include <future>

using namespace xdev;
using namespace std;

using concurrent::channel;

TEST_CASE("Concurrent.BasicChannel") {
    channel<int> to_thread;
    auto fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
            spdlog::info("Value: {}", value);
        } while (value != -1);
    });
    to_thread.push(1);
    to_thread.push(2);
    to_thread.push(3);
    to_thread.push(4);
    to_thread.push(-1);
    fut.get();
}

TEST_CASE("Concurrent.ProducerCusommer") {
    channel<int> to_thread;
    channel<int> from_thread;
    auto echo_fut = async([&rx = to_thread,
                                &tx = from_thread] {
        int value;
        tx.push(0);
        do {
            value = rx.pop();
            tx.push(value);
        } while (value != -1);
    });
    to_thread.push(1);
    auto val = from_thread.pop();
    if(val == 0) {
        spdlog::info("Thread started first");
    } else if (val == 1) {
        spdlog::info("Thread not started first");
    } else {
        FAIL("Hummm... realy strange !!");
    }
    auto consumer_fut = async([&rx = from_thread] {
        int value;
        do {
            value = rx.pop();
            spdlog::info("Value: {}", value);
        } while (value != -1);
    });
    to_thread.push(2);
    to_thread.push(-1);
    echo_fut.get();
    consumer_fut.get();
}

using concurrent::sync_channel;

TEST_CASE("Concurrent.SyncChannel") {
    sync_channel<int> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
            spdlog::info("Value: {}", value);
        } while (value != -1);
    });
    REQUIRE(to_thread.push(42).get() == 0);
    REQUIRE(to_thread.push(-1).get() == 0);
    consumer_fut.get();
}

TEST_CASE("Concurrent.SyncChannelDefaultReturn") {
    sync_channel<int, int, 55> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
        } while (value != -1);
    });
    REQUIRE(to_thread.push(42).get() == 55);
    REQUIRE(to_thread.push(-1).get() == 55);
    consumer_fut.get();
}

TEST_CASE("Concurrent.SyncChannelBoolReturn") {
    sync_channel<int, bool> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            promise<bool> ret;
            value = rx.pop(ret);
            ret.set_value(value != -1 ? true : false);
        } while (value != -1);
    });
    REQUIRE(to_thread.push(42).get() == true);
    REQUIRE(to_thread.push(-1).get() == false);
    consumer_fut.get();
}

using concurrent::subscription_channel;
using concurrent::scoped_subscription;

TEST_CASE("Concurrent.Publisher") {
    int events_received = 0;
    tools::finally _{[&events_received]{
        REQUIRE(events_received == 4);
    }};
    subscription_channel<int> to_thread;
    auto sub1 = async([&rx = to_thread, &events_received] {
        scoped_subscription sub(rx);
        int value;
        do {
            value = rx.pop();
            spdlog::info("event#{}: {}", ++events_received, value);
        } while (value != -1);
    });
    auto sub2 = async([&rx = to_thread, &events_received] {
        scoped_subscription sub(rx);
        int value;
        do {
            value = rx.pop();
            spdlog::info("event#{}: {}", ++events_received, value);
        } while (value != -1);
    });
    this_thread::sleep_for(10ms);
    to_thread.push(42);
    to_thread.push(-1);
    sub1.get();
    sub2.get();
}
