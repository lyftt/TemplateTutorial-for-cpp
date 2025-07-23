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
* mp 元编程系列原语--mp_size
* 技巧：使用sizeof...(T)来获取可变参数模板的大小，还有在模板元编程中会将常量值也当作一个类型，以为模板元编程只能操作类型，将常量也做成类型就可以统一起来了
* 功能：获取类型列表的大小（即包含的类型的数量），可以用于mp_list、std::tuple等类型
* 例如：mp_size<mp_list<int, double, char>>会得到 std::integral_constant<size_t, 3> 
* 注意：得到的结果也是一个类型，因为常量在模板元编程中也是一种类型，如果要得到这个类型中具体的值，则可以使用::value来获取
* 
*/
//主模板
template<class L>
struct mp_size_impl;

//类模板偏特化
template<template<class...> class L, class... T>
struct mp_size_impl<L<T...>>  //最关键的是这里的特化，可以将模板参数重新定义
{
    using type = std::integral_constant<size_t, sizeof...(T)>;
};

template<class L>
using mp_size = mp_size_impl<L>::type;



/*
* mp 元编程系列原语--mp_itoa
* 技巧：
* 功能：类似std::itoa，获得常量类型组成的类型列表
* 例如：mp_itoa<std::integer_constant<std::size_t, 3>> 就会得到mp_list<std::integer_constant<std::size_t, 0>, std::integer_constant<std::size_t, 1>, std::integer_constant<std::size_t, 2>>
* 注意：在元编程中，常量也是一个类型
* 
*/
template<class N>
using mp_itoa = mp_from_sequence<std::make_index_sequence<N::value>>;     //从std::integer_constant<std::size_t, N>获取具体值之后算出索引列表std::integer_sequence<std::size_t, 1,2,3,...N>，最后使用mp_from_sequence获得每个常量类型组成的列表

template<class L>
using F = mp_itoa<mp_size<L>>;    //得到编译器常量（也是个类型），std::integer_constant<std::size_t, N>表示值为N的常量


/*
* mp 元编程系列原语--mp_rename
* 技巧：这里使用模板别名将模板参数转发给类模板，这是常见的操作，因为类模板可以特化和偏特化
* 功能：mp_rename可以重命名任意一个可变参类模板到另一个可变参类模板
* 例如：将std::pair<int, double>重命名为std::tuple<int, double>
* 注意：主要目的是将其他类型重命名成mp_list，或将mp_list重命名成其他类型
*
*/
//主模板，有2个模板参数，A是类型模板参数，B是模板模板参数（即B是一个模板）
template<class A, template<class...> class B> 
struct mp_rename_impl;

//类模板偏特化
template<template<class...> class A, class... T, template<class...> class B>
struct mp_rename_impl<A<T...>, B>  //最关键的是这里的特化，可以将模板参数重新定义
{
    using type = B<T...>;   //元函数调用
};

//通过模板别名来简化使用，将类型模板参数转发给类模板, A是类型模板参数，B是模板模板参数（即B是一个模板）
template<class A, template<class...> class B>
using mp_rename = typename mp_rename_impl<A, B>::type;


/*
* 著名的tuple_cat函数的实现，使用mp系列元编程原语
* 使用的mp系列元编程原语
* - mp_append，mp_append<L1,L2,...>将多个类型列表L2,...等类型列表中的类型全部添加到L1中，用来获取std::tuple_cat的返回值类型
* - mp_fill，
* - mp_transform，
* - mp_from_sequence，
* - mp_size
* - mp_itoa
* - mp_rename
*/
template<class R, class... Is, class... Ks, class Tp>
R mp_tuple_cat_(mp_list<Is...>, mp_list<Ks...>, Tp tp) //辅助函数
{
    return R{ std::get<Ks::value>(std::get<Is::value>(tp))... };
}


template<class... Tp, class R = mp_append<std::tuple<>, std::remove_reference_t<Tp>...>>   //R是返回类型
R mp_tuple_cat(Tp&&... tp)
{
    constexpr std::size_t N = sizeof...(Tp);

    /*inner*/
    //[[x1, x2, x3], [], [y1, y2, y3], [z1, z2]]
    using tp_list = mp_list<std::remove_reference_t<Tp>...>;
    using seq = std::make_index_sequence<N>;     //得到integer_sequence<std::size_t, 0, 1, 2, 3>

    //[0, 1, 2, 3]
    using seq_list = mp_from_sequence<seq>;      //将integer_sequence转换为类型列表mp_list<std::integral_constant<std::size_t, 0>, std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>, std::integral_constant<std::size_t, 3>>

    //[[0, 0, 0], [], [2, 2, 2], [3, 3]]
    using constant_list = mp_transform<mp_fill, tp_list, seq_list>;   //填充

    //inner
    using inner = mp_rename<constant_list, mp_append>;   //注意，此时inner还是std::tuple<...>的类型

    /*outer*/
    //[[0, 1, 2], [], [0, 1, 2], [0, 1]]
    using constant_list2 = mp_transform<F, tp_list>;

    //outer
    using outer = mp_rename<constant_list2, mp_append>; //注意，此时outer是mp_list<...>的类型了

    //return mp_tuple_cat_<R>(inner(), outer(), std::forward_as_tuple(std::forward<Tp>(tp)...));   //会报错，因为此时inner还是std::tuple<...>的类型，无法匹配到mp_tuple_cat_()函数
    return mp_tuple_cat_<R>(mp_rename<inner, mp_list>(), outer(), std::forward_as_tuple(std::forward<Tp>(tp)...));
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


    //测试tuple_cat
    std::tuple<int, char> t1{1, 0x02};
    std::tuple<double, long> t2{2.1, 100};
    auto t3 = mp_tuple_cat(t1, t2);
    std::cout << std::format("{}", demangle(typeid(t3).name())) << std::endl;  //std::tuple<int, char, double, long>
    std::cout << std::get<0>(t3) << std::endl;
    std::cout << std::get<1>(t3) << std::endl;
    std::cout << std::get<2>(t3) << std::endl;
    std::cout << std::get<3>(t3) << std::endl;

    return 0;
}


