#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SZE 10

struct job{
	int id;
	int pages;
	int time;
};

struct server_mem{
	sem_t full;
	sem_t empty;
	sem_t mutex;
	struct job buffer[BUFF_SZE];
	int count;
};

int main(int argc, char** argv){
		
	if(argc  < 4){
		printf("error with number of args\n");
		exit(1);
	}

	size_t shm_size = sizeof(struct server_mem);
	key_t shm_key = 10;
	int shm_id;
	
	//set up stcuct;

	struct server_mem* shared_mem;

	//open shared memory
	shm_id = shm_open("/myserver_mem", O_RDWR, S_IRUSR | S_IWUSR);
	if(shm_id == -1){
		printf("shm_open error\n");
	}
	//if(ftruncate(shm_id, shm_size) == -1){
	//	printf("ftruncate error\n");
	//}

	
	//attach memory -- 
	shared_mem = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
	if(shared_mem == MAP_FAILED){
		printf("error");
	}	
	
	//get params and create
	struct job new_job;

	new_job.id = atoi(argv[1]);
	new_job.pages = atoi(argv[2]);
	new_job.time = atoi(argv[3]);
	

	//place a job
	sem_wait(&shared_mem->empty);
	sem_wait(&shared_mem->mutex);
	shared_mem->count++;	
	int entry = (shared_mem->count % BUFF_SZE);
	shared_mem->buffer[entry] = new_job;


	sem_post(&(shared_mem->mutex));
	sem_post(&(shared_mem->full));

	//print a job
	printf("Client %d has requested %d pages (buffer[%d])\n",
	new_job.id, new_job.pages, entry);
	
	
}

