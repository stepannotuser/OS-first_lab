#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int data;
    int is_ready;
} mutex_data;

mutex_data global_data = {
    .lock = PTHREAD_MUTEX_INITIALIZER, 
    .cond = PTHREAD_COND_INITIALIZER,
    .data = 0,
    .is_ready = 0,
};

void provide_handler(mutex_data *channel, int data) {
	pthread_mutex_lock(&channel->lock);    
    while (channel->is_ready != 0) {
        pthread_cond_wait(&channel->cond, &channel->lock);
    }	
	channel->data = data;
	channel->is_ready = 1;  
	pthread_cond_signal(&channel->cond);
	pthread_mutex_unlock(&channel->lock); 
}

void* provider(void* a) {
    for (int i = 1; i < 10; i++) {
        printf("(Provider) Sending task #%d\n", i);
        provide_handler(&global_data, i);
        sleep(1);
    }
    return NULL;
}

int worker_handler(mutex_data *channel) {
    pthread_mutex_lock(&channel->lock);
    while (channel->is_ready == 0) {
        pthread_cond_wait(&channel->cond, &channel->lock);
    }
    int data = channel->data;
    channel->is_ready = 0;
    pthread_mutex_unlock(&channel->lock); 
    return data;
}

void* worker(void* a) {
    while (1) {
        int data = worker_handler(&global_data);
        printf("(Worker) Received task #%d\n", data);
        if (data == 9) {
            break;
        }
    }
    return NULL;
}

int main() {
    
    pthread_t thread_provider;
    pthread_t thread_consumer;
    
    int result_provider = pthread_create(&thread_provider, NULL, provider, NULL);
    if (result_provider != 0) {
        printf("pthread_create failed: %d\n", result_provider);
        return result_provider;
    }
    
    int result_worker = pthread_create(&thread_consumer, NULL, worker, NULL);
    if (result_provider != 0) {
        printf("pthread_create failed: %d\n", result_worker);
        return result_worker;
    }
    
    pthread_join(thread_provider, NULL);
    pthread_join(thread_consumer, NULL);
    
    return EXIT_SUCCESS;
}
