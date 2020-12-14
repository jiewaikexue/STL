
//一级空间配置器要求
//1. 基本上就是对4个转调函数使用malloc 和free 封装
//2. 最为重要的就是处理内存不足的情况暨 set_new_handler
//

//======
//1. 一级空间配置器的指定
//======

ifdef _USE_MALLOC
... typedef __malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;

//======
//2.一级空间配置器的实现
// 也就是4个转调函数的重写
//======

template <int inst> //inst这个参数完全没用
class __malloc_alloc_template{

    private:
         // 一下函数用来处理内存不足的情况
         // oom --out of memory
         //
         static void * oom_malloc(size_t);
         static void * oom_realloc(void *p,size_t);
         // set_new_handle
         // 这是一个函数指针,有我们自己制定处理函数
         // 用户自定义的oom处理策略
         static void (* __malloc_alloc_oom_handler)();
    public:
        static void * allocate(size_t n)
        {
            //一级空间配置器直接使用malloc
            void * result=malloc(n);
            //考虑malloc失败的可能性
            if(result==0)
            {
                result==oom_malloc(n);//oom_malloc函数时专门用来处理malloc失败的
            }
            return result;//因为外面会强制转换
        }
        static void deallocate (void *p ,size_t /*n*/)
        {
                free(p);
        }
        static void * reallocate(void *p ,size_t old_sz,size_t new_sz)
        {
            void* result=realloc(p,new_sz);//一级空间配置器直接使用realloc
            //考虑realloc配置失败的情况
            if(result==0)
            {
                result=oom_realloc(p,new_sz);
            }
            return  result;
        }






        //=====模仿set_new_handle
        
        
        //// 该函数的参数为函数指针，返回值类型也为函数指针
        //=====
        static void (*set_malloc_handler(void (*f)()))()
        {
            void (* old)() = __malloc_alloc_oom_handler;
            __malloc_alloc_oom_handler = f;
            return(old);
        }
};

//初始化__malloc_allloc_oom_handler
template <int inst>
//让这个函数指针指向为空
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)()=0;

// 定义oom_malloc
template <int inst>
void * __malloc_alloc_template::oom_malloc(size_t n)
{
    //生命一个变量: 函数指针
    void ( * my_malloc_handler)();
    void * result;
    //一个死循环
    for(;;)
    {
    // 检测用户是否设置空间不足应对措施，如果没有设置，抛异常

        //这个函数指针的值,等于用户自定义的处理oom函数
        my_malloc_handler=__malloc_alloc_oom_handler;
        if(my_malloc_handler==0){/*THROW_BAD_ALLOC*/}
        else{
            //对函数指针* 相当于调用malloc_alloc_oom_handler
            (*my_malloc_handler)();//调用处理历程 暨调用用户提供的空间不足的应对措施
            result=realloc(p,n);//再次尝试分配内存
        }

    }

void * __malloc_alloc_template::oom_realloc(size_t n)
{
    //生命一个变量: 函数指针
    void ( * my_realloc_handler)();
    void * result;
    //一个死循环
    for(;;)
    {
    // 检测用户是否设置空间不足应对措施，如果没有设置，抛异常

        //这个函数指针的值,等于用户自定义的处理oom函数
        my_realloc_handler=__malloc_alloc_oom_handler;
        if(my_realloc_handler==0){/*THROW_BAD_ALLOC*/}
        else{
            //对函数指针* 相当于调用malloc_alloc_oom_handler
            (*my_realloc_handler)();//调用处理历程 暨调用用户提供的空间不足的应对措施
            result=realloc(p,n);//再次尝试分配内存
        }

    }
}


//指定配置器为以及空间配置器

typedef __malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;









