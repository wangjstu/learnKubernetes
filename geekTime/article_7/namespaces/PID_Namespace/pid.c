/**
 * 这个程序来自耗子叔的blog: https://coolshell.cn/articles/17010.html
 * 自己手动打一遍以便记住,所以叫copyer
 * @copyer wangjun
 * 
 * 在传统的UNIX系统中，PID为1的进程是init，地位非常特殊。他作为所有进程的父进程，
 * 有很多特权（比如：屏蔽信号等），另外，其还会为检查所有进程的状态，我们知道，
 * 如果某个子进程脱离了父进程（父进程没有wait它），那么init就会负责回收资源并结束这个子进程。
 * 所以，要做到进程空间的隔离，首先要创建出PID为1的进程，最好就像chroot那样，
 * 把子进程的PID在容器内变成1。
 *
 * 但是，我们会发现，在子进程的shell里输入ps,top等命令，我们还是可以看得到所有进程。
 * 说明并没有完全隔离。这是因为，像ps, top这些命令会去读/proc文件系统，
 * 因为/proc文件系统在父进程和子进程都是一样的，所以这些命令显示的东西都是一样的。
 *
 * 所以，我们还需要对文件系统进行隔离。
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
	/*http://man7.org/linux/man-pages/man2/getpid.2.html*/
	printf("Container [%5d] - inside the container!\n", getpid());
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
	printf("Parent [%5d] - start a container!\n", getpid());
	/*调用clone函数，其中传出一个函数，还有一个栈空间的(为什么传尾纸指针，因为栈是反着的)*/
	/*启用PID namespace - CLONE_NEWPID*/
	/*http://man7.org/linux/man-pages/man2/clone.2.html*/
	int container_pid = clone(container_main, container_stack+STACK_SIZE, CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD, NULL);
	/*等待子进程结束*/
	waitpid(container_pid, NULL, 0);
	printf("Parent - container stopped!\n");
	return 0;
}
