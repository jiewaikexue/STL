

template <class T,clas Alloc=alloc>//缺省参数
class vector{
    public:
        //vector 嵌套型别定义
        typedef T value_type;
        typedef value_type* pointer;
        typedef value_type* iterator;
        typedef value_type* reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
    protected:
        //vector有自己定制的空间配置器
        typedef simple_alloc<value_type,Alloc>data_allocator;
        // 三个指针来划分vector的空间
        
        iterator start;
        iterator finish;
        iterator end_of_storage;
protected:
        //这几个函数我没看懂
        void insert_aux(iterator position,const T& x);

        //vector的空间的释放,使用的是配置器中的deallocate
        void deallocate(){
            if(start)
                data_allocator::deallocate(start,end_of_storage-start);
        }

        //对三个指针进行初始化
        void fill_initialize(size_type n,const T& value)
        {
            start =allocate_and_fill(n,value);
            finish=start+n;
            end_of_storage=finish;
        }
public:
        //以下一些函数协成函数声明
        iterator begin()
        {
            return start;
        }
        iterator end()
        {
            return finish;
        }
        size_type size()const{
            return size_type(end()-begin());
        }
        size_type  capacity()const
        {
            return size_type(end_of_storage-end()); 
        }
        bool empty()const{
            return begin()==end();
        }
        reference operator[](size_type n){
            return *(begin()+n);
        }

        reference front()
        {
            return *begin();
        }
        reference back()
        {
            return *(end(-1));
        }
        void push_back(const T&x)
        {
            insert(end(),x);
        }
        void pop_back()
        {
            if(size()>0)
                    erase(end()-1);
        }
        iterator erase(iterator position);
        iterator erase(iterator first,iterator last)
        {
            iterator i=copy(last,finish,first);
            destory(i,finish);
            finish=finish-(last-first);
            return first;
        }
        void resize(size_type new_size,const T&x);
        void resize(size_type new_size);
        void clear()
        {
            return erase(begin(),end());
        }
        iterator insert(iterator loc,size_type n,const T&) ;
        size_type max_size();
        iterator rbegin();
        iterator rend();
        void swap(vector &from);
        value_type& at(size_type loc);
        void reserve(size_type size);
protected:
        //配置空间并填满内容
        
        iterator allocate_and_fill(size_type n,const T&x)
        {
            //空间配置,空间配置器
            iterator result=data_allocator::allocate(n);
            uninitialized_fill_n(result,n,x);
            return result;
        }
public:
        //构造和析构函数
        vector():start(0),finish(0),end_of_storage(0){}
        vector(size_type n  ,const T&value){fill_initialize(n,value);}
        vector(int n,const T& value){fill_initialize(n,value);}
        vector(long n ,const T& value){fill_initialize(n,value);}
        explicit vector(size_type n){fill_initialize(n,T());}
        ~vector()
        {
            //调用全局的析构函数,
            destory(start,finish);
            //调用自己的释放函数,内部再度调用空间配置器的释放函数
            deallocate();
        }
};

