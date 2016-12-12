#include <stdio.h>
#include <numa.h>
#include <errno.h>

int
main() {
	struct	bitmask		*data = NULL;
	struct	bitmask		*a = NULL;
	int			i = 0;
	void			*temp = NULL;
	int			*int_ptr = NULL;

	printf("NUMA available = %d\n", numa_available());
	printf("Max NUMA nodes on system = %d\n", numa_max_possible_node());
	printf("NUMA nodes available = %d\n", numa_max_node());	
	printf("Max configured nodes = %d\n", numa_num_configured_nodes());
	data = numa_get_mems_allowed();
	printf("allowed memory nodes = %ld\n", data->size);
	printf("allowed number of CPU's = %d\n", numa_num_configured_cpus());
	printf("NUMA distance = %d\n", numa_distance(1,0));
	
	/*printf("NUMA sched get affinity:\n");
	numa_sched_getaffinity(0, a);
	for (i = 0; i < 15; i++) {
		printf("\tNUMA get affinity for CPU %d: %d\n", i, numa_bitmask_isbitset(a, i));
	}*/

	printf("NUMA set affinity:");
	a = numa_allocate_cpumask();
	for (i = 6; i < 12; i++)
		 a = numa_bitmask_setbit(a, i);

        for (i = 0; i < 12; i++) {
                printf("\tNUMA set affinity for CPU %d: %d\n", i, numa_bitmask_isbitset(a, i));
        }

	i = numa_sched_setaffinity(0, a);
	if (i != 0)
		printf("sched set affinity error : %s\n", strerror(errno));
	numa_bitmask_free(a);
	
	temp = numa_alloc_onnode(4096, 0);
	if (temp == NULL)
		printf("NUMA alloc on node 0 failed !\n");
	else {
		int_ptr = temp;
		*int_ptr = 101;
		printf("NUMA alloc on node 0 successful :)\n");
	}
	return 0;
}
