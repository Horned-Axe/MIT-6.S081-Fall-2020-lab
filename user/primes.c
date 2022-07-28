#include "../kernel/types.h"
#include "../user/user.h"

int create_griddle(int fd[],int level);

int main()
{
    int fd[2],retpid;

    if(pipe(fd)==-1){
        fprintf(2,"pipe error!");
        exit(1);
    }

    retpid=fork();
    if(retpid==0){
        //要怎样呢...在函数中派生子进程吗...全在主程序的话命名我会觉得有点不好处理...
        create_griddle(fd,1);
        wait(0);//
        exit(0);
    }
    else if(retpid>0){
        close(fd[0]);//关闭管道读端
        for(int i=2;i<=35;i++){
            write(fd[1],&i,sizeof(i));//?
        }
        close(fd[1]);
        wait(0);//
        exit(0);
    }
    else{
        fprintf(2,"fork error!");
        exit(1);
    }
}

//创建一层素数滤网
//fd是与上层相连的通道
//返回创建的下一层滤网所在进程的pid，若为0则无创建
int create_griddle(int fd[],int level)
{
    close(fd[1]);
    int base,num;
    if(read(fd[0],&base,sizeof(base))==0){
        return 0;//这个应当不会空，因为在有需要时才创建//但还是判断一下吧(   //“心于何处”...
    }
    fprintf(2,"prime %d\n",base);//!!!fprintf(2,"prime %d\n",&base)是错的!!!不是地址!!!???

    //读到第一个滤过筛网的数，就跳出循环并创建下一层滤网；否则返回
    while(1){
        int tmpret;
        tmpret=read(fd[0],&num,sizeof(num));//“心于何处”...
        if(tmpret==0){
            return 0;
        }
        else if(num%base!=0){
            break;
        }
    }
    int next_fd[2];
    if(pipe(next_fd)==-1){
        fprintf(2,"pipe error!");
        return -1;
    }

    int retpid=fork();
    if(retpid==0){
        //进入下一层滤网
        return create_griddle(next_fd,level+1);
    }
    else if(retpid>0){
        close(next_fd[0]);//关闭管道读端
        do{
            //进行筛选
            if(num%base!=0){
                write(next_fd[1],&num,sizeof(num));
            }
                
        }while(read(fd[0],&num,sizeof(num))!=0);
        close(next_fd[1]);
        return retpid;
    }
    else{
        fprintf(2,"fork error!");
        return -1;
    }
}