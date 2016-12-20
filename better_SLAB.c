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
#define STEP_ALLOC		50
#define STEP_ACCESS		50
#define	STEP_DEALLOC		50
#define L2_CACHE_SIZE		256*1024
#define CACHE_CLEAN_STEP	10

void
clear_l2_cache() {
	char dummy[L2_CACHE_SIZE];
	memset(dummy, 0, sizeof dummy);
}

void
set_numa_cpu_affinity() {
        int             i = 0, j = 0;
        int             status;
	struct bitmask	*a = NULL;

	/*// schedule process on NUMA node 1 
        a = numa_allocate_cpumask();
        for (i = 6; i < 12; i++)
                 a = numa_bitmask_setbit(a, i);

        //for (i = 0; i < 12; i++) {
        //        printf("\tNUMA set affinity for CPU %d: %d\n", i, numa_bitmask_isbitset(a, i));
        //}

        i = numa_sched_setaffinity(0, a);
        if (i != 0)
                printf("sched set affinity error : %s\n", strerror(errno));
        numa_bitmask_free(a);*/

	numa_run_on_node(1);

        // yield to access memory from different CPU
        /*status = sched_yield();
        if (status != 0)
                printf("\nUnable to yield CPU : %s\n", strerror(errno));*/
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

	numa_set_strict(1);
	if (depth%2 == 0)
		numa_run_on_node(0);
	else
		numa_run_on_node(1);
	// mmap anonymous memory in the parent process
	for (i = 0; i < N; i++) {
	        //ret_addr[i] = numa_alloc_onnode(4096, 0);
		ret_addr[i] = numa_alloc_local(4096);
        	if (ret_addr[i]  == NULL)
                	printf("NUMA alloc on node 0 failed !\n");
	
		/*ret_addr[i] = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

                if (i%STEP_ALLOC == 0)
                        set_numa_cpu_affinity();*/
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

                        // make separate pages in-case of COW fork()
                        for (i = 0; i < N; i++) {
                                int_ptr = ret_addr[i];
                                *int_ptr = 102;
        
                                if (i%STEP_ACCESS == 0)
                                        set_numa_cpu_affinity();
	                }
			// parent process waits for child completion
			while (wait(&status) != pid) 
               			;
		}
	}

        if (depth%2 == 0)
                numa_run_on_node(1);
        else
                numa_run_on_node(0);

	// unmap memory in parent process
	for (i = 0; i < N; i++) {
                numa_free(ret_addr[i], 4096);

                /*if (i%STEP_DEALLOC== 0)
                        set_numa_cpu_affinity();*/
		if (i%CACHE_CLEAN_STEP == 0)
			clear_l2_cache();
	}
	return 0;
}
