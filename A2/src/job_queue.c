#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"


int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->buffer = malloc(capacity * sizeof(void*));

  if (job_queue->buffer == NULL || job_queue == NULL) {
    free(job_queue->buffer);
    return 1;
  }

  job_queue->max = capacity;
  job_queue->num = 0;
  job_queue->top = 0;
  job_queue->rear = 0;

  job_queue->destroyed = false;
  pthread_mutex_init(&job_queue->lock, NULL);
  pthread_cond_init(&job_queue->cond_pop, NULL);
  pthread_cond_init(&job_queue->cond_push, NULL);
  pthread_cond_init(&job_queue->cond_destroy, NULL);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&job_queue->lock);

  while (job_queue->num > 0) {
    pthread_cond_wait(&job_queue->cond_destroy, &job_queue->lock);
  }

  job_queue->destroyed = true;
  pthread_mutex_unlock(&job_queue->lock);
  pthread_cond_broadcast(&job_queue->cond_pop);
  pthread_mutex_destroy(&job_queue->lock);
  pthread_cond_destroy(&job_queue->cond_destroy);
  pthread_cond_destroy(&job_queue->cond_pop);
  pthread_cond_destroy(&job_queue->cond_push);

  free(job_queue->buffer);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  pthread_mutex_lock(&job_queue->lock);

  while (job_queue->num == job_queue->max && !job_queue->destroyed) {
    pthread_cond_wait(&job_queue->cond_push, &job_queue->lock);
  }
  
  job_queue->buffer[job_queue->rear] = data;
  job_queue->rear = (job_queue->rear + 1) % job_queue->max;
  job_queue->num += 1;

  pthread_cond_signal(&job_queue->cond_pop);
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}


int job_queue_pop(struct job_queue *job_queue, void **data) {
  pthread_mutex_lock(&job_queue->lock);

  while (job_queue->num == 0 && !job_queue->destroyed) {
    pthread_cond_wait(&job_queue->cond_pop, &job_queue->lock);
  }

  if (job_queue->destroyed) {
    pthread_mutex_unlock(&job_queue->lock);
    return -1;
  }

  *data = job_queue->buffer[job_queue->top];
  job_queue->buffer[job_queue->top] = NULL;
  job_queue->top = (job_queue->top + 1) % job_queue->max;
  job_queue->num -= 1;

  pthread_cond_signal(&job_queue->cond_push);
  pthread_cond_signal(&job_queue->cond_destroy);
  pthread_mutex_unlock(&job_queue->lock);
  return 0;
}
