#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int no,time=0,res;

    if(argc<=1){
        //错误提示参考Ubuntu系统sleep错误提示，并加上Usage
        fprintf(2,"sleep: missing operand\nUsage: sleep time(int)\n");//为啥是2啊...输出到屏幕的意思吗...
        exit(1);//为啥main要用exit退出？
    }

    //看Ubuntu中sleep的运行情况，当输入参数多于1个时，似乎会sleep各参数的累加时间
    for(no=1;no<argc;no++){
        //问题：使用atoi没办法判断"0"和"abcd"的错误输入...
        if((res=atoi(argv[no]))==0){
        fprintf(2,"sleep: invalid time interval %s\n",argv[no]);//为啥是2啊...输出到屏幕的意思吗...
        exit(1);
        }
        time+=res;
    }

    //fprintf(2,"sleeping %d ticks",&time);
    sleep(time);
    exit(0);
}