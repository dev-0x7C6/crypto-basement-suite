#pragma once

#include <future>
#include <optional>
#include <thread>

template <typename T>
struct task {
    std::future<T> future;
    std::jthread thread;

    auto get() {
        if (value)
            return value.value();

        future.wait();
        value = future.get();
        return value.value();
    };

    std::optional<T> value;
};

template <typename T>
auto schedule(std::function<T()> &&callable) -> task<T> {
    std::packaged_task task([callable{std::move(callable)}]() -> T {
        return callable();
    });

    return {
        .future = task.get_future(),
        .thread = std::jthread(std::move(task)),
    };
}
