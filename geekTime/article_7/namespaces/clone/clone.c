/**
 * 这个程序来自耗子叔的blog: https://coolshell.cn/articles/17010.html
 * 自己手动打一遍以便记住,所以叫copyer
 * @copyer wangjun
 */
#define _GNU_SOURCE
//why define _GNU_SOURCE https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];
char* const container_args[] = {
	"/bin/bash",
	NULL
};

int container_main(void* arg)
{
	printf("Container - inside the container!\n");
	/*直接执行一个shell，以便我们观察这个进程空间里面的资源是否被隔离了*/
	/*http://man7.org/linux/man-pages/man3/exec.3.html*/
	/*https://stackoverflow.com/questions/32142164/how-to-use-execv-system-call-in-linux*/
	execv(container_args[0], container_args);
	printf("Something`s wrong!\n");
	return 1;
}

int main()
{
	printf("Parent - start a container!\n");
	/*调用clone函数，其中传出一个函数，还有一个栈空间的(为什么传尾纸指针，因为栈是反着的)*/
	int container_pid = clone(container_main, container_stack+STACK_SIZE, SIGCHLD, NULL);
	/*等待子进程结束*/
	waitpid(container_pid, NULL, 0);
	printf("Parent - container stopped!\n");
	return 0;
}
