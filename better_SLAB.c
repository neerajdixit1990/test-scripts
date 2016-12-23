#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>
#include <numa.h>

#define	N 			10000
#define	NO_FORKS		1500
#define L2_CACHE_SIZE		256*1024
#define CACHE_CLEAN_STEP	10

void
clear_l2_cache() {
	char dummy[L2_CACHE_SIZE];
	memset(dummy, 0, sizeof dummy);
}

int main(int argc, char **argv) {
	int		count = 0;
        int     	depth = 0;
	int		status = -1;
        int     	i = 0, id = 0;
        void    	*ret_addr[N];
        int     	*int_ptr = NULL;
        pid_t   	pid = 0;
        char    	args[10];

	if (argc < 2) {
		printf("Usage: <prog> <num of forks>\n");
		exit(1);
	}

	depth = atoi(argv[1]);
	numa_set_strict(1);

        if (depth%2 == 0)
        	numa_run_on_node(0);
        else
        	numa_run_on_node(1);

        for (i = 0; i < N; i++) {
        	ret_addr[i] = numa_alloc_local(4096);
        	if (ret_addr[i]  == NULL) {
        		printf("NUMA alloc on node 0 failed !\n");
			exit(1);
		}

        	// access memory to fill up page tables
        	int_ptr = ret_addr[i];
        	*int_ptr = 101;
        }

	if (depth == NO_FORKS) {
		for (id = 0; id < NO_FORKS; id++) {

			pid = fork();
			if (pid < 0) {
				printf("\nError in fork() system call : %s\n", strerror(errno));
			} else if (pid == 0) {
				snprintf(args, 10, "%d", id);
                        	execlp("./better_SLAB", "./better_SLAB", args, (char *)0);
                        	printf("\nShould not return !\n");
                        	return 0;
			} else {
                        	// make separate pages in-case of COW fork()
                        	for (i = 0; i < N; i++) {
                                	int_ptr = ret_addr[i];
                                	*int_ptr = 102;
                        	}
			}
		}
	}

	if (depth == NO_FORKS) {
		// parent process waits for child completion
		count = 0;
		while (1) {
			status = waitpid(-1, NULL, 0);
			if (errno == EINTR)
				continue;
			if (status > 0) {
				count++;
				//printf("Count = %d\n", count);
				continue;
			}
			//printf("returned =%d\n", status);
			break;
		}
	}
	
        if (depth%2 == 0)
                numa_run_on_node(1);
        else
                numa_run_on_node(0);

	// unmap memory in process
	for (i = 0; i < N; i++) {
                numa_free(ret_addr[i], 4096);

		if (i%CACHE_CLEAN_STEP == 0)
			clear_l2_cache();
	}
	return 0;
}
