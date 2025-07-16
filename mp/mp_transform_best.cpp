/*
* mp_transform系列，与mp_transform和mp_transform2的区别在于，mp_transform使用的是一元函数，mp_transform2使用的是2元的元函数（例如std::pair）；
* mp_transform_best则是统一了实现（通过可变参模板），一元和二元只是一种特化
*
*/

#include <iostream>
#include <tuple>
#include <utility>
#include <format>
#include <type_traits>
#include <cxxabi.h>

std::string demangle(const char* name) {
    int status = -1;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result(status == 0 ? demangled : name);
    free(demangled);
    return result;
}


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



int main()
{
    mp_transform<std::pair, std::tuple<int, double, char>, std::tuple<char, long, float>> result1;
    mp_transform<std::add_pointer_t, std::tuple<int, double, char>> result2;

    std::cout << std::format("{}", demangle(typeid(result1).name())) <<std::endl;   //std::tuple<std::pair<int, char>, std::pair<double, long>, std::pair<char, float> >
    std::cout << std::format("{}", demangle(typeid(result2).name())) <<std::endl;   //std::tuple<int*, double*, char*>

    return 0;
}


