/*
* mp_transform2系列，与mp_transform的差别在于，mp_transform2使用的是2元的元函数（例如std::pair）
*
* 一个使用场景： 
* using input = std::pair<std::tuple<X1, X2, X3>, std::tuple<Y1, Y2, Y3>>;
* using expected = std::tuple<std::pair<X1, Y1>, std::pair<X2, Y2>, std::pair<X3, Y3>>;
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
* mp 元编程系列原语--mp_transform2
* 技巧：这里使用C++11以后的参数包展开来实现mp_transform2
* 功能：mp_transform<F, L1, L2>，将类型列表L1和L2中的每个类型应用到元函数F上，并返回一个新的类型列表
* 例如：std::pair<std::tuple<X1, X2, X3>, std::tuple<Y1, Y2, Y3>>  ->  std::tuple<std::pair<X1, Y1>, std::pair<X2, Y2>, std::pair<X3, Y3>>
* 注意：
*
*/
//主模板
template<template<class...> class F, class L1, class L2>
struct mp_transform2_impl;


//类模板偏特化
template<template<class...> class F, 
         template<class...> class L1, class... T1, 
         template<class...> class L2, class... T2>
struct mp_transform2_impl<F, L1<T1...>, L2<T2...>>   //最关键的是这里的特化，可以将模板参数重新定义
{
    using type = L1<F<T1,T2>...>;  //使用参数包展开的方式，将F应用到L1和L2中的每个类型T1和T2上
};


//别名模板
template<template<class...> class F, class L1, class L2>
using mp_transform2 = typename mp_transform2_impl<F, L1, L2>::type;


int main()
{
    //测试mp_transform2
    mp_transform2<std::pair, std::tuple<int, double, char>, std::tuple<char, long, float>> result;
    std::cout << std::format("{}", demangle(typeid(result).name())) <<std::endl;   //std::tuple<std::pair<int, char>, std::pair<double, long>, std::pair<char, float>>

    return 0;
}



