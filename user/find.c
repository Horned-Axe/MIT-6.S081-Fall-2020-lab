#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char*path,char*name);

int main(int argc, char *argv[])
{
    int i;

    if(argc < 2){
        fprintf(2,"find: Parameters are not enough\n");
        exit(1);
    }
    if(argc==2){
        find(".",argv[1]);
        exit(0);
    }
    else{
        find(argv[1],argv[2]);
        exit(0);
    }
}

//得到无前置路径的白名()
char* fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void find(char*path,char*name)
{//或许是因为文件夹这儿也是表示以一个“文件”...
    static char buf[512];//避免递归中调用过多
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){//fd或许是一种类似于标识、接口之类的编号...
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
        case T_FILE: //如果是文件，则进行比较
            if(strcmp(fmtname(path),name)==0)
                printf("%s",path);

        case T_DIR: //如果是目录，则进入进行查找
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            find(buf,name);
            }
            break;
    }
    close(fd);
    return;
}