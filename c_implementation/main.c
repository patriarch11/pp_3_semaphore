#include <stdio.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <math.h>
#include <stdlib.h>

#include "semaphore.h"

#define TOTAL_PRODUCT_COUNT 10
#define PRODUCERS_COUNT 3
#define CONSUMERS_COUNT 4
#define STORAGE_CAPACITY 3

/**************************** TYPES *****************************/

typedef struct
{
    char id[37];
} product_t;

typedef struct
{
    int id;
    int count;
    semaphore_buffer_t *sem;
} worker_t;

/**************************** UTILS *****************************/

product_t *new_product()
{
    product_t *product = (product_t *)malloc(sizeof(product_t));
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, product->id);
    return product;
}

worker_t *new_worker(int id, int count, semaphore_buffer_t *sem)
{
    worker_t *worker = (worker_t *)malloc(sizeof(worker_t));
    worker->id = id;
    worker->count = count;
    worker->sem = sem;
    return worker;
}

int sum_array(int *arr, int length)
{
    int sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += arr[i];
    }
    return sum;
}

void get_job_list(int *job_list, int product_count, int subject_count)
{
    int avg = (int)floor((double)product_count / (double)subject_count);
    for (int i = 0; i < subject_count; i++)
    {
        // якщо це останній суб'єкт, то даємо йому залишок к-ті
        if (i == subject_count - 1)
        {
            job_list[i] = product_count - sum_array(job_list, i);
        }
        else
        {
            job_list[i] = avg;
        }
    }
}

void print_array(int *array, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

/**************************** MAIN *****************************/

void *producer_thread(void *arg)
{
    worker_t *worker = (worker_t *)arg;
    printf("Виробник #%d виробить: %d продуктів\n", worker->id, worker->count);
    for (int i = 0; i < worker->count; i++)
    {
        product_t *product = new_product();
        acquire(worker->sem, product);
        printf("Виробник №%d виробив: #продукт %s\n", worker->id, product->id);
    }
    return NULL;
}

void *consumer_thread(void *arg)
{
    worker_t *worker = (worker_t *)arg;
    printf("Споживач #%d використає: %d продуктів\n", worker->id, worker->count);
    for (int i = 0; i < worker->count; i++)
    {
        product_t *product = (product_t *)release(worker->sem);
        printf("Споживач №%d використав: #продукт %s\n", worker->id, product->id);
        free(product);
    }
    return NULL;
}

int main()
{
    int to_produce_counts[PRODUCERS_COUNT];
    int to_consume_counts[CONSUMERS_COUNT];
    get_job_list(to_produce_counts, TOTAL_PRODUCT_COUNT, PRODUCERS_COUNT);
    get_job_list(to_consume_counts, TOTAL_PRODUCT_COUNT, CONSUMERS_COUNT);

    semaphore_buffer_t *sem = new_semaphore(STORAGE_CAPACITY);

    worker_t producers[PRODUCERS_COUNT];
    worker_t consumers[CONSUMERS_COUNT];

    pthread_t producer_threads[PRODUCERS_COUNT];
    pthread_t consumer_threads[CONSUMERS_COUNT];

    for (int i = 0; i < PRODUCERS_COUNT; i++)
    {
        producers[i] = *new_worker(i, to_produce_counts[i], sem);
        pthread_create(&producer_threads[i], NULL, producer_thread, &producers[i]);
    }

    for (int i = 0; i < CONSUMERS_COUNT; i++)
    {
        consumers[i] = *new_worker(i, to_consume_counts[i], sem);
        pthread_create(&consumer_threads[i], NULL, consumer_thread, &consumers[i]);
    }

    for (int i = 0; i < PRODUCERS_COUNT; i++)
    {
        pthread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < CONSUMERS_COUNT; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }

    free_semaphore(sem);

    return 0;
}