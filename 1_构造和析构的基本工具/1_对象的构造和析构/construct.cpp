#include <new.h>

    template <class T1,class T2>
inline void destory(T1 *p,const T2& value)
{
    new(p)T1(value);//这里使用的是placementnew

}

//第一个版本的析构函数,接受一个指针
    template <class T>
inline void destory(T * pointer)
{
    pointer->~T();//调用该类里面的析构函数
}
//第二个版本你的destory()
//第二个版本接受两个迭代器
//我们无法知道元素的数值型别,
//所以使用type_traits<>来求取最适当的措施
//
    template <class ForwardIterator>
inline void destory(ForwardIterator first,ForwardIterator last)
{
    __destory(first,last,value_type(first));
    //内部交给__destory函数处理,该函数利用两个迭代器以及第三个参数::迭代器锁指向数据的类型
}

//===============
//首先,判断元素的数值型别内部是否有 trival destructor

    template <class ForwardIterator,class T*>
inline void __destory(ForwardIterator first, ForwardIterator last,T*)
{
    // typename 告诉编译器这是一个类型
    // 
    typedef  typename  __type_traits <T>::has_trival_destructor trival_destructor;
    //将结果交付给下层
    //该函数利用 上面typedef的结果,
    __destory_aux(first,last,trival_destructor());
}
//============
//这里是__destructor_aux函数的两种情况 
//暨: 对象类别有虚析构 没有虚析构
//===========
//
//1.无虚析构
template <class ForwardIterator>
// ]第三个参数可能是一个宏 有上面的 __type_traits<t>::has_trival_destructor 判断出来的
inline void __destory_aux( ForwardIterator first, ForwardIterator last,__false_type)
{
    //循环析构两个迭代器中间的每一个对象
    for(;first<last;++first)
    {
        //这里用的是取地址
        //使用的是 我们第一个定义的
        //以指针为阐述destory()
        destory(&*first);
    }
}

//2.如果对象的里面有虚析构
//
template <class ForwardIterator >
inline void __destory_aux(ForwardIterator first,ForwardIterator last,__true_type)
{
    //因为是虚函数,所以不做处理
}

// 特化地带器版本
// 1. 迭代器为char*

//2.迭代器为wchar_t*
