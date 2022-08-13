#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
 
int main() {
     
    pid_t childPid;
    int status;
     
    childPid = fork();
     
    std::cout << "WCONTINUED : " << WCONTINUED <<std::endl;
    std::cout << "WNOHANG    : " << WNOHANG <<std::endl;
    std::cout << "WUNTRACED  : " << WUNTRACED <<std::endl;


    if(childPid > 0) {  // 부모 프로세스
        int ret;
        printf("부모 PID : %ld, pid : %d\n",(long)getpid(), childPid);
        sleep(3);
        ret = waitpid(childPid,&status,WNOHANG);  // WNOHANG 옵션을 사용하면 자식 프로세스가 종료되지 않았을 경우 blocking 되지 않고 즉시 0값을 리턴합니다.
         
        printf("부모 종료 %d %d\n",ret,WIFEXITED(status));
        exit(0);
    }
    else if(childPid == 0){  // 자식 코드
        printf("자식 시작 PID : %ld\n", (long)getpid());
        sleep(8);
         
        printf("자식 종료\n");
        exit(0);
    }
    else {  // fork 실패
        perror("fork Fail! \n");
        return -1;
    }
     
    return 0;
}
