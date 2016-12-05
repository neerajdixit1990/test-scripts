#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>

#define	N 			10000
#define STEP_ALLOC		50
#define STEP_ACCESS		100
#define	STEP_DEALLOC		150

void
change_cpu_other_numanode() {
	int		i = 0, j = 0;
	cpu_set_t       mask;
	int		status;

	// get CPU affinity
	CPU_ZERO(&mask);
	status = sched_getaffinity(0, sizeof(mask), &mask);
	if (status != 0)
		printf("\nUnable to get CPU affinity: %s\n", strerror(errno));

	for (j = 0; j < 12; j++) {
		if (CPU_ISSET(j, &mask))
		break;
	}

	// change CPU affinity to CPU on another NUMA node 
	CPU_ZERO(&mask);
	CPU_SET((j+6)%12, &mask);

        status = sched_setaffinity(0, sizeof(mask), &mask);
        if (status != 0)
        	printf("\nUnable to set CPU affinity: %s\n", strerror(errno));

	// yield to access memory from different CPU
	status = sched_yield();
	if (status != 0)
		printf("\nUnable to yield CPU : %s\n", strerror(errno));
}

void
change_cpu_same_numanode() {
        int             i = 0, j = 0;
        cpu_set_t       mask;
        int             status;

        // get CPU affinity
        CPU_ZERO(&mask);
        status = sched_getaffinity(0, sizeof(mask), &mask);
        if (status != 0)
                printf("\nUnable to get CPU affinity: %s\n", strerror(errno));

        for (j = 0; j < 12; j++) {
                if (CPU_ISSET(j, &mask))
                	break;
        }

        // change CPU affinity to CPU on same NUMA node 
        CPU_ZERO(&mask);
        if (j < 6) {
                CPU_SET((j+1)%6, &mask);
        } else {
                CPU_SET(((j-6+1)%6) + 6, &mask);
        }

        status = sched_setaffinity(0, sizeof(mask), &mask);
        if (status != 0)
                printf("\nUnable to set CPU affinity: %s\n", strerror(errno));

        // yield to access memory from different CPU
        status = sched_yield();
        if (status != 0)
                printf("\nUnable to yield CPU : %s\n", strerror(errno));
}

int main(int argc, char **argv) {
        int     	depth = 0;
	int		status = -1;
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
	snprintf(args, 10, "%d", depth-1);

	// mmap anonymous memory in the parent process
	for (i = 0; i < N; i++) {
		ret_addr[i] = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

                if (i%STEP_ALLOC == 0)
                        change_cpu_same_numanode();
	}

	// access that memory to fill up the page tables
	for (i = 0; i < N; i++) {
		int_ptr = ret_addr[i];
		*int_ptr = 101;	
	}

	if (depth > 0) {
		pid = fork();
		if (pid == 0) {
			// spawn same program with reduced depth
			execlp("./better_SLAB", "./better_SLAB", args, (char *)0);
			printf("\nShould not return !!\n");
			return 0;
		} else if (pid < 0) {
			printf("\nError in fork() system call : %s\n", strerror(errno));
		} else {

		        // switch to other NUMA node for de-allocation
        		change_cpu_other_numanode();

                        // make separate pages in-case of COW fork()
                        for (i = 0; i < N; i++) {
                                int_ptr = ret_addr[i];
                                *int_ptr = 102;
        
                                if (i%STEP_ACCESS == 0)
                                        change_cpu_same_numanode();
	                }
			// parent process waits for child completion
			while (wait(&status) != pid) 
               			;
		}
	}

	// unmap memory in parent process
	for (i = 0; i < N; i++) {
                status = munmap(ret_addr[i], 4096);

                if (i%STEP_DEALLOC== 0)
                        change_cpu_same_numanode();
	}
	return 0;
}
