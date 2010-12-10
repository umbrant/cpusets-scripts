/*******************************************************************
 * This program has the sole purpose of showing some kernel API 
 * for CPU affinity. Consider this merely a demo...
 * 
 * Written by Eli Michael Dow <emdow@us.ibm.com>
 * 
 * Last Modified: May 31 2005. 
 *
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <unistd.h>
#define  __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>

#include <time.h>

/* Create us some pretty boolean types and definitions */
typedef int bool;	
#define TRUE  1
#define FALSE 0 

enum {
	CACHE_TEST,
	MEM_TEST,
	CPU_TEST
};

/* Method Declarations */
void usage();								 /* Simple generic usage function */
bool do_cpu_stress(int numthreads, int numcpus);		 /* Entry point into CPU thrash   */
int  do_cpu_expensive_op(int myitem);            /* Single thread cpu intensive   */
bool check_cpu_expensive_op(int possible_result);/* Compare value to precomputed  */
unsigned int get_random();

int oneoff = 0;
int cpu = -1;

unsigned long long int gettime(void)
{
	unsigned long long int nsec;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	nsec = now.tv_nsec + now.tv_sec*(1000000000);
	return nsec;
}


unsigned long long int rdtsc(void)
{
   unsigned long long int x;
   unsigned a, d;

   __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

   return ((unsigned long long)a) | (((unsigned long long)d) << 32);;
}


// Linear feedback shift register, bad pseudo random generating function
unsigned int lfsr = 1;
unsigned int get_random() {
  /* taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1 */
  lfsr = (lfsr >> 1) ^ (unsigned int)(0 - (lfsr & 1u) & 0xd0000001u); 
  //++period;
  return lfsr;
}


int main( int argc, char **argv )
{
	int return_code = FALSE;

	/* Determine the actual number of processors */
	int NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF);
	printf("System has %i processor(s).\n", NUM_PROCS);

	/* These need sane defaults, because the values will be used unless overriden */
	int num_cpus_to_spin = NUM_PROCS; 
	int num_threads_to_spin = NUM_PROCS; 

	// Type of test to run, default CPU
	int test = CPU_TEST;

    /* Check for user specified parameters */
	int option = 0; 
	while ((option = getopt(argc, argv, "t:n:1:cmpa?")) != -1)
	{
		switch (option)
		{
			case 't': /* specify number of threads to spawn */
				num_threads_to_spin = atoi(optarg);
				printf("Starting %i thrasher threads.\n", num_threads_to_spin);
				break;

			case 'n': /* SPECIFY NUM CPUS TO MAKE BUSY */
				num_cpus_to_spin = atoi(optarg);
				if (num_cpus_to_spin < 1)
				{
					printf("WARNING: Must utilize at least 1 cpu. Spinning "
							" all %i cpu(s) instead...\n", NUM_PROCS);
					num_cpus_to_spin = 1;
				}
				else if (num_cpus_to_spin > NUM_PROCS)
				{
					printf("WARNING: %i cpu(s), are not "
							"available on this system, spinning all %i cpu(s) "
                            "instead...\n", num_cpus_to_spin, NUM_PROCS);
					num_cpus_to_spin = NUM_PROCS;
				}
				else
				{	
					printf("Maxing computation on %i cpu(s)...\n",
							num_cpus_to_spin);
				}
				break;

			case '1': /* One off timing test on specified CPU */
				cpu = atoi(optarg);
				if(cpu < 0 || cpu >= NUM_PROCS) {
					printf("Invalid cpu %i specified, defaulting to last...\n", cpu);
					cpu = NUM_PROCS-1;
				}
				oneoff = 1;
				break;

			case 'm': /* mem bandwidth test */
				test = MEM_TEST;
				break;

			case 'c': /* cache test */
				test = CACHE_TEST;
				break;

			case 'p': /* cpu test */
				test = CPU_TEST;
				break;

			case '?':
				if (isprint (optopt))
				{
					fprintf (stderr, 
							"Unknown option `-%c'.\n", optopt);
				}
				else
				{
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
				}
				usage(argv[0]);
				exit(0);
				break;

			default:
				usage(argv[0]);
				exit(0);
		}
	}

	if(test == MEM_TEST) {
		printf("Will do memory bandwidth testing");
	} else if(test == CPU_TEST) {
		printf("Will do CPU testing");
	} else {
		printf("Will do cache testing");
	}

	if(oneoff) {
		printf(" in oneoff mode.\n");
	} else {
		printf(".\n");
	}

	/* Kick off the actual work of spawning threads and computing */
	do_stress(num_threads_to_spin, num_cpus_to_spin, test); 
	return return_code;
}



/* This method simply prints the usage information for this program */
void usage()
{
	printf("-n  NUM_CPUS_TO_STRESS\n");
	printf("-t  NUM_THREADS_TO_SPAWN\n");
	printf("-c cache testing\n");
	printf("-m memory bandwidth testing\n");
	printf("-p CPU testing\n");
	printf("If no parameters are specified all cpu's will be made busy.\n");
	return;
}



/* This method creates the threads and sets the affinity. */
bool do_stress(int numthreads, int numcpus, int test)
{
	int ret = TRUE;
	int created_thread = 0;

	/* We need a thread for each cpu we have... */
	while ( created_thread < numthreads - 1 )
	{
		int mypid = fork();

		if (mypid == 0) /* Child process */
		{
			printf("\tCreating Child Thread: #%i\n", created_thread);
			break;
		}

		else /* Only parent executes this */
		{ 
			/* Continue looping until we spawned enough threads! */ ;
			created_thread++;
		} 
	}



	/* NOTE: All threads execute code from here down! */



	cpu_set_t mask;

	/* CPU_ZERO initializes all the bits in the mask to zero. */ 
    CPU_ZERO( &mask );	

	/* CPU_SET sets only the bit corresponding to cpu. */
	/* Allow the threads to run on any CPU < numcpus, letting the 
	 * linux scheduler take care of things */
	int i;
	if(oneoff) {
		CPU_SET(cpu, &mask);
	} else {
		for(i=0; i<numcpus; i++) {
			CPU_SET( i, &mask );  
    	}
    }

	/* sched_setaffinity returns 0 in success */
    if( sched_setaffinity( 0, sizeof(mask), &mask ) == -1 )
	{
		printf("WARNING: Could not set CPU Affinity, continuing...\n");
	}
	/* sched_setaffinity sets the CPU affinity mask of the process denoted by pid. 
	   If pid is zero, then the current process is used.

	   The affinity mask is represented by the bitmask stored in mask. The least
	   significant bit corresponds to the first logical processor number on the
	   system, while the most significant bit corresponds to the last logical 
	   processor number on the system. A set bit corresponds to a legally schedulable
	   CPU while an unset bit corresponds to an illegally schedulable CPU. In other 
	   words, a process is bound to and will only run on processors whose 
       corresponding bit is set. Usually, all bits in the mask are set.

	   Also the affinity is passed on to any children!
	   */

	/* Now we have a single thread bound to each cpu on the system */
	// Spin threads until a manual kill, or counter times in oneoff case
	int counter = 10;
	unsigned long long int start_time = gettime();
	while(counter > 0) {
		// Need to multiply each test to get the same orders of magnitude
		if(test == CACHE_TEST) {
			do_cache_expensive_op();
		} else if(test == CPU_TEST) {
			do_cpu_expensive_op(39);
		} else if(test == MEM_TEST) {
			do_mem_expensive_op();
		}
		if(oneoff)
			counter--;
	}
	unsigned long long int end_time = gettime();

	//printf("nsec: %lld\n", end_time-start_time);
	printf("sec: %f\n", (double)(end_time-start_time)/(double)(1000000000));

	/*
	cpu_set_t mycpuid;	 
	sched_getaffinity(0, sizeof(mycpuid), &mycpuid);

	if ( check_cpu_expensive_op(computation_res) )
	{
		printf("SUCCESS: Thread completed computational task, and PASSED integrity check!\n",
				mycpuid);
		ret = TRUE;	
	}
	else 
	{
		printf("FAILURE: Thread failed integrity check!\n",
				mycpuid);
		ret = FALSE;
		return ret;
	}	
	*/

	return ret;
} 


/* Lame (computationally wasteful) recursive fibonaci sequence finder 
   Intentionally does not store known computed values. 
   */
int do_cpu_expensive_op(int myitem)
{
	/* FIXME: Should check myitem size because this could overflow quick */
	if (myitem == 0 || myitem == 1) 
	{ 
		return myitem; 
	}

	return ( do_cpu_expensive_op( myitem - 1 ) + do_cpu_expensive_op( myitem - 2 ) ); 
}

/* Cache intensive thrasher
 * Calculates partial sums of an array, and does it stupidly with bad time complexity
 */
int do_cache_expensive_op()
{
	// Shared cache is probably about 6 MB big? That's how big L3 on K10s are
	// Allocate an array that is that big, to theoretically push everything else out
	double size = 6*(2^20) / sizeof(int);
	int* array = (int*)calloc(size, sizeof(int));
	
	// Populate the array
	unsigned int i,k;
	for(i=0; i<size; i++) {
		array[i] = i;
	}
	// Do some stupid sum operations and store in place
	// Also spin on this, to dominate malloc/free times
	int counter = 200000;
	while(counter > 0) {
		for(i=0; i<size; i++) {
			int sum = 0;
			for(k=0; k<size; k++) {
				if(i!=k) {
					sum += array[k];
				}
			}
			array[i] = sum;
		}
		if(oneoff)
			counter--;
	}

	free(array);
	return 0;
}

/* Mem intensive operation
 * Repeatedly mallocs, strides, and frees blocks of data 
 */
int do_mem_expensive_op()
{
	int j;
	for(j=0; j<1000; j++) {
		// Allocate a very large array this time, to make sure we're bigger than the cache
		unsigned int size = 64*(1<<20) / sizeof(int);
		int* array = (int*)calloc(size, sizeof(int));
		// Stride by 16 ints each time. that should do it?
		unsigned int i, k;

		// Pick random locations to put the running sum
		unsigned int index, sum;
		for(k=0; k<30; k++) {
			for(i=0; i<size; i++) {
				index = (get_random()+i) % size;
				sum += array[index];
				array[index] = sum;
			}
		}

		/*
		int stride = 8;
		// Do a bad stride pattern
		for(k=0; k<stride; k++) {
			for(i=k; i<size/stride; i+=stride) {
				array[i] = i;
			}
		}
		*/

		free(array);
	}

	return 0;
}


/* This method simply takes an integer argument
   and compares it to a precomputed correct value.
   */
bool check_cpu_expensive_op(int possible_result)
{
	/* 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987 ... fib(41) = 165580141 */ 
	int actual_result = 165580141;
	return ( actual_result == possible_result );
}
