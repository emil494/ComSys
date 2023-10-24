#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"


int job_queue_init(struct job_queue *job_queue, int capacity) {
  *(job_queue->buffer) = malloc(capacity*sizeof(void*));
  if (!job_queue->buffer) {
    return 1;
  }
  
  job_queue->n = capacity;
  job_queue->num = 0;
  pthread_mutex_init(&job_queue->lock, NULL);
  pthread_cond_init(&job_queue->cond_empty, NULL);
  pthread_cond_init(&job_queue->cond_not_empty, NULL);
  pthread_cond_init(&job_queue->cond_not_full, NULL);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&job_queue->lock);
  while (job_queue->num != 0) {
    pthread_cond_wait(&job_queue->cond_empty, &job_queue->lock);
  }
  free(*(job_queue->buffer));
  free(job_queue);
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (!job_queue) {
    printf("Can't push onto a destroyed queue\n");
    return 1;
  }
  pthread_mutex_lock(&job_queue->lock);
  while (job_queue->num == job_queue->n) {
    pthread_cond_wait(&job_queue->cond_not_full, &job_queue->lock);
  }
  job_queue->buffer[job_queue->num] = data;
  job_queue->num += 1;
  pthread_mutex_unlock(&job_queue->lock);
  pthread_cond_broadcast(&job_queue->cond_not_empty);
  return 0;
}


int job_queue_pop(struct job_queue *job_queue, void **data) {
  if (!job_queue) {
    printf("Can't pop from a destroyed queue\n");
    return 1;
  }
  pthread_mutex_lock(&job_queue->lock);
  while (job_queue->num == 0) {
    pthread_cond_wait(&job_queue->cond_not_empty, &job_queue->lock);
  }
  *data = job_queue->buffer[0];
  job_queue->num -= 1;
  for (int i = 0; i < job_queue->num; i++) {
    job_queue->buffer[i] = job_queue->buffer[i+1];
  }
  job_queue->buffer[job_queue->num] = NULL;
  pthread_cond_broadcast(&job_queue->cond_not_empty);
  pthread_cond_broadcast(&job_queue->cond_not_full);
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}

