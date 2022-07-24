#include "../kernel/types.h"
#include "../user/user.h"

int main()
{
    //好有道理！可以有两个不同方向的管道啊
    int child_fd[2];//child导向father的管道
    int father_fd[2];//father导向child的管道
    char buffer[128]={'\0'};
    if((pipe(child_fd)==-1)&&(pipe(father_fd)==-1)){//创建管道//暂且当作-1时错误吧...
        fprintf(2,"pipe error!");
        exit(1);
    }

    int respid=fork();//创建子进程
    if(respid==0){
        //in child procedure
        int pid_t=getpid();
        close(father_fd[1]);
        close(child_fd[0]);

        read(father_fd[0],buffer,sizeof(buffer));
        fprintf(2,"%d:received %s",pid_t,buffer);

        char *words="pong";
        write(child_fd[1],words,strlen(words)+1);
        exit(0);
    }
    else if(respid>0){
        //in father procedure
        int pid_t=getpid();
        close(father_fd[0]);
        close(child_fd[1]);

        char *words="ping";
        write(father_fd[1], words, strlen(words) + 1); 

        read(child_fd[0],buffer,sizeof(buffer)); 
        fprintf(2,"%d:received %s",pid_t,buffer);
        exit(0);
    }
    else{
        fprintf(2,"fork error!");
        exit(1);
    }
}