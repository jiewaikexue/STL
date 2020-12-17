
/* 函数接受三个参数
 * 一个迭代器前闭后开区间
 * 一个迭代器指向的输出端的起始位置(想要初始化的空间)
 */
template <class InputIterator,class ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator first,InputIterator last,ForwardIterator result)
{
    return __uninitialized_copy(first,last,result,value_type(first));
}
    template <class InputIterator,class ForwardIterator,class T>
inline ForwardIterator __uninitialized_copy(InputIterator first,InputIterator last,ForwardIterator result,T*)
{
    typedef typename __type_traits<T>::is_POD_type is_POD;
    return __uninitialized_copy_aux(first,last,result,is_POD());
}

/*
 * 判断迭代器指向的对象是否是POD类型
 */

template <class InputIterator,class ForwardIterator>
inline ForwardIterator __uninitialized_copy__aux(InputIterator first,InputIterator last,ForwardIterator result,__true_type)
{
    return copy(first,last,result);//采用最高效率的做法
}

//如果不是pod类型
template <class InputIterator,class ForwardIterator>
inline ForwardIterator __uninitialized_copy__aux(InputIterator first,InputIterator last,ForwardIterator result,__false_type)
{
    ForwardIterator cur=result;
    for(;first!=last;first++,cur++)
    {
        construcor(&*cur,* first);//每一个对象都要通过构造函数慢慢来
    }
}
