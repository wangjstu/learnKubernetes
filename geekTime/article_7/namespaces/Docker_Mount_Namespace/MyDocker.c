/**
 * 注意事项：
 * 1、sh命令必需要copy进去，不然我们无法 chroot 
 * 2、mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL); 必须加，在系统 Linux wangjstu 3.10.0-693.2.2.el7.x86_64 #1 SMP Tue Sep 12 22:26:13 UTC 2017 x86_64 x86_64 x86_64 GNU/Linux
 * 3、cp动态库尽量用shell。另外/usr/bin/id报错: cannot find name for user ID 0 ,可以进入系统后使用:strace whoami 2>&1 | grep -E '/etc|/lib'查找确实的动态库，参考：
 * 	https://unix.stackexchange.com/questions/63182/whoami-cannot-find-name-for-user-id-0
 * 4、mnt: No such file or directory   需要创建目录 tmp/t1
 *
 *
 *
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024*1024)

static char container_stack[STACK_SIZE];
char* const container_args[] = {
	"/bin/bash",
	"-l",
	NULL
};

int container_main(void* arg)
{
	printf("Container [%5d] -- inside the container!\n", getpid());
	
	//set hostname
	sethostname("ilovemeibao", 12);

	/*重新mount proc文件系统到/proc下 */
        /*https://unix.stackexchange.com/questions/281844/why-does-child-with-mount-namespace-affect-parent-mounts*/
        mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL); // make mount point private.

	//remount "/proc" to make sure the "top" and ps show container`s information
	if (mount("proc", "rootfs/proc", "proc", 0 ,NULL) != 0) {
		perror("proc");
	}
	
	if (mount("sysfs", "rootfs/sys", "sysfs", 0 ,NULL) != 0) {
		perror("sys");
	}
	
	if (mount("none", "rootfs/tmp", "tmpfs", 0 ,NULL) != 0) {
		perror("tmp");
	}

	if (mount("udev", "rootfs/dev", "devtmpfs", 0 ,NULL) != 0) {
		perror("dev");
	}

	if (mount("devpts", "rootfs/dev/pts", "devpts", 0 ,NULL) != 0) {
		perror("dev/pts");
	}

	if (mount("shm", "rootfs/dev/shm", "tmpfs", 0 ,NULL) != 0) {
		perror("dev/shm");
	}

	if (mount("tmpfs", "rootfs/run", "tmpfs", 0 ,NULL) != 0) {
		perror("run");
	}

	/**
 	* 模仿Docker从容器外向容器里面mount相关的配置文件
 	* 你可以查看/var/lib/docker/containers/<container_id>/目录,
 	* 你会看到docker的这些文件的
 	*/ 
	if (mount("conf/hosts", "rootfs/etc/hosts", "none", MS_BIND, NULL)!=0 ||
		mount("conf/hostname", "rootfs/etc/hostname", "none", MS_BIND, NULL)!=0 ||
		mount("conf/resolv.conf", "rootfs/etc/resolv.conf", "none", MS_BIND, NULL)!=0	
	) {
		perror("conf");	
	}

	/*模仿docker run 命令中的-v， --volume=[] 参数干的事情*/	
	if (mount("tmp/t1", "rootfs/mnt", "none", MS_BIND, NULL)!=0) {
		perror("mnt");
	}
	
	/* chroot 隔离目录*/
	if (chdir("./rootfs")!=0 || chroot("./")!=0) {
		perror("chdir/chroot");
	}
	
	execv(container_args[0], container_args);
	perror("exec");
	printf("Something`s wrong!\n");
	
	return 1;
}

int main()
{
	printf("Parent [%5d] -- start a container!\n", getpid());
	int container_pid = clone(container_main, container_stack+STACK_SIZE,
				CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);
	waitpid(container_pid, NULL, 0);
	printf("Parent -- container stopped!\n");
	return 0;
}
