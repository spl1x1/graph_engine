#ifndef VECROR_HPP
#define VECROR_HPP

#include <raylib.h>
#include <cstddef>
#include <utility>
#include <type_traits>

template <typename T, std::size_t N>
class Vec {
    T data[N]{};

template <std::size_t... I, typename... Args>
    void init(std::index_sequence<I...>, Args&&... args) {
        ((data[I] = static_cast<T>(std::forward<Args>(args))), ...);
    }

public:
    // Construct from exactly N arguments: Vec<float,2> v(1.f, 2.f);
    template <typename... Args,typename = std::enable_if_t<sizeof...(Args) == N>>
    explicit Vec(Args&&... args) {
        init(std::index_sequence_for<Args...>{}, std::forward<Args>(args)...);
    }

    Vec() = default;
    Vec(const Vec& other) = default;
    Vec(Vec&& other) = default;


    Vec floor() const {
        Vec result;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] = std::floor(data[i]);
        }
        return result;
    }

    Vec ceil() const {
        Vec result;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] = std::ceil(data[i]);
        }
        return result;
    }

    // Access operators
    T &operator[](std::size_t index) {
        return data[index];
    }
    const T& operator[](std::size_t index) const { return data[index];}

    //Operator overloads for basic arithmetic
    Vec operator+(const Vec& other) const{
        Vec result = *this;        for (std::size_t i = 0; i < N; ++i) {
            result[i] += other.data[i];
        }
        return result;
    };

    Vec operator-(const Vec& other) const{
        Vec result = *this;        for (std::size_t i = 0; i < N; ++i) {
            result[i] -= other.data[i];
        }
        return result;
    };
    Vec& operator+=(const Vec& other){
        return *this = *this + other;
    }
    Vec& operator-=(const Vec& other){
        return *this = *this - other;
    };

    T operator*(const Vec& other) const{
        T result = T{};
        for (std::size_t i = 0; i < N; ++i) {
            result += data[i] * other.data[i];
        }
        return result;
    };

    Vec operator+(T scalar) const{
        Vec result = *this;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] += scalar;
        }
        return result;
    };
    Vec operator-(T scalar) const {
        Vec result = *this;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] -= scalar;
        }
        return result;
    };
    Vec& operator+=(T scalar) {
        return *this = *this + scalar;
    };
    Vec& operator-=(T scalar) {
        return *this = *this - scalar;
    };

    Vec operator*(T scalar) const {
        Vec result = *this;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] *= scalar;
        }
        return result;
    };
    Vec operator/(T scalar) const {
        Vec result = *this;
        for (std::size_t i = 0; i < N; ++i) {
            result[i] /= scalar;
        }
        return result;
    };
    Vec& operator*=(T scalar) {
        return *this = *this * scalar;
    };
    Vec& operator/=(T scalar) {
        return *this = *this / scalar;
    };
};
#endif // VECROR_HPP
