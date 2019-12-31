//
// Created by alex on 11/1/19.
//

#ifndef SPHERE_VIS_RINGBUFFER_H
#define SPHERE_VIS_RINGBUFFER_H

#include <optional>
#include <atomic>

template<typename T, std::size_t N>
class RingBuffer
{
    static constexpr std::size_t MOD = N + 1;
public:
    RingBuffer(): back(0), front(0) {}
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;

    void push(const T& t)
    {
        auto p = back.load();

        new (data + p) T(t);

        p = (p + 1) % MOD;
        back.store(p);
    }

    void push(T&& t)
    {
        auto p = back.load();

        new (data + p) T(std::move(t));

        p = (p + 1) % MOD;
        back.store(p);
    }

    std::optional<T> pop()
    {
        if(empty())
            return std::nullopt;

        return raw_pop();
    }

    T raw_pop()
    {
        auto p = front.load();
        T *e = reinterpret_cast<T*>(data + p);
        T t = *e;
        e->~T();

        p = (p + 1) % MOD;
        front.store(p);
        return t;
    }

    [[nodiscard]]
    std::size_t size() const
    {
        auto b = back.load(), f = front.load();

        if(f <= b)
            return b - f;
        else return MOD - f + b;
    }

    [[nodiscard]]
    bool empty() const
    {
        return front.load() == back.load();
    }

    [[nodiscard]]
    bool full() const
    {
        return (back.load() + 1) % MOD == front.load();
    }

    T operator[](std::size_t i)
    {
        std::size_t f = front;
        return *reinterpret_cast<T*>(data + ((f + i) % MOD));
    }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data[N + 1];

    std::atomic<std::size_t> front, back;
};

#endif //SPHERE_VIS_RINGBUFFER_H
