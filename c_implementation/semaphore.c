#include <stdlib.h>
#include <pthread.h>
#include "semaphore.h"

/*
в C є semaphore.h для unix-ів, але було цікаво реалізувати самому
*/

semaphore_buffer_t *new_semaphore(int size)
{
    // виділяємо пам'ять під семафор
    semaphore_buffer_t *sem = (semaphore_buffer_t *)malloc(sizeof(semaphore_buffer_t));
    // виділяємо пам'ять під буфер
    sem->buffer = (void **)malloc(size * sizeof(void *));
    sem->size = size;
    sem->count = 0;
    sem->in = 0;
    sem->out = 0;
    // ініціалізуємо mutex та умовні змінні, щоб розуміти,
    // що в певнийний момент часу по заповненості буфера
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->not_full, NULL);
    pthread_cond_init(&sem->not_empty, NULL);

    return sem;
}

void acquire(semaphore_buffer_t *sem, void *item)
{
    pthread_mutex_lock(&sem->mutex);
    // чекаємо поки звільниться місце в буфері
    while (sem->count == sem->size)
    {
        pthread_cond_wait(&sem->not_full, &sem->mutex);
    }
    sem->buffer[sem->in] = item;
    // ділимо по модулю, щоб коли досягали повного розміру, індекс повертався до 0
    sem->in = (sem->in + 1) % sem->size;
    sem->count++;
    pthread_cond_signal(&sem->not_empty);
    pthread_mutex_unlock(&sem->mutex);
}

void *release(semaphore_buffer_t *sem)
{
    pthread_mutex_lock(&sem->mutex);
    // чекаємо, поки в буфері щось з'явиться
    while (sem->count == 0)
    {
        pthread_cond_wait(&sem->not_empty, &sem->mutex);
    }
    void *item = sem->buffer[sem->out];
    // ділимо по модулю, щоб циклічно "ходити" по буферу
    sem->out = (sem->out + 1) % sem->size;
    sem->count--;
    pthread_cond_signal(&sem->not_full);
    pthread_mutex_unlock(&sem->mutex);
    return item;
}

void free_semaphore(semaphore_buffer_t *sem)
{
    free(sem->buffer);
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->not_full);
    pthread_cond_destroy(&sem->not_empty);
    free(sem);
}