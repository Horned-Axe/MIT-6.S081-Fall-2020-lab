#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/fs.h"
#include "../kernel/param.h"

char* organizeLine(char *buf);
int organizeArgs(char *lines,char *args[MAXARG],int startpos);

int main(int argc,char*argv[])
{
    //关于管道...重定向...输入...
    //因此...那不就是获取输入吗...

    char *cmd;
    char *lines[MAXARG];
    char *args[MAXARG];
    char buf[MAXPATH];

    if(argc<2){
        //默认命令是echo
        cmd="echo";
    }
    else{
        cmd=argv[1];
    }

    //读取每行内容并作为参数列表存储，记录行数
    int lines_num=0,ret;
    while(1){
        memset(buf, 0, sizeof(buf));
        gets(buf, MAXPATH);//得到每行输入//从ulib.c源码来看，gets函数保留了\r和\n
        char *tmp = organizeLine(buf);//去除每行输入中的\n，并得到一个新分配的地址

        if(strlen(tmp) == 0 || lines_num >= MAXARG){
            break;
        }
        lines[lines_num]= tmp;
        lines_num++;
    }

    //将每一行输入内容拆分为独立参数
    
    //整理已有参数
    int arg_start_pos=0;
    for(int i=2;i<argc;i++){
        args[i-2]=argv[i];
        arg_start_pos++;
    }
    args[arg_start_pos]=0;
    //拆分得到输入参数   //本处未进行优化，因此所有行中的参数放至一起，而不拆分为多个命令
    int endpos=arg_start_pos;
    for(int line=0;line<lines_num;line++){
        endpos=organizeArgs(lines[line],args,endpos);
    }

    //释放lines和args中的空间   //虽然习惯放后面一点，但是这儿已经不需要，先释放也可以节省资源
    for(int i=0;i<lines_num;i++){
        if(lines[i]==0)
            free(lines[i]);
    }
    for(int i=arg_start_pos;i<endpos;i++){
        if(args[i]==0)
            free(args[i]);
    }

    //运行
    int retpid;
    retpid=fork();//为啥要用fork啥的啊...
    if(retpid == 0){   
        exec(cmd, args);    
    }  
    else
    {
        wait();
    }

    exit(0);
}

//返回最后一个之后的位置，即被设为0的位置
int organizeArgs(char *lines,char *args[MAXARG],int startpos)
{
    int i=0, cc;
    char c;
    char buf[MAXPATH];
    int buftop=0;

    int len=strlen(lines);
    int pos=startpos;
    int tmp;
    char*arg;
    while(pos<MAXARG){
        memset(buf, 0, sizeof(buf));
        for(; i < len; i++){
            cc = lines[i];
            if(cc==' '){
                tmp=strlen(buf);
                if(tmp>0){
                    arg=malloc((tmp+1)*sizeof(char));
                    if(arg==0){
                        fprintf(2,"No Enough Space for malloc!\n");
                        exit(1);
                    }
                    strcpy(arg,buf);
                    args[pos++]=arg;
                    args[pos]=0;  

                    memset(buf, 0, sizeof(buf));//清理buf
                    buftop=0;  
                }
            }
            else{
                buf[buftop]=lines[i];
                buftop++;
            }
        }
        if(i==len)
            break;
    }
    return pos;
}

/* 剔除"\n"并分配空间 */
char* organizeLine(char *buf){
    /* 为char *新分配了地址空间，后续要记得释放 */
    if(strlen(buf) > 1 && buf[strlen(buf) - 1] == '\n'){
        char *subbuff = (char*)malloc(sizeof(char) * (strlen(buf) - 1));
        if(subbuff==0){
            fprintf(2,"No Enough Space for malloc!\n");
            exit(1);
        }
        substring(buf, subbuff, 0, strlen(buf) - 1);
        return subbuff;
    }
    else
    {
        char *subbuff = (char*)malloc(sizeof(char) * strlen(buf));
        if(subbuff==0){
            fprintf(2,"No Enough Space for malloc!\n");
            exit(1);
        }
        strcpy(subbuff, buf);
        return subbuff;
    }
}
