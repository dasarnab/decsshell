#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
int child = -1;

void handle_sigint(int sig){
    kill(child,SIGSTOP);
    char ch;
    char buff[]="the program is interrupted, do you want to exit [Y/N]\n";
    write(STDOUT_FILENO,buff,sizeof(buff)+1);
    scanf("%c",&ch);
    //printf("%c\n",ch);
    fflush(stdin);
    if(ch == 'Y'){
        kill(child,SIGKILL);
        exit(2);
    }
    fflush(stdout);
    kill(child,SIGCONT);
    return;
}
int main(){
    int pid = fork();
    if(pid < 0){
        perror("fork error\n");
        exit(-1);
    }
    child = pid;
    printf("[%d]:I'am parent\n",getpid());
    if(pid == 0){
    printf("[%d]I'm child\n",getpid());
    signal(SIGINT,SIG_IGN);
    while(1){
        printf("[%d]: Enter Some input:",getpid());
        char str[128];
        scanf("%s",str);
        printf("here is what was given : %s\n",str);
    
    }
    }else{
        struct sigaction sa;
        sa.sa_handler = &handle_sigint;
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT,&sa,NULL);
        while(1);
        while(wait(&pid) < 0);
        wait(&pid);

            
    }
}
