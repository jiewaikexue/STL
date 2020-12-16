
/*
 * 二级空间配置器基本准则:
 *      1.维护16个自由链表,负责16中小型区块的次配置能力  ---次层配置
 *      2. 内存池通过malloc配置而得,如果内存池不够,就转调一级配置器
 *      3. 如果需求的块大于128字节,就转调一级空间配置器
 *
 * 二级空间配置器的
 *
 * 次层配置:每次配置一大块内存,并维护与之对应的自由链表
 *          如果下次有相同大小的内存需求,则直接从自由一链表拔出.
 *          如果客户释放小额块区,则将内存块插入到自由链表中
 *
 *
 *  小额块区的内存需求量自动上条到8的倍数,维护16个自由链表,分别管理 8xn字节的小额块区
 *
 */
//自由链表的节点结构

unio obj{
    union obj*free_list_link;//自由链表指针,指向下一个相同形式的obj
    char client_data[1];//一个数组,可以看成是一个指针,指向实际区块
}
/*
 * 该union很巧妙
 * 一物二用的结果就是 obj 可以被看成是一个指针
 * 也可以看升是指向一个实际却快的指针,
 *
 * 避免了每一个小额块区的维护指针所造成的浪费
 * union 同一时间只能使用一个
 */


/* 二级配置器部分内容*/

enum 
{
__ALIGN=8 //小型区块的上条边界
};
enum{ __MAX_BYTES=128 };//小型区块的最大大小
enum { __NFREELISTS=__MAX_BYTES/__ALIGN };//自由链表的个数
// 第一个参数用于多线程的环境下,第二个参数没用
template <bool threads, int inst>
class __default_alloc_template{
    private:
        //需要将请求份额上调至8的倍数
        static size_t ROUND_UP(size_t bytes)
        {
            return (((bytes)+__ALIGN-1)& ~(__ALIGN-1));
        }
    private:
        union obj{
            union obj *free_list_link;
            char client_data[1];
        };
    private:
        //当要求使用 volatile 声明的变量的值的时候，系统总是重新从它所在的内存读取数据，即使它前面的指令刚刚从该处读取过数据。而且读取的数据立刻被保存。
        static obj * volatile free_list[__NFREELISTS];
        // 选择自由链表
        // 可以理解为一个hash函数
        static size_t FREELIST_INDEX(size_t bytes)
        {
            return (((bytes)+__ALIGN-1)/__ALIGN-1);
        }
        //返回一个大小为n的对象,并可能加入大小为n的其他区块到freelist
        static void * refill(size_t n);

        // 配置一大块空间,可以容纳的nobjs个大小为size的区块
        // //如果配置nobjs个区块有所不便,nobjs可能会降低
        static char * chunk_alloc(size_t size,int&nobjs);
        /* ========================
        /* 三个变量 和内存池相关
         * ========================
         */
        static char* start_free;//内存池空闲区域的起始位置
        static char* end_free;//内存池空闲区域的结束位置
       // 用来记录该空间配置器已经想系统索要了多少的内存块 
        static  size_t heap_size;//内存池的大小,

    public:
        static void * allocate(size_t n);
        static void* reallocate(void *p,size_t old_sz,size_t new_sz);
        static void *deallocate(void *p,size_t n);
};
//对内部变量进行初始化
template <bool threads,int inst>
char * __default_alloc_template<threads,inst>::start_free=nullptr;
template <bool threads,int inst>
char * __default_alloc_template<threads,inst>::end_free=nullptr;
template <bool threads,int inst>
char * __default_alloc_template<threads,inst>::heap_size=0;
//定义freelist链表
template <bool threads,int inst>
 __default_alloc_template<threads,inst>::free_list[__NFREELISTS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};






/* 空间配置函数 allocate 
 */
template <bool threads,int inst>
 void * __default_alloc_template<threads,inst>::allocate(size_t n)
{
/*
 *主要逻辑
 * 1.检查申请空间是否大于128字节
 *        ----大于,交给以及空间配置器
 * 2.检查对应的free_List查看是否有可用区块
 *        ----有 就分配
 *        ----没有就调用refill为free_list填充空间
 *
 */ 
    obj * volatile *my_free_list; //二级指针
    obj* result;//一级指针
    if(n>128)
    {
        return (malloc_allocate::allocate(n));//由一级空间配置器申请

    }
    //嗲用hash函数,寻找管理对应空间大小的free_list
    my_free_list=free_list+FREELIST_INDEX(n); //--->free_list数组首地址, 二级指针指向自由链表数组对应的元素,该元素内部还是一个指针
    result=*my_free_list;// result 指针指向当前自由链表的第一个空闲obj
    if(result==nullptr)//没哟空闲的obj节点了
    {
        void * r=refill(ROUND_UP(n));// 使用refill去内存池里面拿
        return r; //注意这里,这里返回给客户的 是一个指针,一般可能申请多个,但是只返回给客户一个(提前申请)

    }
    //还有剩余的空闲obj
    *my_free_list=result->free_list_link; //让当前自由链表数组的对应索引的元素,指向下一个空闲obj  ---->这一个空闲的obj分配出去
    return (result);
}

// __default_alloc_template::reallocate 同理,只是利用的是一级空间配置器里的reallocate

/* __default_alloc_template::deallocate 
 * 空间配置器的释放
 *
 * 基本逻辑:
 * 1.大于128字节,就交给以及配置器
 * 2.小于128字节,就修改自由链表的指针,让他重新挂回到自由链表
 */ 
template <bool threads,int inst>
 void* __default_alloc_template<threads,inst>::deallocate(void *p,size_t n)
{
    obj *q=(obj *)p;
    obj * volatile *my_free_list;
    if(n>128)
        return (malloc_allocate::deallocate(p,n));
    else{
        my_free_list=free_list+FREELIST_INDEX(n);//找出对应的自由链表数组位置
        q->free_list_link=*my_free_list; //q->free_list_link是我当前这个待释放的obj *my_free_list指向我第一个空闲的obj
        *my_free_list=q; //让当前待释放的obj回归到free_list,这样他就释放完成了

    }
}

//=========================
//和空间配置函数allocate息息相关的 重新装填函数refill
//=========================
//
//首先明确,refill函数的工作,是当freelist中没有可以用的块之后,就调用refill来装填,是从内存池获取空间的
//
template <bool threads,int inst>
void * __default_alloc_template<threads,inst>::refill(size_t n)
{
    //1.明确 新空间是从内存池中来的
    //2.考虑申请空间的大小
    //2.1 因为会提前申请,可能会申请多了
    //2.2 如果恰好值申请到一个
    
    int nobjs=20;//默认每次申请都申请20个区块
    /* 此处应该先将 n进行边界调整
     * __default_alloc_template:ROUND_UP(n)
     */
    char * chunk=chunk_alloc(n,objs);//n为区块大小,objs 为区块个数默认20,  该函数为向内存池申请空间
    obj * volatile *my_free_list;//二级指针
    obj * result;
    obj * current_obj, *next_obj; //当前obj区块,下一个obj区块
    int i;
if(1==nobjs)//如果只申请到一个节点返回就好了
    return (chunk);
 //多于一个节点,就将多于的纳入到自由链表 
    my_free_list=free_list+FREELIST_INDEX(n);
    result=(obj *)chunk;//result是第一个自由节点,即将返回给申请者
    //即将在以chunk指针为起点的空间内建立free_list
    next_obj=(obj*)(chunk+n);//指向chunk区域内部的第二个自由节点
    *my_free_list=next_obj;

    for(i=1;;i++)
    {
        //双指针循环 
        current_obj=next_obj;
        next_obj=(obj*)((char*)next_obj+n);//next_obj指向下一个
        if(njobs-1==i)//njobs缺省为20
        {
            //最后一个以chunk为七点的区域被划分成freelist节点完成
                current_obj->free_list_link=0;
        }
        else{
            current_obj->free_list_link=next_obj;
        }
    }
    
}

//refill内部其实就干了两件事
//
//1.向内存池申请空间
//2.把多余的空间规划成free_list
//根本上最为重要的还是 chunk_alloc函数
// 
template<bool threads,int inst>
char *__default_alloc_template<threads,inst>::chunk_alloc(size_t size,int &njobs)
{

    /* 主要逻辑
     * 1.判断需求量是否大于供给量
     *      1.1供过于求: 一切相安无事,要多少就给多少,反正内存池里面还够
     *      1.2供不应求:考虑两种情况
     *          1.2.1 剩余空间不能完全满足需求,但是可以提供至少一个
     *          1.2.2 剩余空间连一个都无法满足------>内存池不够了 malloc重新申请内存
     *                1.2.2.1 malloc成功
     *                1.2.2.2 malloc失败 
     */
    char * result;
    size_t total_bytes=size*nobjs;//总共所需要的空间
    size_t byte_left=end_free-start_free; //内存池剩下的空间
    if(byte_left>=total_bytes)//供过于求
    {
        result=start_free;
        start_free+=total_bytes;//将start_free移动,此区域划分出去了
        return (result);//返回result ,也就相当于 refill里面的chunk指针
        //refill内部 会对 该指针返回的区域进行处理
        //在这里 njobs一定会等于20
        //因为内存池够20个n 所以一定会分配出去
    }
    else if(byte_left>=size)//供不应求,但至少有一个

    {
        nobjs=byte_left/size; //在这里改变nobjs的个数  对应refill内部的objs 因为传进来的是引用
        total_bytes=size*nobjs;
        result=start_free;
        start_free+=total_bytes;
        return (result);
        //可能返回多个 
        //可能返回一个
        //但是一定不是20个
        //交给上层的refill 根据nobjs 的情况进行划分  
    }
    else //供应不求,一个没有
    {
        //考虑 .确实连一个都分配不出去了,
        //但是缺可以分配出去0.5个
        //如果这点内存不尽兴处理,就会造成内存碎片
        

        //所以: 最为重要的一步
        //1.当前内存池无法满足分配需求,所以需要把剩余的start_free~end_free划分出去,
        //2.然后再调用malloc来分配heap
        
        size_t bytes_to_get=2*total_bytes+ROUND_UP(heap_size>>4);//新申请的内存,的计算公式.
        //上述这个公式,能够有效的减少频繁manlloc
        //eg 当前heap_size=32 块大小为8    新申请的内存=2*20*8 +8=328
        



        //当前内存池已经不够了 ,先整理内存池剩余空间的小碎片
        

        if(byte_left>0)
        {
            //寻找适当的freelist数组下标
            obj * volatile * my_free_list=free_list+FREELIST_INDEX(byte_left);
            //调整free_list 将内存池剩余空间编入
            ((obj*)start_free)->free_list_link=*my_free_list;
            *my_free_list=(obj*)start_free;
        }
      //申请堆空间来补充内存池
      


        //为什么这里的start_free是这样的?
        //因为:你的以前的内存池,已经彻底分配完成了
        //由此可见 内存池 不一定是完全连续的 打有可能是离散分布的
        

      start_free=(char*)malloc(bytes_to_get);//申请带有扩大因子的内存
      
      
      // malloc成功啥都好说
      // malloc失败该怎么做
      
      if(start_free!=nullptr)
      {
          //malloc成功
          //内存池有新的内存了,源头活水来了
          
          heap_size+=bytes_to_get;
          end_free=start_free+=bytes_to_get;
          //递归调用自己,因为你新的内存已经补充进来了 所以你可以再次申请,这次申请时一定 供大于求
          

          return(chunk_alloc(size,objs));  
          
          //新的内存池 不是连续的该怎么办
          //chunk_alloc申请到新的内存
          //然后返回到上层refill
          //refill拿到内存之后,会对拿到的20个块的内存进行编成free_lisy
          //
          //但是并内存池剩余(-20)的内存 还是在哪里的 start_free和end_free之间是连续的 malloc之间是离散的
          

      }
      else{
          //最坏的情况
          //malloc失败
          
          //malloc失败该怎么做
          //
          //================
          //遍历我们的16个自由链表
          //找出其中的每一个空闲块
          //释放
          int i;
          obj *volatile * my_free_list,*p;
          for (i=size;i<=__MAX_BYTES;i+=__ALIGN)
          {
              my_free_list=free_list+=FREELIST_INDEX(i);
              p=*my_free_list;
              if(0!=p)
              {
                  //调整free_list以释放没有使用的区块
                  
                  *my_free_list=p->free_list_link;
                  start_free=char(char*)p;
                  end_free+=start_free+i;
                  //递归调用子街
                  return(chunk_alloc(size,nobjs));
              }
          }
          end_free=0;
          //调用第一季空间配置器 看oom能不能救一下
          start_free=(char*)malloc_alloc::allocate(bytes_to_get);

      }
    }

}
