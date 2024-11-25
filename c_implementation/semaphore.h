#include <pthread.h>

/*
в C є semaphore.h для unix-ів, але було цікаво реалізувати самому)
*/

typedef struct
{
    void **buffer; // буфер "масив", який дозволить нам зберігати, будь-який тип даних
    int size;      // розмір буфера
    int count;     // к-ть елементів в буфера, вона ж макс. к-ть одночасних доступів
    int in;        // індекс для додавання елемету в буфер
    int out;       // індекс для вичитування елементу з буфера
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} semaphore_buffer_t;

semaphore_buffer_t *new_semaphore(int size);
void acquire(semaphore_buffer_t *sem, void *item);
void *release(semaphore_buffer_t *sem);
void free_semaphore(semaphore_buffer_t *sem);
