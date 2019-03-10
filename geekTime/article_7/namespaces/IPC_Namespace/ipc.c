/**
 * 这个程序来自耗子叔的blog: https://coolshell.cn/articles/17010.html
 * 自己手动打一遍以便记住,所以叫copyer
 * @copyer wangjun
 *
 * IPC全称 Inter-Process Communication，是Unix/Linux下进程间通信的一种方式，
 * IPC有共享内存、信号量、消息队列等方法。所以，为了隔离，
 * 我们也需要把IPC给隔离开来，这样，只有在同一个Namespace下的进程才能相互通信。
 * 如果你熟悉IPC的原理的话，你会知道，IPC需要有一个全局的ID，即然是全局的，
 * 那么就意味着我们的Namespace需要对这个ID隔离，不能让别的Namespace的进程看到。
 *
 * 要启动IPC隔离，我们只需要在调用clone时加上CLONE_NEWIPC参数就可以了。
 *
 * int container_pid = clone(container_main, container_stack+STACK_SIZE, 
 *             CLONE_NEWUTS | CLONE_NEWIPC | SIGCHLD, NULL);
 *
 * ************************************************************
 * ipcs : http://www.linux-commands-examples.com/ipcs
 * ipcmk : http://www.linux-commands-examples.com/ipcmk
 * ## 通过ipcmk -Q 创建一个IPC Queue
 * ## 在当前父容器中查看 ipcs -q
 * ## 在加了CLONE_NEWIPC的子容器中查看，将会发现IPC已经被隔离了
 * ************************************************************
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
	/*http://man7.org/linux/man-pages/man2/sethostname.2.html*/
	sethostname("ilovemeibao", 12); /*设置hostname*/
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
	/*http://man7.org/linux/man-pages/man2/clone.2.html*/
	int container_pid = clone(container_main, container_stack+STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC | SIGCHLD, NULL);
	/*等待子进程结束*/
	waitpid(container_pid, NULL, 0);
	printf("Parent - container stopped!\n");
	return 0;
}
