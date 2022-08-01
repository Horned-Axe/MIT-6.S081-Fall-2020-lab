#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){//1.确保提供参数多于2个 2.确保第一个提供参数的首位是数字
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }
  
  for(i = 2; i < argc && i < MAXARG; i++){//可参考的exec调用前置流程 //前面差不多也是这样的，不过没判断 i < MAXARG
    nargv[i-2] = argv[i];
  }
  exec(nargv[0], nargv);
  exit(0);
}
