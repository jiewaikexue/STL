#include <sstream>
#include<vector>
#include<stdio.h>
#include<mysql/mysql.h>
#include<unistd.h>
#include<iostream>
#include<string>
#include<string.h>
using namespace std;
#define  NOW_DATABASE "first_test"
class MY_SQL_CONNECT
{
    public:
        static int mysql_time;
    public:
        MY_SQL_CONNECT()=default;
        MY_SQL_CONNECT(MYSQL *mysqlx,MYSQL * my_resx)
        {
            mysql=mysqlx;
            my_res=my_resx;
        }
        ~MY_SQL_CONNECT()
        {
        }
    public:
        int MYSQL_CONNECT()
        {
            my_res=mysql_real_connect(mysql,"localhost","root","123456",NOW_DATABASE,3306,NULL,0); 
            if(my_res==NULL)
            {
                return 0;
            }
            return 1;
        }
        int MYSQL_INIT()
        {
            mysql=mysql_init(NULL);
            if(mysql==NULL)
            {
                return 0;
            }
            return 1;
        }
        MYSQL *Get_mysql()
        {
            return mysql;
        }
    public:
        MYSQL * mysql=NULL;
        MYSQL * my_res=NULL;
};
class MY_SQL_CREATE :virtual public MY_SQL_CONNECT
{
    public:
        MY_SQL_CREATE()=default;
        MY_SQL_CREATE(MYSQL * mysql,MYSQL * my_res): MY_SQL_CONNECT(mysql,my_res){}
        ~MY_SQL_CREATE(){}
};
class MY_SQL_QUERY_CLASS:virtual public MY_SQL_CONNECT 
{
    public:
        MY_SQL_QUERY_CLASS()=default;
        MY_SQL_QUERY_CLASS(MYSQL * mysql,MYSQL * my_res):MY_SQL_CONNECT(mysql,my_res){}
        ~MY_SQL_QUERY_CLASS(){}
        int MYSQL_QUERY(string str)
        {
            if(mysql_query(mysql,str.c_str()))
            {
                return 0;
            }
            return 1;
        }
        MYSQL_RES * MYSQL_STORE_RES()
        {
            //取回mysql_query得到的结果集
            MYSQL_RES * res;
            res =mysql_store_result(mysql);
            if(res!=NULL)
            {
                //有结果集:
                //对结果集进行处理
                //交给后续吧
                //讲结果集返回给外部全局变量的结果集
                return res;
            }
            return NULL;
        }
        //将外部全局的结果集拿进来进行处理
        int MYSQL_PRINT_RES(MYSQL_RES * res)
        {
            //结果集的每一行
            MYSQL_ROW row;
            //结果集的表头
            MYSQL_FIELD  * fields;
            //结果集中某一行的第几个字段
            unsigned int num_fields=0;
            //结果集表头列数确定
            num_fields=mysql_num_fields(res);
            //获取字段名称结构体
            fields=mysql_fetch_fields(res);
            //表头打印完成

            for(unsigned long int i=0;i<num_fields;i++)
            {
                printf("%10s",fields[i].name);
                if(i!=num_fields-1)
                    printf("\t\t");

            }
            cout<<endl;
            //开始打印表格数据
            while((row=mysql_fetch_row(res)))
            {
                if(!*row){
                    cout<<"查询结果为空"<<endl;
                    return 0;
                }
                for(unsigned long int x=0;x<num_fields;x++)
                {
                    printf("%10s",row[x]);    
                    if(x!=num_fields-1)
                        printf("\t\t");
                }
                printf("\n");
            }
            //显示收到此次影响的行数
            printf("%ld row in set ",(long)mysql_affected_rows(mysql));
            return 1;
        }
        void MYSQL_FREE_RESULT(MYSQL_RES * res)
        {
            mysql_free_result(res);
        }
};
//这个类的返回值都是mysql_query的返回值 0true 
class MY_SQL_TRANSCATION:virtual public MY_SQL_CONNECT
{
    public:
        MY_SQL_TRANSCATION(MYSQL * mysql,MYSQL * my_res):MY_SQL_CONNECT(mysql,my_res){}
        ~MY_SQL_TRANSCATION(){}
        MY_SQL_TRANSCATION()=default;
    public:
        //开启事务
        int MYSQL_OPENTRANSCATION()
        {
            int ret=0;
            ret=mysql_query(mysql,"start transaction");
            if(ret !=0)
            {
                return ret;
            }
            return ret;
        }
        //设置为手动提交
        int MYSQL_HAND_COOMIT()
        {
            int ret=0;
            ret=mysql_query(mysql,"set autocommit=0");
            if(!ret)
            {
                return ret;
            }
            return ret;
        }
        // 设置为自动提交
        int MYSQL_AUTO_COOMI()
        {
            int ret=0;
            ret=mysql_query(mysql,"set autocommit=1");
            if(!ret)
            {
                return ret;
            }
            return ret;
        }
        //彻底回滚
        int MYSQL_ROLLBACK()
        {
            int ret=0;
            ret=mysql_query(mysql,"rollback");
            if(ret==0)
            {
                return ret;
            }
        }
        int MYSQL_ROLLBACK(const char* str)
        {
            string savepoint(str);
            string roll_back("rollback");
            roll_back+=" ";
            roll_back+=savepoint;
            int ret= mysql_query(mysql,roll_back.c_str());
        }
        //commit命令
        int MYSQL_COMMIT()
        {
            int ret=0;
            ret=mysql_query(mysql,"commit");
            if(ret)
            {
                return ret;
            }
        }
};
class MY_SQL_DEFINE:virtual public MY_SQL_CONNECT 
{
    public:
        MY_SQL_DEFINE()=default;
        MY_SQL_DEFINE(MYSQL  * mysql, MYSQL * my_res):MY_SQL_CONNECT(mysql,my_res){}
        ~MY_SQL_DEFINE()
        {
            if(stmt)
            {
                mysql_stmt_close(stmt);
            }
            else{
            }
            if(define_select_result)
            {
                mysql_free_result(define_select_result);
            }
            else{
            }
        }
        //创建stmt句柄
        int MYSQL_STMT_INIT()
        {
            MYSQL_STMT * a;
            a=mysql_stmt_init(mysql);
            if(!a)
            {
                return 0;
            }
            stmt=a;
            //mysql_stmt_close(a);
        }
        //再写一个重载
        //上面是用来自己创建自己玩的
        //下面这个重载,只是为了 新建一个对象,避免重复连接,
        //不用连接数据库,直接init就行了
        int MYSQL_STMT_INIT(MYSQL * mysqlxx)
        {
            MYSQL_STMT * a;
            a=mysql_stmt_init(mysqlxx);
            if(!a)
            {
                return 0;
            }
            stmt=a;
            //mysql_stmt_close(a);
        }
        int MYSQL_STMT_PREPARE(string str,unsigned long  a)
        {
            if(mysql_stmt_prepare(stmt,str.c_str(),a))
            {
                return 0;
            }
            return 1;
        }
        void MYSQL_STMT_PREPARE_COUNT()
        {
            unsigned int a;
            a= mysql_stmt_param_count(stmt);
            param_count=a;
        }
        //属性补全之后,就要开始填充信息了
        //
        //
        //信息填充完毕,就直接发送给服务器
        int MYSQL_STMT_EXECUTE()
        {
            if (mysql_stmt_execute(stmt))
            {
                return 0;
            }
            return 1;
        }
    public:
        /*25.2.7.1. mysql_stmt_affected_rows()
         * my_ulonglong mysql_stmt_affected_rows(MYSQL_STMT *stmt) 
         *
         * 描述
         *
         * 返回上次执行语句更改、删除或插入的总行数。对于UPDATE、DELETE或INSERT语句，可在mysql_stmt_execute()之后立刻调用它们。对于SELECT语句，mysql_stmt_affected_rows()的工作方式类似于mysql_num_rows()。
         *
         */
        //取回本次操作影响行数
        int MYSQL_STMT_AFFECTED_ROWS()
        {
            my_ulonglong t;
            t= mysql_stmt_affected_rows(stmt);
            if(t==-1)
            {
                return 0;
            }
            affected_rows=t;
            printf("total affected rows : %lu\n",(unsigned long )affected_rows);
            return 1;
        }
        int MYSQL_STMT_BIND_PARAM(int a)
        {
            if (mysql_stmt_bind_param(stmt, &bind[a]))
            {
                return 0;
            }
            return 1;
        }
        int MYSQL_STMT_BIND_PARAM()
        {
            if (mysql_stmt_bind_param(stmt, &bind[0]))
            {
                return 0;
            }
            return 1;
        }
    public:
        //select语句在stmt_prama之后产生结果集
        //先计算出sql语句中有几个参数
        //在针对结果集进行操作
        // 
        void MYSQL_DEFINE_select_heard_get()
        { 
            define_select_result= mysql_stmt_result_metadata(stmt);
            define_select_fields=mysql_fetch_field(define_select_result);
            define_select_fields_num=mysql_num_fields(define_select_result);
        }
        //打印前的准备
        int MYSQL_DEFINE_PRINT_SELECT_PREPARE()
        {
            //我们默认列属性的字段已经填充完成
            //机, select相关变量已经填充完成
            //进行输出缓冲区的绑定
            bind.shrink_to_fit();
            if (mysql_stmt_bind_result(stmt, &bind[0]))
            {
                return 0;
            }
            /*对于成功生成结果集的所有语句
             *  （SELECT、SHOW、DESCRIBE、EXPLAIN
             *   ，而且仅当你打算对客户端的全部结果集进行缓冲处理时
             *    ，必须调用mysql_stmt_store_result()，
             *     以便后续的mysql_stmt_fetch()调用能返回缓冲数据*/
            mysql_stmt_store_result(stmt);
            return 1;
        }
    public:
        void INIT_BIND()
        {
            bind.clear();
        }
    public:
        //操作句柄
        MYSQL_STMT * stmt=NULL;
        //一个数组用来存储参数
        vector<MYSQL_BIND> bind;
        my_ulonglong affected_rows=0;
        //这个是用来返回预处理sql语句中的参数的数目
        int param_count=0;
        //全部使用数组,是为了防止用多个相同类型的数据要填充
        short small_data[9];
        int int_data[9];
        //字符串缓冲区
        char  str_data[9][200];
        //缓冲区的实际大小
        unsigned long real_leng[9];
        my_bool is_null[9];
        //
        //一下是针对于select的预处理的所使用的道德变量
        vector<my_bool> select_is_null;
        vector<unsigned long>select_length;
        //stmt_prama之后产生的结果集
        MYSQL_RES     *define_select_result=NULL;
        //对上面这个结果集进行fetch_field操作之后产生的表头
        MYSQL_FIELD   *define_select_fields=NULL;
        int    define_select_fields_num;//表头有多少列
        MYSQL_ROW define_select_row;
};
class MY_SQL_ALLRIGHT:public MY_SQL_CREATE,public MY_SQL_QUERY_CLASS,public MY_SQL_TRANSCATION,public MY_SQL_DEFINE
{
    public:
        MY_SQL_ALLRIGHT(MYSQL * mysql,MYSQL * my_res):MY_SQL_CREATE(mysql,my_res),MY_SQL_QUERY_CLASS(mysql,my_res),MY_SQL_TRANSCATION(mysql,my_res),MY_SQL_DEFINE(mysql,my_res){}
        ~MY_SQL_ALLRIGHT(){}
};
int  MY_SQL_CONNECT::mysql_time=1;
#define INSERET_ALL_INTO_test_add "insert into test_add (id,name) values(?,?)"
#define SELECT_ALL_test_add "select id,name from test_add"
#define INSETR_INTO_STU_COUNT_PASSWD "insert into stu_count_passwd (id,passwd) values(?,?)"
int stu_login_success_after(int whoami,MYSQL &pp);
void find_all_lessones(MYSQL &pp);
void find_sort_lessones(MYSQL &pp);
int after_find_lessones(MYSQL &pp,int whoami);
int commit_my_choose_lesson(MYSQL & pp,int whoami);
//return 1 返回上一级菜单
//return 0 返回到主菜单
int commit_my_choose_lesson(MYSQL & pp,int whoami)
{
    int ctrl=0;
    MYSQL * mysql;
    MYSQL * my_res;
    MY_SQL_ALLRIGHT a(mysql,my_res);
    a.mysql=&pp;
    string select_course("select course_sort,course_name from course where course_id=");
    string course_name;
    string course_sort;
    while(!ctrl)
    {

        cout<<"请输入你想选择的课程的编号:";
        int lesson_id;
        cin>>lesson_id;
        //在course表里查询信息,如果没有查找到,就是没有这门课
        //你就必须要重新查看
        //下面这个if 里面有打印语句,是因为我在这个函数接上了上一级菜单
        select_course+=to_string(lesson_id);
        if(!a.MYSQL_QUERY(select_course))
        {
            cout<<"请输入正确的课程编号"<<endl;
            cout<<"课程号: "<<lesson_id<<"未查询到"<<endl;
            cout<<"请重新输入课程号"<<endl;
            continue;
        }
        else
        {
            //有这门课
            //我要提前获得这门课的基本信息 id name sort 方便后面直接插入history这张表
            MYSQL_RES * res;
            res=a.MYSQL_STORE_RES();
            MYSQL_ROW row;
            row=mysql_fetch_row(res);
            stringstream ss;
            if(!*row){
                cout<<"你输入的课程id有误"<<endl;
                cout<<"即将重新输入"<<endl;
                continue;

            }
            ss<<row[0];
            cout<<"here"<<endl;
            ss>>course_sort;
            ss.str("");
            ss.clear();
            ss<<row[1];
            ss>>course_name;
            ss.str("");
            ss.clear();
            //在提交选课的时候,我们的学生历史选课表里面的信息
            //和班级无关
            //只有课程id课程种类,和课程编号
            //
            int ctrl2=0;
            //这个循环,是用来让学生溪红心输入自己想选择的班级信息的
            //考虑到的学生在输入是会出各种错误
            //或者在最后提交选课的时候,被别人抢到了
            while(!ctrl2)
            {
                cout<<"这是该课程所开设的班级信息."<<endl;
                cout<<"请选择你的班级"<<endl;
                string select_head("select * from ");
                string select_table_name=to_string(lesson_id);
                select_table_name+="_class_info ";
                string select_course_class_info=select_head;
                select_course_class_info+=select_table_name;
                string course_class_table_name=to_string(lesson_id)+"_class_info ";
                //打印一下这个表
                //上面这个goto点,是为了下面好多不符合操作的输入,能够返回到选班的这里
                a.MYSQL_QUERY(select_course_class_info);
                //临时用一下
                MYSQL_RES  *res_x;
                res_x=a.MYSQL_STORE_RES();
                a.MYSQL_PRINT_RES(res_x);
                a.MYSQL_FREE_RESULT(res_x);
                cout<<"查询班级表的语句i:"<<select_course_class_info<<endl;
                cout<<endl<<endl;
                int class_id=0;
                cout<<"请输入你所选择班级的编号:"<<endl;
                cin>>class_id;
                string select_class_info("select id,remain_num from ");
                select_class_info+=select_table_name;
                select_class_info+=" where id=";
                select_class_info+=to_string(class_id);
                cout<<"选择班级idi:"<<select_course_class_info<<endl;
                cout<<endl<<endl;
                a.MYSQL_QUERY(select_class_info);
                if(((my_ulonglong)~0)!= mysql_affected_rows(a.mysql))
                {
                    cout<<"请输入正确的班级编号"<<endl;
                    cout<<"班级号: "<<lesson_id<<"未查询到"<<endl;
                    cout<<"请重新输入班级"<<endl;
                    continue;
                    //这个continue是为了回到ctrl2
                    //那个循环
                    //这样就可以重复输入班级id
                }
                else{
                    cout<<"班级号正确: 正在检查课程余量"<<endl;
                    MYSQL_RES *res;
                    res=a.MYSQL_STORE_RES();
                    cout<<"正在比对余量"<<endl;
                    MYSQL_ROW row;
                    row=mysql_fetch_row(res);
                    int after_select_class_id=0;
                    int after_select_class_remain_num=0;
                    stringstream ss;
                    ss<<row[0];
                    ss>>after_select_class_id;
                    ss.str("");
                    ss.clear();
                    ss<<row[1];
                    ss>>after_select_class_remain_num;
                    ss.str("");
                    ss.clear();
                    if(after_select_class_remain_num<=0)
                    {
                        cout<<"当前班级余量为零:  请另选班级"<<endl;
                        int ctrl3=0;
                        while(!ctrl3)
                        {
                            char return_back;
                            cout<<endl<<endl<<endl;
                            cout<<"是否返回上一级菜单?(y/n):";
                            cin>>return_back;
                            switch(return_back)
                            {
                                case 'y':
                                    {
                                        return 1;
                                    }
                                case 'n':
                                    {
                                        ctrl3=1;
                                        //我在这里默认这个函数返回值是1时,我们回到上一级菜单
                                        return 1;
                                    }
                                default:
                                    {
                                        cout<<"请规范输入"<<endl;
                                        break;
                                    }
                            }
                        }
                        continue;
                    }
                    else
                    {
                        //开启事务
                        a.MYSQL_OPENTRANSCATION();
                        //设置为手动提交
                        a.MYSQL_HAND_COOMIT();
                        string update_coures_class_info="update ";
                        update_coures_class_info+=course_class_table_name;
                        update_coures_class_info+=" set remain_num= remain_num-1 ";
                        //这里是为了去抢课的余量
                        if(!a.MYSQL_QUERY(update_coures_class_info))
                        {
                            cout<<"选课失败:"<<endl;
                            cout<<"即将回到,选课界面"<<endl;
                            sleep(1);
                            cout<<endl<<endl;
                            //选课失败,把事务关掉,不能影响外面的操作;
                            a.MYSQL_ROLLBACK();
                        }
                        else 
                        {
                            //选上了,但是还是没有提交事务
                            //开始问你是否提交
                            //如果提交
                            //就把课抢了,顺带加到历史里面去
                            //如果没有提交,就不加
                            cout<<"恭喜:你已经抢到课程,课程编号为: "<<lesson_id<<endl;
                            cout<<"提交之后,将保存到历史选课中,退出改系统后,选课有效"<<endl;
                            cout<<"如果不提交,退出后,本次选课无效"<<endl;
                            cout<<endl<<endl;
GOTO_POINT_ctrl:         cout<<"是否提交? y/n/r(返回主菜单): "<<endl;
                         char commit_ctrlx;
                         cin>>commit_ctrlx;
                         cout<<"commit"<<commit_ctrlx<<endl;
                         switch(commit_ctrlx)
                         {
                             case 'y':
                                 {
                                     string commit_before_inseret_into_history;
                                     commit_before_inseret_into_history+="insert into ";
                                     commit_before_inseret_into_history+=to_string(whoami);
                                     commit_before_inseret_into_history+="_choose_history ";
                                     commit_before_inseret_into_history+=" (course_sort,course_id,course_name) ";
                                     commit_before_inseret_into_history+=" values";
                                     commit_before_inseret_into_history+="(";
                                     commit_before_inseret_into_history+="\'";
                                     commit_before_inseret_into_history+=course_sort;
                                     commit_before_inseret_into_history+="\'";
                                     commit_before_inseret_into_history+=",";
                                     commit_before_inseret_into_history+=to_string(lesson_id);
                                     commit_before_inseret_into_history+=",";
                                     commit_before_inseret_into_history+="\'";
                                     commit_before_inseret_into_history+=course_name;
                                     commit_before_inseret_into_history+="\'";
                                     commit_before_inseret_into_history+=") ";
                                     if(!a.MYSQL_QUERY(commit_before_inseret_into_history))
                                     {
                                         cout<<"出错啦,对不起先生"<<endl;
                                         //回滚回去,选课历史无效
                                         a.MYSQL_ROLLBACK();
                                         //操作失败回滚之后,默认是自动提交
                                         a.MYSQL_AUTO_COOMI();
                                         return 1;
                                     }
                                     else
                                     {
                                         //事务提交
                                         a.MYSQL_COMMIT();
                                         //开启自动提交
                                         a.MYSQL_AUTO_COOMI();
                                         cout<<"选课成功,即将回到主菜单"<<endl;
                                         //出循环
                                         return 0;
                                     }     
                                     break;
                                 }
                             case 'n':
                                 {
                                     cout<<"即将返回上一级菜单"<<endl;
                                     sleep(1);
                                     cout<<endl<<endl;
                                     //回滚回去,选课历史无效
                                     a.MYSQL_ROLLBACK();
                                     //操作失败回滚之后,默认是自动提交
                                     a.MYSQL_AUTO_COOMI();
                                     //这里缺一句goto
                                     cout<<"即将返回上一级菜单"<<endl;
                                     return 1;
                                 }
                             case 'r':
                                 {
                                     a.MYSQL_ROLLBACK();
                                     a.MYSQL_AUTO_COOMI();
                                     cout<<"byebye"<<endl;
                                     return 0;
                                 }
                             default:
                                 {
                                     cout<<"请规范输入"<<endl;
                                     goto GOTO_POINT_ctrl;
                                     break;
                                 }
                         }
                        }
                    }
                }
            }
        }
    }
}
int after_find_lessones(MYSQL &pp,int whoami)
{
    int ctrl =0; 
    while(!ctrl)
    {
        cout<<endl<<endl;
        int key=0;
        cout<<"1. >进行选课<"<<endl;
        cout<<"2. >刷新查询界面<"<<endl;
        cout<<"3. >回到上一级菜单"<<endl;
        cout<<"4. >退出<"<<endl;
        cout<<endl<<"请输入你的选项"<<endl;
        cin>>key;
        switch(key)
        {
            case 1:
                {
                    commit_my_choose_lesson(pp,whoami);

                }
            case 2:
                {
                }
            case 3:
                {
                    cout<<"即将返回到上一级菜单!"<<endl;
                    sleep(2);
                    //利用这个函数的返回值进行goto
                    return 1;
                }
            case 4:
                {
                    cout<<"即将退出系统 byebye"<<endl;
                    //利用返回值进行 return0
                    return 0;
                }
            default:
                {
                    cout<<"请规范输入"<<endl;
                    ctrl=1;
                }
        } 
    }
    //2是正常退出
    return 2;
}
//return ,登录成功
//登录失败,要退出客户端
//
//   
//登陆之后的动作,全部分成小块
//return 1;返回管理员住菜单
//return 0:推出客户端
//return 2:返回最顶级的菜单
int after_root_login_1_1(int & whoami,MYSQL &pp)
{   
    MYSQL * mysql;
    MYSQL * my_res;
    MY_SQL_ALLRIGHT a(mysql,my_res);
    a.mysql=&pp;
    string str("select  * from course ");
    if(!a.MYSQL_QUERY(str))
    {
        cout<<"查询失败"<<endl;
        return 1;
    }
    MYSQL_RES *res;
    res=a.MYSQL_STORE_RES();
    if(!res)
    {
        cout<<"这条sql的结果集为空"<<endl;
        return 1;
    }
    if(!a.MYSQL_PRINT_RES(res))
    {
        cout<<"打印失败"<<endl;
        return 1;
    }
    //走到这里就是彻底成功
    //回到主菜单算了
    return 1;
}
//return 1;返回管理员住菜单
//return 0:推出客户端
//return 2:返回最顶级的菜单
int after_root_login_1_2(int &whoami,MYSQL &pp)
{
    int course_id;
    char course_sort;
    string course_name;
    cout<<"请输入新增程的课程id"<<endl;
    cin>>course_id;
    cout<<"请输入课程的名称(拒绝中文)"<<endl;
    cin>>course_name;
GOTO_SORT:
    cout<<"请输入课程的分类(A B C D四选一,统一大写)"<<endl;
    cin>>course_sort; 
    int b=course_sort-'A';
    if(b<0||b>3)
    {
        cout<<"请规范输入课程分类"<<endl;
        goto GOTO_SORT;
    }
sql语句
    //开启事务,因为我们没添加一门课就要创建一张班级表,这个是关联在一起的操作
    //
    a.MYSQL_OPENTRANSCATION();
    a.MYSQL_HAND_COOMIT();
    if(!a.MYSQL_QUERY(insert_crouse_sql))
    {
        cout<<"插入新课程失败"<<endl;
        cout<<"请检查你的课程id"<<endl;
        cout<<"即将回到上一级菜单"<<endl;
        a.MYSQL_ROLLBACK();
        a.MYSQL_AUTO_COOMI();
        return 1;
    }
    else{
        //插入成功,现在需要创建表;
        
        if(!a.MYSQL_QUERY(create_class_info_head))
        {
            cout<<"创建先关的班级表失败A"<<endl;
            cout<<"即将返回上一级菜单"<<endl;
            a.MYSQL_ROLLBACK();
            a.MYSQL_AUTO_COOMI();
            return 1;
        }
        else{
            a.MYSQL_COMMIT();
            a.MYSQL_AUTO_COOMI();
            cout<<"新增课程成功"<<endl;
            cout<<"即将返回上一级主界面"<<endl;
            return 1;
        }
    }
}
int after_root_login_1_3(int whoami ,MYSQL &pp)
{ 
    cout<<"请输入你想删除的课程的课程id"<<endl;
    int course_id=0;
    cin>>course_id;
    string dellete_sql("delete  from course where course_id=");
    dellete_sql+=to_string(course_id);
    MYSQL *mysql;
    MYSQL *my_res;
    MY_SQL_ALLRIGHT a(mysql,my_res);
    //开启事务
  
    cout<<"事务开始;了"<<endl;
    if(!a.MYSQL_QUERY(dellete_sql))
    {
        cout<<"删除语句执行失败"<<endl;
        cout<<"即将返回上一级界面"<<endl;
        a.MYSQL_ROLLBACK();
        a.MYSQL_AUTO_COOMI();
    }
    else{
        //删除课程成功,现在需要把先关的班级表也删掉
     
        if(!a.MYSQL_QUERY(sql_head))
        {
            cout<<"删除表失败"<<endl;
            cout<<"即将返回上一级菜单"<<endl;
            a.MYSQL_ROLLBACK();
            a.MYSQL_AUTO_COOMI();
            return 1;
        }
        else{
            //一切成功
            a.MYSQL_COMMIT();
            a.MYSQL_AUTO_COOMI();
            cout<<"即将返回上一级菜单"<<endl;
            return 1;
        }
    }
}
int after_root_login_2_1(int whoami,MYSQL &pp)
{
    cout<<"请输入课程id"<<endl;
    int course_id;
    cin>>course_id;
    cout<<"即将展示相关课程信息"<<endl;
    sleep(1);
   
    a.mysql=&pp;
    if(!a.MYSQL_QUERY(str_sql))
    {
        cout<<"查询语句执行失败"<<endl;
        cout<<"即将返回上一级界面"<<endl;
        return 1;
    }
    else 
    {
        MYSQL_RES *res;
        res=a.MYSQL_STORE_RES();
        if(!res)
        {
            cout<<"这条sql的结果集为空"<<endl;
            return 1;
        }
        if(!a.MYSQL_PRINT_RES(res))
        {
            cout<<"打印失败"<<endl;
            return 1;
        }
    }
    return 1;
}
int after_root_login_2_2(int whoami ,MYSQL &pp)
{

GOTO_POINT_AFTER_ROOT_LOGIN_2_2_start:
    cout<<"请输入目标课程的课程id:"<<endl;
    int course_id;
    cin>>course_id;

    cout<<"请输入你想修改那个班级的id:"<<endl;
    int class_id;
    cin>>class_id;
    cout<<"现在正在修改的班级为:"<<course_id<<"::"<<class_id<<endl;



sql

    cout<<"1. >修改指导老师<"<<endl;
    cout<<"2. >修改上课地点<"<<endl;;
    cout<<"3. >修改班级课程余量<"<<endl;
    cout<<"请输入你想修改的属性"<<endl;
    int key=0;
    cin>>key;
    switch(key)
    {
        case 1:
            {
                update_sql+="course_teacher_name= ";
                cout<<"请输入教师的名字:";
                string course_teacher;
                cin >>course_teacher;
                update_sql+="\'";
                update_sql+=course_teacher;
                update_sql+="\'";
                update_sql+=update_tail;
                cout<<"测试:"<<update_sql;
                MYSQL  *mysql;
                MYSQL * my_res;
                MY_SQL_ALLRIGHT a(mysql,my_res);
                a.mysql=&pp;
                if(!a.MYSQL_QUERY(update_sql))
                {
                    cout<<"更新失败"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }
                else{
                    cout<<"更新成功"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }
            }
        case 2:
            {

                update_sql+="course_address= ";
                cout<<"请输入教师的名字:";
                string course_address;
                cin >>course_address;
                update_sql+="\'";
                update_sql+=course_address ;
                update_sql+="\'";
                update_sql+=update_tail;
                cout<<"测试:"<<update_sql;
                MYSQL  *mysql;
                MYSQL * my_res;
                MY_SQL_ALLRIGHT a(mysql,my_res);
                a.mysql=&pp;
                if(!a.MYSQL_QUERY(update_sql))
                {
                    cout<<"更新失败"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }
                else{
                    cout<<"更新成功"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }
            }
        case 3:
            {
                update_sql+="remain_num= ";
                cout<<"请输入教师的名字:";
                int remain_num;
                cin>>remain_num;
                update_sql+=to_string(remain_num);
                update_sql+=update_tail;
                cout<<"测试:"<<update_sql;
                MYSQL  *mysql;
                MYSQL * my_res;
                MY_SQL_ALLRIGHT a(mysql,my_res);
                a.mysql=&pp;
                if(!a.MYSQL_QUERY(update_sql))
                {
                    cout<<"更新失败"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }
                else{
                    cout<<"更新成功"<<endl;
                    cout<<"即将返回上一届界面"<<endl;
                    return 1;
                }

            }
        default:
            {
                cout<<"请规范输入"<<endl;
                goto GOTO_POINT_AFTER_ROOT_LOGIN_2_2_start;

            }
    }
}
int  after_root_login_2_3(int whoami,MYSQL &pp)
{
    cout<<"你想给那门课成新增班级"<<endl;
    cout<<"请输入他的课程id"<<endl;
    int course_id ;
    cin>>course_id;
    cout<<"你现在在操作的是课程id为:"<<course_id<<"的课程"<<endl;
    cout<<"请输入新增班级的指导老师"<<endl;
    string  course_teacher_name;
    cin>>course_teacher_name;

    cout<<"请输入新增班级的上课地点"<<endl;
    string course_address;
    cin>>course_address;
    cout<<"你是否想更改班级默认的人数(课程余量)?";
    cout<<"  (y?)(非y则按照默认的30人)"<<endl;
    char k;
    cin>>k;
    string add_sql("insert into ");
    add_sql+=to_string(course_id);
    add_sql+="_class_info ";
    if(k!='y')
    {
       sql
        cout<<"这里的更新语句:"<<add_sql<<endl;
    }
    else 
    {
        int remain_num;
        cout<<"请输入班级人数"<<endl;
        cin >>remain_num;
       sql语句

    }
    MYSQL *mysql;
    MYSQL * my_res;
    MY_SQL_ALLRIGHT a(mysql,my_res);
    a.mysql=&pp;
    if(!a.MYSQL_QUERY(add_sql))
    {
        cout<<"增加失败"<<endl;
        cout<<"即将返回上一届界面"<<endl;
        return 1;
    }
    else{
        cout<<"增加成功"<<endl;
        cout<<"即将返回上一届界面"<<endl;
        return 1;
    }
}


int  after_root_login_2_4(int whoami,MYSQL &pp)
{
    cout<<"你想给那门课删除班级"<<endl;
    cout<<"请输入他的课程id"<<endl;
    int course_id ;
    cin>>course_id;
    cout<<"你现在在操作的是课程id为:"<<course_id<<"的课程"<<endl;
    int class_id ;
    cout<<"请输入你想删除的班级的id"<<endl;
    cin>>class_id;
    string  table_name;
   sql语句
    MY_SQL_ALLRIGHT a(mysql,my_res);
    a.mysql=&pp;
    cout<<"delete_sql"<<delete_sql<<endl;
    if(!a.MYSQL_QUERY(delete_sql))
    {
        cout<<"删除失败"<<endl;
        cout<<"即将返回上一届界面"<<endl;
        return 1;
    }
    else{
        cout<<"删除成功"<<endl;
        cout<<"即将返回上一届界面"<<endl;
        return 1;
    }

}
//return 0 退出客户端
//return 1返回最顶级菜单
//
int AFTER_ROOT_LOGIN(int &whoami,MYSQL &pp)
{

    cout<<"你已经成功的一管理员身份登录改系统"<<endl;
GOTO_POINT_ROOT_MANAGER: 
    cout<<"   这是管理员的主菜单"<<endl;
    cout<<"1. >对课程进行操作(增删查改)<"<<endl;
    cout<<"2. >对班级的操作(增删查改)<"<<endl;
    cout<<"3. >对学生个人信息的操作(账户密码增删查改)<"<<endl;
    cout<<"4. >管理员新增账号<"<<endl;
    cout<<"5. >开放选课(这个做不做我也不知道看进度把)<"<<endl;
    cout<<"6. >退出本客户端<"<<endl;
    cout<<"7. >返回最顶级的菜单<"<<endl;
    cout<<"!!!!!!!!! 2  3 4 5 未完成"<<endl;
    cout<<"请输入你的选项"<<endl;
    int key =0;
    cin>>key;
    switch(key)
    {
        case 1:
            {
OGOTO_POINT_AFTER_ROOT_LOGIN:           
                cout<<"这里是对课程的操作"<<endl;

                cout<<"1. >查看全部课程<"<<endl;
                cout<<"2. >增加课程<"<<endl;
                cout<<"3. >删除课程<"<<endl;
                cout<<"4. >回到上一级菜单"<<endl;
                int key=0;
                cout<<"请输入你的选项"<<endl;
                cin>>key;
                switch(key)
                {

                    case 1:
                        {
                            if(!after_root_login_1_1(whoami,pp))
                            {
                                cout<<"查询失败"<<endl;
                            }
                            else{cout<<"查询成功"<<endl;}
                            cout<<"即将回到上一级菜单";
                            goto OGOTO_POINT_AFTER_ROOT_LOGIN; 
                        }
                    case 2:
                        {
                            after_root_login_1_2(whoami,pp);
                            goto OGOTO_POINT_AFTER_ROOT_LOGIN; 
                        }
                    case 3:
                        {
                            after_root_login_1_3(whoami,pp);
                            goto OGOTO_POINT_AFTER_ROOT_LOGIN; 
                        }
                    case 4:
                        {
                            goto GOTO_POINT_ROOT_MANAGER; 
                        }
                    default:
                        {
                            cout<<"请规范输入"<<endl;
                            goto OGOTO_POINT_AFTER_ROOT_LOGIN; 
                        }
                }
                break;
            }
        case 2:
            {
GOTO_POINT_AFTER_ROOT_LOGIN_2_manger:
                cout<<"这里是对班级的操作"<<endl;
                cout<<"1. >查看魔门课程的班级<"<<endl;
                cout<<"2. >修改指定的班级的相关信息<"<<endl;
                cout<<"3. >增加班级<"<<endl;
                cout<<"4. >删除班级<"<<endl;
                cout<<"5. >回到上一级菜单"<<endl;
                int key=0;
                cout<<"请输入你的选项"<<endl;
                cin>>key;
                switch(key)
                {

                    case 1:
                        {
                            after_root_login_2_1(whoami,pp);

                            goto GOTO_POINT_AFTER_ROOT_LOGIN_2_manger;
                        }
                    case 2:
                        {
                            after_root_login_2_2(whoami,pp);
                            goto GOTO_POINT_AFTER_ROOT_LOGIN_2_manger;
                        }
                    case 3:
                        {
                            after_root_login_2_3(whoami,pp);
                            goto GOTO_POINT_AFTER_ROOT_LOGIN_2_manger;
                        }
                    case 4:
                        {
                            after_root_login_2_4(whoami,pp);
                            goto GOTO_POINT_AFTER_ROOT_LOGIN_2_manger;
                        }
                    case 5:
                        {

                            cout<<"即将返回root主菜单"<<endl;
                            goto GOTO_POINT_ROOT_MANAGER; 
                        }
                    default:
                        {
                            cout<<"请规范输入"<<endl;
                            goto GOTO_POINT_AFTER_ROOT_LOGIN_2_manger;
                        }
                }
                break;
            }
        case 3:
            {

                cout<<"z这里是对学生个人信息的操作"<<endl;
                cout<<"1. >帮助学生找回密码<"<<endl;
                cout<<"2. >将学生密码设置为初始状态"<<endl;
                cout<<"3. >回到上一级菜单"<<endl;
            }
        case 4:
            {
                cout<<"这里是管理员账号管理"<<endl;

                cout<<"提示:管理员无权影响管理员,所以只能新增管理员"<<endl;
                cout<<"1. >新增管理员<"<<endl;
                cout<<"2. >回到上一级菜单"<<endl;
            }
        case 5:
            {
                cout<<"这里是开放题选课"<<endl;
                cout<<"这里我不去耳钉写不写"<<endl;
            }
        case 6:
            {
                cout<<"即将推出本客户端"<<endl;
                sleep(2);
                return 0;
            }
        case 7:
            {
                cout<<"即将返回最顶级菜单"<<endl;
                sleep(2);
                goto GOTO_POINT_ROOT_MANAGER;
            }
        default:
            {
                cout<<"请规范输入"<<endl;
                goto  GOTO_POINT_ROOT_MANAGER; 
            }
    }
}
int PEOPLE_LOGIN(int &whoami,MYSQL &pp,string str)
{
    MYSQL *mysql;
    MYSQL * my_res;
    MY_SQL_ALLRIGHT pair_id_passwd(mysql,my_res);
    pair_id_passwd.mysql=&pp;
    int ctrl=1;
    int login_times=3;
    int id=0;
    string passwd;
    while(ctrl&&login_times)
    {
        MYSQL_RES * res;

        //string str_pair_id_passwd("select passwd from stu_count_passwd where id=");
        string str_pair_id_passwd;
        str_pair_id_passwd+="select passwd from ";
        str_pair_id_passwd+=str;
        str_pair_id_passwd+="_count_passwd where id=";

        MYSQL_ROW row_passwd_pair;
        cout<<"请输入你的账户"<<endl;
        cin>>id;
        str_pair_id_passwd+=to_string(id);
        cout<<"请输入你的密码"<<endl;
        cin>>passwd;
        login_times--;
        if(  pair_id_passwd.MYSQL_QUERY(str_pair_id_passwd))
        {
            //处理结果集的
            res=pair_id_passwd.MYSQL_STORE_RES();
            if(res != NULL)
            {
                row_passwd_pair=mysql_fetch_row(res);
                if(row_passwd_pair!=0)
                {
                    if(passwd==row_passwd_pair[0])
                    {
                        cout<<"登录成功!"<<endl;
                        whoami=id;
                        ctrl=0;
                        return 1;
                    }
                    else{
                        cout<<"密码不正确"<<endl;
                        continue;
                    }
                }
                else{
                    cout<<"账号不存在"<<endl;
                    continue;
                }
            }//pair_id_passwd.MYSQL_PRINT_RES(res);
        }
        else{
            cout<<"账号不存在"<<endl;
            continue;
        }
    }
    if(login_times==0)
    {
        cout<<"看来你忘了你的账号和密码"<<endl;
        cout<<"本沙雕客户端不提供密码找回服务,请联系管理员"<<endl;
        cout<<"byebye"<<endl;
        return 0;
    }
    return 0;
}

int main()
{
    int whoami=0;
    MYSQL * mysqle;
    MYSQL * my_res;
    MY_SQL_ALLRIGHT a(mysqle,my_res);
    a.MYSQL_INIT();
    a.MYSQL_CONNECT();
    mysqle=a.Get_mysql();
    MYSQL pp=*mysqle;
    cout<<"欢迎来到傻逼选课系统"<<endl;
GOTO_POINT:        
    cout<<"1. >学生登录<"<<endl;
    cout<<"2. >新用户(学生)注册<"<<endl;
    cout<<"3. >root 登录"<<endl;
    cout<<"4. >退出<"<<endl;
    cout<<"请输入你的选择"<<endl; 
    int key=0;
    cin>>key;
    int id;
    string passwd;
    MY_SQL_ALLRIGHT pair_id_passwd(mysqle,my_res);
    pair_id_passwd.mysql=&pp;
    switch(key)
    {
        case 1:
            {
                if(!PEOPLE_LOGIN(whoami,pp,"stu"))
                {
                    return 0;
                }
                break;
                /*   int ctrl=1;
                     int login_times=3;
                     while(ctrl&&login_times)
                     {
                     MYSQL_RES * res;

                     string str_pair_id_passwd("select passwd from stu_count_passwd where id=");
                     MYSQL_ROW row_passwd_pair;
                     cout<<"请输入你的账户"<<endl;
                     cin>>id;
                     str_pair_id_passwd+=to_string(id);
                     cout<<"请输入你的密码"<<endl;
                     cin>>passwd;
                     login_times--;
                     if(  pair_id_passwd.MYSQL_QUERY(str_pair_id_passwd))
                     {
                //处理结果集的
                res=pair_id_passwd.MYSQL_STORE_RES();
                if(res != NULL)
                {
                row_passwd_pair=mysql_fetch_row(res);
                if(row_passwd_pair!=0)
                {
                if(passwd==row_passwd_pair[0])
                {
                cout<<"登录成功!"<<endl;
                whoami=id;
                ctrl=0;
                }
                else{
                cout<<"密码不正确"<<endl;
                }
                }
                else{
                cout<<"账号不存在"<<endl;
                }
                }//pair_id_passwd.MYSQL_PRINT_RES(res);
                }
                else{
                cout<<"账号不存在"<<endl;
                }
                }
                if(login_times==0)
                {
                cout<<"看来你忘了你的账号和密码"<<endl;
                cout<<"本沙雕客户端不提供密码找回服务,请联系管理员"<<endl;
                cout<<"byebye"<<endl;
                return 0;
                }
                break;*/
            }
        case 2:
            {
              sql语句
                //列属性信息注册完成
                regst.MYSQL_STMT_BIND_PARAM();
                int ctrl =1;
                int regst_times=3;
                while(ctrl&&regst_times)
                {
                    cout<<"开始注册新用户"<<endl;
                    cout<<"请输入你的账号:";
                    int id_regst;
                    stringstream ss;
                    string str_id;
                    cin >>str_id;
                    ss<<str_id;
                    ss>>id_regst;
                    if(!id_regst)
                    {
                        cout<<"别用字母 爱你偶"<<endl;
                        continue;
                    }
                    cout<<"请输入你的密码";
                    string passwd_regst;
                    cin>>passwd_regst;
                    regst.int_data[0]=id_regst;
                    strncpy(regst.str_data[1],passwd_regst.c_str(),200);
                    regst.real_leng[1]=20;
                    regst_times--;
                    if(regst.MYSQL_STMT_EXECUTE())
                    {
                        cout<<"注册成功,即将返回登录界面"<<endl;
                        ctrl=0;
                       sql语句
                        cout<<endl<<endl<<endl;
                        goto GOTO_POINT;
                        cout<<"你已经回到登录界面,请开始登录"<<endl;
                    }
                    else 
                    {
                        cout<<"你的账户密码不正确,请重新注册"<<endl;
                    }
                }
                break;
            }
        case 3:
            {
                //登陆失败:直接结束客户端
                if(!PEOPLE_LOGIN(whoami,pp,"root"))
                {
                    return 0; 
                }
                //登录成功,进行root的后续操作
                //这个case,进来的话,就不会进行后面的操作
                if(!AFTER_ROOT_LOGIN(whoami,pp))
                {
                    return 0;
                }

                break;
            }
        case 4:
            {
                cout<<"翻滚吧 狗剩"<<endl;
                return 0;
            }
        default:
            cout<<"请输入规范选项"<<endl;
            goto GOTO_POINT;
    }
GOTO_POINT_COMMIT: 
    int ctrl_pro=0;
    ctrl_pro=stu_login_success_after(whoami,pp);
    if(ctrl_pro==0)
    {
        return 0; 
    }
    else if(ctrl_pro==3)
    {

        goto GOTO_POINT;
    }
    else
    {
        int kkk=0;
        kkk=commit_my_choose_lesson(pp,whoami);
        if(kkk==1)
        {
            goto GOTO_POINT_COMMIT;

        }
        else if(kkk==0)
        {
            goto GOTO_POINT;

        }


    }
    return 0;
}
//return 3返回主界面
//return 0 退出
////return 1 本函数退出
int stu_login_success_after(int whoami,MYSQL &pp)
{
    cout<<"欢迎"<<whoami<<"同学"<<endl;
GOTO_POINT2:
    cout<<endl<<endl;

    cout<<"1. >进行选课<"<<endl;
    cout<<"2. >查看我的选课历史<"<<endl;
    cout<<"3. >返回主界面<"<<endl;
    cout<<"4. >我要走了<"<<endl;
    int key=0;
    cin>>key;
    switch(key)
    {
        case 1:
            {
GOTO_POINT1:
                cout<<"1. >查看全部课程<"<<endl;
                cout<<"2. >产看指定分类课程<"<<endl;
                int key2=0;
                cin>>key2;
                switch (key2)
                {
                    case 1:{
                               find_all_lessones(pp);
                               return 1;
                           }
                    case 2:
                           {
                               //掉个函数
                               find_sort_lessones(pp);
                               return 1;
                           }
                    default:
                           {
                               cout<<"做人要规范"<<endl;
                               cout<<endl<<endl;
                               goto  GOTO_POINT1;
                           }
                }
                break;
            }
        case 2:
            {
                //查看选课历史
             sql语句
                goto GOTO_POINT2;
            }
        case 3:
            {
                cout<<endl<<endl<<endl;
                return 3;
                break;
            }
        case 4:
            {
                cout<<"byebye"<<endl;
                return 0;
            }
        default:
            {
                cout<<"做人要规范"<<endl;
                goto  GOTO_POINT2;
            }
    }
    cout<<"大的switch已经出来了"<<endl;
    return 1;
}
void find_sort_lessones(MYSQL &pp)
{
    char sort;
GOTO_POINT3:
    cout<<" 请输入要查询的分类: A  B C  D "<<endl;
    cin>>sort;
    int judge=sort-'A';
    if(judge>4||judge<0)
    {
        cout<<"请规范输入"<<endl;
        goto GOTO_POINT3;
    }
    string find_sort_lessones(" select * from course where course_sort='");
    find_sort_lessones+=sort;
    find_sort_lessones+="'";
    MYSQL * mysql;
    MYSQL * my_res;
    MYSQL_RES * res;
    MY_SQL_ALLRIGHT find_course(mysql,my_res);
    find_course.mysql=&pp;
    find_course.MYSQL_QUERY(find_sort_lessones);
    res=find_course.MYSQL_STORE_RES();
    find_course.MYSQL_PRINT_RES(res);
    find_course.MYSQL_FREE_RESULT(res);
}    
void find_all_lessones(MYSQL &pp)
{
    string find_all_lessones(" select * from course");
    MYSQL * mysql;
    MYSQL * my_res;
    MYSQL_RES * res;
    MY_SQL_ALLRIGHT find_course(mysql,my_res);
    find_course.mysql=&pp;
    find_course.MYSQL_QUERY(find_all_lessones);
    res=find_course.MYSQL_STORE_RES();
    find_course.MYSQL_PRINT_RES(res);
    find_course.MYSQL_FREE_RESULT(res);
}    
