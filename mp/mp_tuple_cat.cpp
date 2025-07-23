/*
* tuple_cat 挑战实现代码
* 即实现std::tuple_cat的功能，它接受多个元组并将它们连接到另一个 std::tuple 中
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
* mp_list 类型列表
*
*/
template<class... T>
struct mp_list{};


/*
* mp 元编程系列原语--mp_append
* 技巧：这里使用C++11以后的参数包展开来实现mp_append
* 功能：mp_append<L1,L2,...>将多个类型列表L2,...等类型列表中的类型全部添加到L1中，用来获取std::tuple_cat的返回值类型
* 例如：mp_append<std::tuple<int,char>, std::tuple<double>, std::tuple<char>> 会得到std::tuple<int, char, double, char>
* 注意：
*
*/
//主模板
template<class... L>
struct mp_append_impl{};

//类模板偏特化
template<>
struct mp_append_impl<>
{
    using type = mp_list<>;   //如果没有类型列表，则返回空类型列表
};

//类模板偏特化
template<template<class...> class L, class... T>
struct mp_append_impl<L<T...>>
{
    using type = L<T...>;  //如果只有一个类型列表，则直接返回该类型列表
};

//类模板偏特化
template<template<class...> class L1, class... T1, 
         template<class...> class L2, class... T2,
         class... Lr>   //这里的Lr并未被声明未模板模板参数，因为此时不需要，否则如果声明成了模板模板参数则必须当作模板使用
struct mp_append_impl<L1<T1...>, L2<T2...>, Lr...>
{
    using type = mp_append_impl<L1<T1..., T2...>, Lr...>::type;  //递归方式展开参数包
};

//别名模板，简化使用
template<class... L>
using mp_append = typename mp_append_impl<L...>::type;



/*
* mp 元编程系列原语--mp_from_sequence
* 技巧：这里使用C++11以后的参数包展开来实现mp_from_sequence
* 功能：mp_from_sequence<S>，将integer_sequence<std::size_t, 0, 1, 2, 3>转换为类型列表mp_list<std::integral_constant<std::size_t, 0>, std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>, std::integral_constant<std::size_t, 3>>
* 例如：
* 注意：
*
*/
template<class S>
struct mp_from_sequence_impl;

//类模板偏特化
template<template<class T, T... I> class S, class U, U... J>     //这里指明了S是一个模板模板参数，这个模板有2个模板参数（类型T和T类型的多个常量值参数）
struct mp_from_sequence_impl<S<U, J...>>
{
    using type = mp_list<std::integral_constant<U,J>...>;  //如果没有类型，则返回空类型列表
};

//别名模板简化使用
template<class S>
using mp_from_sequence = typename mp_from_sequence_impl<S>::type;



/*
* mp 元编程系列原语--mp_fill
* 技巧：这里使用C++11以后的参数包展开来实现mp_fill
* 功能：mp_fill<L, V>，将类型列表L中的每个类型都替换（填充）为值V（也是一个类型std::integral_constant）
* 例如：
* 注意：
*
*/
//主模板
template<class L, class V>
struct mp_fill_impl{};

//类模板偏特化
template<template<class...> class L, class... T, class V>
struct mp_fill_impl<L<T...>, V>
{
    template<class...> using _fv = V;    //这里别名模板很重要，表示接收任意类型模板参数，但是都会被忽略掉，直接映射到类型V
    using type = L<_fv<T>...>;           //使用参数包展开的方式，将V应用到L中的每个类型T上
};


//别名模板简化使用
template<class L, class V>
using mp_fill = typename mp_fill_impl<L, V>::type;



/*
* mp 元编程系列原语--mp_transform
* 技巧：这里使用C++11以后的参数包展开来实现mp_transform
* 功能：mp_transform<F, L...>
* 例如：mp_transform<F, >
* 注意：
*
*/
//主模板
template<template<class...> class F, class... L>
struct mp_transform_impl;

//模板偏特化1--一元函数
template<template<class...> class F, template<class...> class L, class... T>
struct mp_transform_impl<F, L<T...>>
{
    using type = L<F<T>...>;  //使用参数包展开的方式，将F应用到L中的每个类型T上
};

//模板偏特化2--二元函数
template<template<class...> class F, 
         template<class...> class L1, class... T1, 
         template<class...> class L2, class... T2>
struct mp_transform_impl<F, L1<T1...>, L2<T2...>>
{
    using type = L1<F<T1,T2>...>;     //使用参数包展开
};

//还可以有更多模板偏特化...


//别名模板，简化使用
template<template<class...> class F, class... L>
using mp_transform = typename mp_transform_impl<F, L...>::type;



/*
* 
*
*
*/



/*
* 著名的tuple_cat函数的实现，使用mp系列元编程原语
* 使用的mp系列元编程原语
* - mp_append，mp_append<L1,L2,...>将多个类型列表L2,...等类型列表中的类型全部添加到L1中，用来获取std::tuple_cat的返回值类型
* - mp_fill，
* - mp_transform，
* - mp_from_sequence，
*
*/
template<class... Tp, class R = mp_append<std::tuple<>, std::remove_reference_t<Tp>...>>   //R是返回类型
R tuple_cat(Tp&&... tp)
{
    constexpr std::size_t N = sizeof...(Tp);

    /*inner*/
    //[[x1, x2, x3], [], [y1, y2, y3], [z1, z2]]
    using typ_list = mp_list<std::remove_reference_t<Tp>...>
    using seq = std::make_index_sequence<N>;     //得到integer_sequence<std::size_t, 0, 1, 2, 3>

    //[0, 1, 2, 3]
    using seq_list = mp_from_sequence<seq>;      //将integer_sequence转换为类型列表mp_list<std::integral_constant<std::size_t, 0>, std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>, std::integral_constant<std::size_t, 3>>

    //[[0, 0, 0], [], [2, 2, 2], [3, 3]]
    using constant_list = mp_transform<mp_fill, typ_list, seq_list>;   //填充

    //inner
    using inner = mp_rename<constant_list, mp_append>;

    /*outer*/
    using 

}


int main()
{
    //测试mp_append
    mp_append<std::tuple<int, char>, std::tuple<double>, std::tuple<char>> result1;
    mp_append<std::tuple<int, char>, std::tuple<double>, std::tuple<char, float>> result2;
    mp_append<> result3;  //测试空类型列表
    std::cout << std::format("{}", demangle(typeid(result1).name())) << std::endl;   //std::tuple<int, char, double, char>
    std::cout << std::format("{}", demangle(typeid(result2).name())) << std::endl;   //std::tuple<int, char, double, char, float>
    std::cout << std::format("{}", demangle(typeid(result3).name())) << std::endl;   //mp_list<>

    return 0;
}


