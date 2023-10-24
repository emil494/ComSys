#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"


int job_queue_init(struct job_queue *job_queue, int capacity) {
  if (job_queue->buffer != NULL) {
    printf("job_queue must be uninitialized\n");
    return 1;
  }
  job_queue = (struct job_queue *)malloc(sizeof(struct job_queue));
  *(job_queue)->buffer = (void**)malloc(capacity*sizeof(void*));
  if (job_queue->buffer == NULL || job_queue == NULL) {
    return 1;
  }
  
  job_queue->n = capacity;
  job_queue->num = 0;
  job_queue->destroyed = 0;
  pthread_mutex_init(&job_queue->lock, NULL);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&job_queue->lock);
  if (job_queue->num == 0) {
    pthread_mutex_unlock(&job_queue->lock);
    pthread_mutex_lock(&job_queue->lock);
  }
  free(job_queue);
  job_queue->destroyed = 1;
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (job_queue->destroyed == 1) {
    printf("Can't push onto a destroyed queue\n");
    return 1;
  }
  pthread_mutex_lock(&job_queue->lock);
  while (job_queue->num == job_queue->n) {
    pthread_mutex_unlock(&job_queue->lock);
    pthread_mutex_lock(&job_queue->lock);
  }
  job_queue->buffer[job_queue->num-1] = data;
  job_queue->num += 1;
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}


int job_queue_pop(struct job_queue *job_queue, void **data) {
  if (job_queue->destroyed == 1) {
    printf("Can't pop from a destroyed queue\n");
    return 1;
  }
  pthread_mutex_lock(&job_queue->lock);
  while (job_queue->num == 0) {
    pthread_mutex_unlock(&job_queue->lock);
    pthread_mutex_lock(&job_queue->lock);
  }
  *data = &(job_queue->buffer[0]);
  job_queue->buffer[0] = NULL;
  job_queue->num -= 1;
  if (job_queue->num != 0) {
    for (int i = 0; i < job_queue->num; i++) {
      job_queue->buffer[i] = job_queue->buffer[i+1];
    }
    job_queue->buffer[job_queue->num] = NULL;
  }
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}

