/*
* mp_transform系列
*
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
* mp 元编程系列原语--mp_push_front
* 技巧：这里使用模板别名将模板参数转发给类模板，这是常见的操作，因为类模板可以特化和偏特化
* 功能：mp_push_front<L, T...>将多个类型T...添加到类型列表L中
* 例如：mp_push_front<mp_list<int, double>, char>会得到mp_list<char, int, double>
* 注意：
*
*/
//主模板
template<class L, class... T>
struct mp_push_front_impl;

//类模板偏特化
template<template<class...> class L, class... U, class... T>
struct mp_push_front_impl<L<U...>, T...>  //最关键的是这里的特化，可以将模板参数重新定义
{
    using type = L<T..., U...>;
};


//通过模板别名来简化使用，将类型模板参数转发给类模板
template<class L, class... T>
using mp_push_front = typename mp_push_front_impl<L, T...>::type;


/*
* mp 元编程系列原语--mp_transform
* 技巧：这里使用mp_push_front来实现mp_transform（这只是一种实现方式）
* 功能：mp_transform<F, L>将类型列表L中的每个类型应用到元函数F上，并返回一个新的类型列表
* 例如：mp_transform<std::add_pointer_t, std::tuple<int, double, char>>会得到std::tuple<int*, double*, char*>
* 注意：这个实现方式比较old，C++11之后有更好的实现方式
*
*/
//主模板
template<template<class...> class F, class L>
struct mp_transform_impl;

//类模板偏特化（递归结束）
template<template<class...> class F, template<class...> class L>
struct mp_transform_impl<F, L<>>   //最关键的是这里的特化，可以将模板参数重新定义
{
    using type = L<>;  //返回空类型列表，递归返回
};

//类模板偏特化（递归）
template<template<class...> class F, template<class...> class L, class T1, class... T>
struct mp_transform_impl<F, L<T1, T...>>    //最关键的是这里的特化，可以将模板参数重新定义
{
    using _first = F<T1>;
    using _rest = typename mp_transform_impl<F, L<T...>>::type;

    using type = mp_push_front<_rest, _first>;
};


//通过模板别名来简化使用，将类型模板参数转发给类模板
template<template<class...> class F, class L>
using mp_transform = mp_transform_impl<F, L>::type;



/*
* mp 元编程系列原语--mp_transform_pro（等价于mp_transform，实现当时不同）
* 技巧：这里使用mp_push_front来实现mp_transform（这只是一种实现方式）
* 功能：mp_transform<F, L>将类型列表L中的每个类型应用到元函数F上，并返回一个新的类型列表
* 例如：mp_transform<std::add_pointer_t, std::tuple<int, double, char>>会得到std::tuple<int*, double*, char*>
* 注意：这个实现方式是C++11以后的实现方式（使用参数包展开的方式）
*
*/
//主模板
template<template<class...> class F, class L>
struct mp_transform_pro_impl;

//类模板偏特化
template<template<class...> class F, template<class...> class L, class... T>
struct mp_transform_pro_impl<F, L<T...>>
{
    using type = L<F<T>...>;  //使用参数包展开的方式，将F应用到L中的每个类型T上
};


//通过模板别名来简化使用，将类型模板参数转发给类模板
template<template<class...> class F, class L>
using mp_transform_pro = mp_transform_pro_impl<F, L>::type;




int main()
{
    //测试mp_transform
    mp_transform<std::add_pointer_t, std::tuple<int, double, char>> mp_transform_result1;       //std::tuple<int*, double*, char*>
    mp_transform<std::add_const_t, std::tuple<int, double, char>> mp_transform_result2;         //std::tuple<int const, double const, char const>
    //mp_transform<std::add_const_t, std::pair<int, double>> mp_transform_result3;              //std::pair不行，因为std::pair必须有2类类型参数，std::pair<int>是不对的
    
    std::cout << std::format("{}", demangle(typeid(mp_transform_result1).name())) <<std::endl;  //std::tuple<int*, double*, char*>
    std::cout << std::format("{}", demangle(typeid(mp_transform_result2).name())) <<std::endl;  //std::tuple<int const, double const, char const>



    //测试mp_transform_pro
    mp_transform_pro<std::add_pointer_t, std::tuple<int, double, char>> mp_transform_pro_result1; //std::tuple<int*, double*, char*>
    mp_transform_pro<std::add_const_t, std::tuple<int, double, char>> mp_transform_pro_result2;   //std::tuple<int const, double const, char const>

    std::cout << std::format("{}", demangle(typeid(mp_transform_pro_result1).name())) <<std::endl; //std::tuple<int*, double*, char*>
    std::cout << std::format("{}", demangle(typeid(mp_transform_pro_result2).name())) <<std::endl; //std::tuple<int const, double const, char const>

    return 0;
}



