#include <xdev/xdev-concurrent.hpp>
#include <gtest/gtest.h>

#include <future>

using namespace xdev;
using namespace std;

using concurrent::channel;

TEST(Concurrent, BasicChannel) {
    channel<int> to_thread;
    auto fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
            cout << "Value: " << to_string(value) << endl;
        } while (value != -1);
    });
    to_thread.push(1);
    to_thread.push(2);
    to_thread.push(3);
    to_thread.push(4);
    to_thread.push(-1);
    fut.get();
}

TEST(Concurrent, ProducerCusommer) {
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
        cout << "Thread started first" << endl;
    } else if (val == 1) {
        cout << "Thread not started first" << endl;
    } else {
        FAIL() << "Hummm... realy strange !!";
    }
    auto consumer_fut = async([&rx = from_thread] {
        int value;
        do {
            value = rx.pop();
            cout << "Value: " << to_string(value) << endl;
        } while (value != -1);
    });
    to_thread.push(2);
    to_thread.push(-1);
    echo_fut.get();
    consumer_fut.get();
}

using concurrent::sync_channel;

TEST(Concurrent, SyncChannel) {
    sync_channel<int> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
            cout << "Value: " << to_string(value) << endl;
        } while (value != -1);
    });
    ASSERT_EQ(to_thread.push(42).get(), 0);
    ASSERT_EQ(to_thread.push(-1).get(), 0);
    consumer_fut.get();
}

TEST(Concurrent, SyncChannelDefaultReturn) {
    sync_channel<int, int, 55> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            value = rx.pop();
        } while (value != -1);
    });
    ASSERT_EQ(to_thread.push(42).get(), 55);
    ASSERT_EQ(to_thread.push(-1).get(), 55);
    consumer_fut.get();
}

TEST(Concurrent, SyncChannelBoolReturn) {
    sync_channel<int, bool> to_thread;
    auto consumer_fut = async([&rx = to_thread] {
        int value;
        do {
            promise<bool> ret;
            value = rx.pop(ret);
            ret.set_value(value != -1 ? true : false);
        } while (value != -1);
    });
    ASSERT_EQ(to_thread.push(42).get(), true);
    ASSERT_EQ(to_thread.push(-1).get(), false);
    consumer_fut.get();
}

using concurrent::subscription_channel;
using concurrent::scoped_subscription;

TEST(Concurrent, Publisher) {
    subscription_channel<int> to_thread;
    auto sub1 = async([&rx = to_thread] {
        scoped_subscription sub(rx);
        int value;
        do {
            value = rx.pop();
        } while (value != -1);
    });
    auto sub2 = async([&rx = to_thread] {
        scoped_subscription sub(rx);
        int value;
        do {
            value = rx.pop();
        } while (value != -1);
    });
    this_thread::sleep_for(10ms);
    to_thread.push(42);
    to_thread.push(-1);
    sub1.get();
    sub2.get();
}
