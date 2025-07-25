/*
* mp_count、mp_plus、mp_count_if等mp系列元编程原语实现
*
*
*/

#include <iostream>
#include <tuple>
#include <utility>
#include <format>
#include <type_traits>
#include <cxxabi.h>
#include <string>

std::string demangle(const char* name) {
    int status = -1;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result(status == 0 ? demangled : name);
    free(demangled);
    return result;
}



/*
* mp 元编程系列原语--mp_plus
* 功能：累加功能，将传递的多个类型的值累加起来，返回一个新的类型（std::integral_constant）
* 例如：mp_plus<std::integral_constant<bool, true>, std::integral_constant<bool, true>>  返回 std::integral_constant<unsigned long, 2ul>
* 注意：在元编程中常量值也应该处理成类型（使用std::integral_constant），mp_plus可以作为mp_count的底层实现
*
*/
//主模板，不给实现
template<class... T>
struct mp_plus_impl;

//别名模板，简化使用
template<class... T>
using mp_plus = mp_plus_impl<T...>::type;

//模板偏特化
template<>
struct mp_plus_impl<>
{
    using type = std::integral_constant<std::size_t, 0>;    //常量值也是类型（std::integral_constant）
};

//模板偏特化
template<class T1, class... T>
struct mp_plus_impl<T1, T...>
{
    static constexpr auto _v = T1::value + mp_plus<T...>::value;   // 这里使用mp_plus，而不是mp_plus_impl

    using type = std::integral_constant<typename std::remove_const_t<decltype(_v)>, _v>;
};

//模板偏特化
//这个模板是用来优化加速的，一次性处理10个类型，减少实例化的模板数量
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class... T>
    struct mp_plus_impl<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T...>
{
    static constexpr auto _v = T1::value + T2::value + T3::value + T4::value +
        T5::value + T6::value + T7::value + T8::value + T9::value + T10::value +
        mp_plus<T...>::value;

    using type = std::integral_constant<
        typename std::remove_const<decltype(_v)>::type, _v>;
};


/*
* mp 元编程系列原语--mp_count
* 功能：判断一个类型列表L<T...>中类型V出现的次数
* 例如：mp_count<std::tuple<int,int,char,double>, int> 返回 std::integral_constant<unsigned long, 2ul>
* 注意：mp_count借助mp_plus实现
*
*/
//主模板
template<class L, class V>
struct mp_count_impl;

//别名模板，简化使用
template<class L, class V>
using mp_count = mp_count_impl<L, V>::type;

//模板偏特化
template<template<class...> class L, class... T, class V>
struct mp_count_impl<L<T...>, V>
{
    using type = mp_plus<std::is_same<T, V>...>;
};


/*
* mp 元编程系列原语--mp_count_if
* 功能：mp_count_if<L, P> 计算类型列表L中P<T>为true的类型的数量
* 例如：mp_count_if<std::tuple<int,int,char,double>, std::is_integral> 返回 std::integral_constant<unsigned long, 3ul>
* 注意：mp_count_if借助mp_plus实现
*
*/
//主模板
template<class L, template<class...> class P>     //只有当P声明成模板模板参数的时候，P才能传std::is_integral，因为std::is_integral并不是一个具体的类型，而是一个模板，只有模板模板参数才能和它对应
struct mp_count_if_impl;

//别名模板
template<class L, template<class...> class P>
using mp_count_if = mp_count_if_impl<L, P>::type;

//模板偏特化
template<template<class...> class L, class... T, template<class...> class P>
struct mp_count_if_impl<L<T...>, P>
{
    using type = mp_plus<P<T>...>;
};


/*
* mp 元编程系列原语--mp_bool
* 功能：mp_bool代表一个常量，是std::integral_constant的别名模板
* 例如：
* 注意：
*
*/
template<bool v>
using mp_bool = std::integral_constant<bool, v>;



/*
* mp 元编程系列原语--mp_contains
* 功能：mp_contains<L, V> 返回类型列表L是否包含类型V
* 例如：
* 注意：mp_count_if借助mp_count、mp_bool实现，但是这样的效率比较低，因为这样其实计算了所有次数
*
*/
template<class L, class V>
using mp_contains = mp_bool<mp_count<L, V>::value != 0>;



int main()
{
    //测试mp_plus_impl
    mp_plus<std::integral_constant<bool, true>, std::integral_constant<bool, true>, std::integral_constant<bool, true>> result1;
    std::cout << std::format("{}", demangle(typeid(result1).name())) <<std::endl;     //std::integral_constant<unsigned long, 3ul>

    //测试mp_count
    mp_count<std::tuple<int, int, char, double>, int> result2;
    std::cout << std::format("{}", demangle(typeid(result2).name())) <<std::endl;     //std::integral_constant<unsigned long, 2ul>

    //测试mp_count_if
    mp_count_if<std::tuple<int, int, char, long>, std::is_integral> result3;
    std::cout << std::format("{}", demangle(typeid(result3).name())) <<std::endl;     //std::integral_constant<unsigned long, 4ul>

    return 0;
}


