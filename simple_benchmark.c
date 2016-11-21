#include <stdio.h>
#include <sys/mman.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>
#define	N 10000

int main(int argc, char **argv) {
        int     	depth = 0;
	int		status = -1;
	cpu_set_t  	mask;
        int     	i = 0;
        void    	*ret_addr[N];
        int     	*int_ptr = NULL;
        pid_t   	pid = 0;
        char    	args[10];

	if (argc < 2) {
		printf("Usage: <prog> <num of forks>\n");
		exit(1);
	}
        depth = atoi(argv[1]);
	// set a CPU for the current process to ensure that the processes are not
 	// scheduled on the same CPU
	CPU_ZERO(&mask);
	CPU_SET(depth%8, &mask);
	status = sched_setaffinity(0, sizeof(mask), &mask);
	if (status != 0)
		printf("\nUnable to set CPU affinity: %s\n", strerror(errno));	

	snprintf(args, 10, "%d", depth-1);

	// mmap anonymous memory in the parent process
	for (i = 0; i < N; i++)
		ret_addr[i] = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	// access that memory to fill up the page tables
	for (i = 0; i < N; i++) {
		int_ptr = ret_addr[i];
		*int_ptr = 101;	
	}

	if (depth > 0) {
		pid = fork();
		if (pid == 0) {
			// spawn same program with reduced depth
			execlp("./simple_benchmark", "./simple_benchmark", args, (char *)0);
			printf("\nShould not return !!\n");
			return 0;
		} else if (pid < 0) {
			printf("\nError in fork() system call !\n");
		} else {
                        // make separate pages in-case of COW fork()
                        for (i = 0; i < N; i++) {
                                int_ptr = ret_addr[i];
                                *int_ptr = 102;
                        }
			// parent process waits for child completion
			while (wait(&status) != pid) 
               			;
		}
	}

        // unmap memory in parent process
	for (i = 0; i < N; i++)
                status = munmap(ret_addr[i], 4096);
	return 0;
}
