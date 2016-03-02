#include <stdio.h>
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

	size_t shm_size = sizeof(struct server_mem);
	key_t shm_key = 10;
	int shm_id;
	
	//set up stcuct;

	struct server_mem* shared_mem;

	//create shared memory
	shm_id = shm_open("/myserver_mem",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(shm_id == -1){
		printf("shm_open error\n");
	}
	if(ftruncate(shm_id, shm_size) == -1){
		printf("ftruncate error\n");
	}

	
	//attach memory -- 
	shared_mem = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
	if(shared_mem == MAP_FAILED){
		printf("error");
	}	
	
	//setup semaphores
	sem_init(&(shared_mem->full),1,0);
	sem_init(&(shared_mem->empty),1,BUFF_SZE);
	sem_init(&(shared_mem->mutex),1,1);
	shared_mem->count = -1;

	struct job remove;
	int buff_id;
	while(1){
		//take a job
		printf("waiting on job\n");
		sem_wait(&shared_mem->full);
		sem_wait(&shared_mem->mutex);
		
		remove = shared_mem->buffer[shared_mem->count % BUFF_SZE];
		buff_id = (shared_mem->count % BUFF_SZE);
		printf("removing job from buffer[%d]\n", buff_id);
		//shared_mem->count--;
		sem_post(&(shared_mem->mutex));
		sem_post(&(shared_mem->empty));

		//print a job
		printf("Started printing %d pages from client %d (buffer[%d])\n",
		remove.pages, remove.id, buff_id);
		//sleep
		usleep(remove.time);
		printf("Finished printing %d pages from client %d (buffer[%d])\n",
		remove.pages, remove.id, buff_id);
	
	}
	
}

