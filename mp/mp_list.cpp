/*
* mp_list系列
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
* mp 元编程系列原语--mp_length(类似mp_size)
* 功能：获取类型列表的长度（即包含的类型的数量），可以用于mp_list、std::tuple等类型
* 例如：mp_length<mp_list<int, double, char>>会得到 std::integral_constant<size_t, 3>
* 注意：得到的结果也是一个类型，因为常量在模板元编程中也是一种类型，如果要得到这个类型中具体的值，则可以使用::value来获取
*
*/
template<class... T>
using mp_length = std::integral_constant<size_t, sizeof...(T)>;

// 技巧，可以使用mp_rename和mp_length来实现mp_size
//template<class L>
//using mp_size = mp_rename<L, mp_length>;



/*
* mp 元编程系列原语--mp_apply(使用类型列表来调用某个元函数)
* 注意：类模板和模板模板参数可以视为元函数，类型列表和可变参模板参数可以视为元函数的参数
*
*/
template<template<class...> class F, class L>
using mp_apply = mp_rename<L, F>;


/*
* mp 元编程系列数据结构，类型列表
* 这里使用模板类来定义一个类型列表，允许存储任意数量的类型
*
*/
template<class... T> 
struct mp_list {};


int main() 
{
    //m_rename使用方式
    mp_rename<std::pair<int, double>, std::tuple> my_list;      //std::tuple<int, double>
    mp_rename<mp_list<int, double, char>, std::tuple> my_list2; //std::tuple<int, double, char>

    std::cout << std::format("{}", demangle(typeid(my_list).name())) <<std::endl;     //std::tuple<int, double>
    std::cout << std::format("{}", demangle(typeid(my_list2).name())) << std::endl;   //std::tuple<int, double, char>

    //mp_size使用方式
    mp_size<mp_list<int, double, char>> my_size1;    //std::integral_constant<size_t, 3>
    mp_size<std::tuple<int, char>> my_size2;         //std::integral_constant<size_t, 2>

    std::cout << std::format("{}", demangle(typeid(my_size1).name())) << std::endl;  //std::integral_constant<unsigned long, 3ul>
    std::cout << std::format("{}", demangle(typeid(my_size2).name())) << std::endl;  //std::integral_constant<unsigned long, 2ul>
    std::cout << "value of my_size1: " << mp_size<mp_list<int, double, char>>::value << std::endl;  //3
    std::cout << "value of my_size2: " << mp_size<std::tuple<int, char>>::value << std::endl;       //2

    //mp_apply使用方式
    mp_apply<mp_list, std::tuple<int, char, bool>> my_apply1;
    mp_apply<mp_length, std::tuple<char, bool>> my_apply2;
    std::cout << std::format("{}", demangle(typeid(my_apply1).name())) <<std::endl;   //mp_list<int, char, bool>
    std::cout << std::format("{}", demangle(typeid(my_apply2).name())) <<std::endl;   //std::integral_constant<unsigned long, 2ul>

    return 0;
}