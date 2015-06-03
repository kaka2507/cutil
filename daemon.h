#ifndef DAEMON_H
#define DAEMON_H

#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <signal.h>  
#include <syslog.h>  
#include <fcntl.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>  

void SigHandler(int signo) {
    printf("a signal %d is catched", signo);
    exit(0);
}  

void Daemonize(const char *cmd)  
{  
    int fd;
    pid_t pid;  
    struct sigaction sa;  
    umask(0); //Set file creation mask to zero  
  
    //Become a session leader  
    if ((pid = fork()) < 0) {  
      printf("%s: can't fork", cmd);  
      exit(0);  
    }  
    else if (pid != 0) /* parent */  
        exit(0);  
    setsid();

    // Ignore signals that may cause an unexpected exit due to external errors.
    if (signal(SIGINT, SigHandler) == SIG_ERR)
        printf("can't catch SIGINT\n");
    if (signal(SIGHUP, SigHandler) == SIG_ERR)
        printf("can't catch SIGHUP\n");
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        printf("can't catch SIGPIPE\n");
    if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)
        printf("can't catch SIGTTIN\n");
    if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)
        printf("can't catch SIGTTOU\n");
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
        printf("can't catch SIGCHLD\n");
    if (signal(SIGTERM, SigHandler) == SIG_ERR)
        printf("can't catch SIGTERM\n");
    if (signal(SIGUSR1, SigHandler) == SIG_ERR)
        printf("can't catch SIGUSR1\n");
  
    //Ensure future opens won't allocate controlling TTYs.  
    sa.sa_handler = SIG_IGN;  
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = 0;  
    if (sigaction(SIGHUP, &sa, NULL) < 0) {  
        printf("Can't ignore SIGHUP");  
        exit(0);
    }  

    if ((pid = fork()) < 0) {
        printf("%s: can't fork", cmd);  
        exit(0);
    }
    else if (pid != 0) /* parent */  
        exit(0);
    
    fd = open(cmd,O_RDWR, 0);  
  
    if (fd != -1) {
        dup2 (fd, STDIN_FILENO);  
        dup2 (fd, STDOUT_FILENO);  
        dup2 (fd, STDERR_FILENO);  
        if (fd > 2) {
            close (fd);  
        }  
    }
  
    /* 
     * Change the current working directory to the root so 
     * we won't prevent file systems from being unmounted. 
     */ 
	/*
    if (chdir("/") < 0)  
    {  
        printf("Can't change directory to /");  
        exit(0);  
    } 
	*/	
  
    // Reset number of open files
    rlimit rlim;
    rlim.rlim_cur = 120000;
    rlim.rlim_max = 120000;
    setrlimit(RLIMIT_NOFILE, &rlim);
}  

#endif // DAEMON_H