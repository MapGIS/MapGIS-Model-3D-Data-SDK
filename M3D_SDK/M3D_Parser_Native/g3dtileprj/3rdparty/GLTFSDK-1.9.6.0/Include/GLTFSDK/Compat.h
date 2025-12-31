#pragma once
#include <memory>
#include <type_traits> // 确保包含必要的类型特性头文件
#if __cplusplus == 201103L 
namespace std {

// 非数组版本的make_unique
template<typename T, typename... Args>
typename enable_if<!is_array<T>::value, unique_ptr<T>>::type
make_unique(Args&&... args) {
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

// 处理无边界数组的特化版本
template<typename T>
typename enable_if<is_array<T>::value && extent<T>::value == 0, unique_ptr<T>>::type
make_unique(size_t size) {
    using U = typename remove_extent<T>::type;
    return unique_ptr<T>(new U[size]);
}

// 禁止已知边界数组的实例化（如make_unique<char[5]>()）
template<typename T, typename... Args>
typename enable_if<is_array<T>::value && extent<T>::value != 0, unique_ptr<T>>::type
make_unique(Args&&...) = delete;

// 其他using声明（保持原样）
template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

template<bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

} // namespace std
#endif
