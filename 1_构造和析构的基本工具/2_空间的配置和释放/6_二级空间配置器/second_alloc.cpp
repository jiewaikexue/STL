
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
        static size_t FREELIST_INDEX(size_t bytes)
        {
            return (((bytes)+__ALIGN-1)/__ALIGN-1);
        }
        //返回一个大小为n的对象,并可能加入大小为n的其他区块到freelist
        static void * refill(size_t n);

        // 配置一大块空间,可以容纳的nobjs个大小为size的区块
        // //如果配置nobjs个区块有所不便,nobjs可能会降低
        //
};
