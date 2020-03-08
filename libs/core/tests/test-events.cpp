#include <xdev/xdev-object.hpp>
#include <catch2/catch.hpp>

#include <any>
#include <queue>
#include <memory>
#include <functional>
#include <list>

namespace channels {

    class Subscription {
    public:
        Subscription() = default;
        Subscription(std::string channel, size_t id): _channel{channel}, _id{id} {}
        ~Subscription();

        Subscription(Subscription&& other): _channel{std::move(other._channel)}, _id{other._id} {}
        Subscription& operator=(Subscription&& other) {
            _channel = std::move(other._channel);
            _id = other._id;
            return *this;
        }

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        const std::string& channel() const { return _channel; }
        size_t id() const { return _id; }
    private:
        std::string _channel = "";
        size_t _id = 0;
    };

    struct BaseListenerPool {};
    using BaseListenerPoolPtr = std::unique_ptr<BaseListenerPool>;
    template <typename T>
    struct ListenerPool: BaseListenerPool {
        using callback_type = std::function<void(T)>;
        std::list<callback_type> _listeners;
        void operator()(const T& data) {
            for (auto& listener : _listeners) {
                listener(data);
            }
        }
        template <typename...ArgsT>
        constexpr size_t emplace_back(ArgsT&&... args) {
            return reinterpret_cast<size_t>(&_listeners.emplace_back(std::forward<ArgsT>(args)...));
        }

        constexpr void remove(size_t id) {
            _listeners.remove_if([id](const auto& listener) {
                return reinterpret_cast<size_t>(&listener) == id;
            });
        }
        constexpr size_t count() const {
            return _listeners.size();
        }
    };

    static std::map<std::string, BaseListenerPoolPtr> g_pools;

    template <typename T = void*>
    auto& pool(const std::string& channel) {
        if (g_pools.count(channel) == 0) {
            g_pools.emplace(channel, std::make_unique<ListenerPool<T>>());
        }
        return *static_cast<ListenerPool<T>*>(g_pools[channel].get());
    }

    template <typename T>
    [[nodiscard]] constexpr Subscription subscribe(const std::string& channel, typename ListenerPool<T>::callback_type&& cb) {
        return {
            channel,
            pool<T>(channel).emplace_back(std::forward<typename ListenerPool<T>::callback_type>(cb))
        };
    }

    void unsubscribe(const Subscription& sub) {
        auto& p = pool(sub.channel());
        p.remove(sub.id());
        if (p.count() == 0) {
            g_pools.erase(sub.channel());
        }
    }

    Subscription::~Subscription() {
        if (!_channel.empty())
            unsubscribe(*this);
    }

    template <typename T>
    void post(std::string channel, T data) {
        pool<T>(channel)(data);
    }
}

struct TestData {

};

TEST_CASE("Channels, Nominal") {
    bool received = false;
    auto sub = channels::subscribe<TestData>("test", [&received](const auto&/*data*/) {
        received = true;
    });
    channels::post<TestData>("test", {});
    REQUIRE(received);
}

TEST_CASE("Channels.Unsubscibe") {
    int received_counter = 0;
    auto test_handler = [&received_counter](const auto& /*data*/) {
        ++received_counter;
    };
    auto sub = channels::subscribe<TestData>("test", test_handler);
    channels::post<TestData>("test", {});
    REQUIRE(received_counter == 1);

    {
        auto sub2 = channels::subscribe<TestData>("test", test_handler);
        channels::post<TestData>("test", {});
        REQUIRE(received_counter == 3);

        channels::unsubscribe(sub);
        channels::post<TestData>("test", {});
        REQUIRE(received_counter == 4);
    } // sub2 has unsubscribed !
    channels::post<TestData>("test", {});
    REQUIRE(received_counter == 4);
}

using namespace xdev;

TEST_CASE("Events.FunctionWrap") {
    int val = 42;
    bool called = false;
    // note: lambdas must be typed as function
    function lambda = [&](int value) {
        REQUIRE(val == value);
        called = true;
    };
    xfn test = lambda;
    test(val);
    REQUIRE(called);
}

//TEST_CASE("Events.FunctionArrayArg") {
//    XArray val = {42};
//    bool called = false;
//    // note: lambdas must be typed as function
//    xfn test = function([&](const XArray& value) {
//        for (size_t ii = 0 ; ii < value.size(); ++ii) {
//            ASSERT_EQ(val[ii], value[ii]);
//        }
//        called = true;
//    });
//    test(val);
//    REQUIRE(called);
//}

TEST_CASE("Properties.BadConnect") {
    event<int> intEvt;
    int val = 42;
    bool called = false;
    // note: lambdas must be typed as function
    function lambda = [&](int value) {
        REQUIRE(val == value);
        called = true;
    };
    intEvt.connect(lambda); // connection not hold so not called
    intEvt(42);
    REQUIRE_FALSE(called);
}

TEST_CASE("Properties.GoodConnect") {
    event<int> intEvt;
    int val = 42;
    bool called = false;
    // note: lambdas must be typed as function
    auto conn = intEvt.connect(function([&](int value) {
        REQUIRE(val == value);
        called = true;
    }));
    REQUIRE_FALSE(called);
    intEvt(42);
    REQUIRE(called);
}

TEST_CASE("Properties.ScopeConnect") {
    event<int> intEvt;
    int val = 42;
    int call_cnt = 0;
    {
        // note: lambdas must be typed as function
        auto conn = intEvt.connect(function([&](int value) {
            REQUIRE(val == value);
            ++call_cnt;
        }));
        intEvt(42);
    }
    intEvt(42);
    REQUIRE(call_cnt == 1);
}
