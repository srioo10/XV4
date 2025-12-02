#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 10 
#define X 2   
#define Y 2   

int buffer[N];
int pro = 0; 
int con = 0;
sem_t empty;
sem_t full;
pthread_mutex_t mutex;

void *producer(void *arg){
    int id = *((int*)arg);
    int flag1 = 1;
    while(flag1 <= 10){
        int value = rand()%1000; 
        sem_wait(&empty);  
        pthread_mutex_lock(&mutex);
        buffer[pro] = value;
        printf("--------------------------------------------\nProducer %d:\nProduced Value:%d,Index:%d,Production No:%d\n--------------------------------------------\n", id,value,pro,flag1);
        pro = (pro+1)%N;
        flag1++;  
        pthread_mutex_unlock(&mutex);
        sem_post(&full);   
        sleep(1);  
    }
    return NULL;
}

void *consumer(void *arg){
	int id = *((int*)arg);
	int flag2 = 1; 
    while(flag2 <= 10){
        sem_wait(&full);  
        pthread_mutex_lock(&mutex);
        int num = buffer[con];
        printf("--------------------------------------------\nConsumer %d:\nConsumed value:%d,Index %d\n--------------------------------------------\n",id,num,con);
        con = (con+1)%N;
        flag2++;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        sleep(1);
    }
    return NULL;
}

int main(){
	printf("--------------------------------------------\nThe Upper bound for each Producer is Set to 10. So if a Producer produces 10 values into the buffer it will terminate.\n--------------------------------------------\n");
	pthread_t producers[X];
	pthread_t consumers[Y];
    int p_ids[X], c_ids[Y];

    sem_init(&empty, 0, N);  
    sem_init(&full, 0, 0);   
    pthread_mutex_init(&mutex, NULL); 

    for(int i = 0; i < X; i++){
        p_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer, &p_ids[i]) != 0) {
            perror("--------------------------------------------\nError:\nFailed to create Producer Thread.\n--------------------------------------------\n");
            exit(1);
        }
    }
    for(int i = 0; i < Y; i++){
        c_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer, &c_ids[i]) != 0) {
            perror("--------------------------------------------\nError:\nFailed to create Consumer Thread.\n--------------------------------------------\n");
            exit(1);
        }
    }
    for(int i = 0; i < X; i++){
        pthread_join(producers[i], NULL);
    }
    for(int i = 0; i < Y; i++){
        pthread_join(consumers[i], NULL);
    }
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    return 0;
}

