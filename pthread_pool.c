#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 5

// 任务结构体
typedef struct {
    void* (*function)(void*);  // 线程函数指针
    void* arg;  // 函数参数
} Task;

// 线程池结构体
typedef struct {
    pthread_t* threads;  // 线程数组
    Task* tasks;  // 任务数组
    int thread_count;  // 线程数量
    int task_count;  // 任务数量
    int head;  // 任务队列头部索引
    int tail;  // 任务队列尾部索引
    int is_shutdown;  // 是否关闭线程池
    pthread_mutex_t mutex;  // 互斥锁
    pthread_cond_t condition;  // 条件变量
} ThreadPool;

ThreadPool threadPool;  // 全局线程池实例

// 线程函数
void* get_pthread_playurl(void* arg) {
    printf("11111\n");
    return NULL;
}

void* thread_function(void* arg) {
    while (1) {
        Task task;

        pthread_mutex_lock(&threadPool.mutex);

        // 等待任务队列非空
        while (threadPool.task_count == 0 && !threadPool.is_shutdown) {
            pthread_cond_wait(&threadPool.condition, &threadPool.mutex);
        }

        // 线程池已关闭，退出线程
        if (threadPool.is_shutdown) {
            pthread_mutex_unlock(&threadPool.mutex);
            pthread_exit(NULL);
        }

        // 从任务队列取出任务
        task.function = threadPool.tasks[threadPool.head].function;
        task.arg = threadPool.tasks[threadPool.head].arg;
        threadPool.head = (threadPool.head + 1) % THREAD_POOL_SIZE;
        threadPool.task_count--;

        pthread_mutex_unlock(&threadPool.mutex);

        // 执行任务
        task.function(task.arg);
    }

    return NULL;
}



// 初始化线程池
void thread_pool_init() {
    threadPool.threads = (pthread_t*)malloc(sizeof(pthread_t) * THREAD_POOL_SIZE);
    threadPool.tasks = (Task*)malloc(sizeof(Task) * THREAD_POOL_SIZE);
    threadPool.thread_count = 0;
    threadPool.task_count = 0;
    threadPool.head = 0;
    threadPool.tail = 0;
    threadPool.is_shutdown = 0;
    pthread_mutex_init(&threadPool.mutex, NULL);
    pthread_cond_init(&threadPool.condition, NULL);

    // 创建线程，并设置为分离状态
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threadPool.threads[i], &attr, thread_function, NULL);
        threadPool.thread_count++;
    }
    
    pthread_attr_destroy(&attr);
}

// 添加任务到线程池
void thread_pool_add_task(void* (*function)(void*), void* arg) {
    pthread_mutex_lock(&threadPool.mutex);

    // 任务队列已满，等待任务队列有空闲位置
    while (threadPool.task_count == THREAD_POOL_SIZE) {
        pthread_cond_wait(&threadPool.condition, &threadPool.mutex);
    }

    // 添加任务到任务队列
    threadPool.tasks[threadPool.tail].function = function;
    threadPool.tasks[threadPool.tail].arg = arg;
    threadPool.tail = (threadPool.tail + 1) % THREAD_POOL_SIZE;
    threadPool.task_count++;

    // 唤醒等待条件变量的线程
    pthread_cond_signal(&threadPool.condition);

    pthread_mutex_unlock(&threadPool.mutex);
}

// 销毁线程池
void thread_pool_destroy() {
    threadPool.is_shutdown = 1;

    // 唤醒所有等待条件变量的线程
    pthread_cond_broadcast(&threadPool.condition);

    // 等待线程结束
    int i;
    for (i = 0; i < threadPool.thread_count; i++) {
        pthread_join(threadPool.threads[i], NULL);
    }

    // 释放资源
    free(threadPool.threads);
    free(threadPool.tasks);
}

int main() {
    // 初始化线程池
    thread_pool_init();

    // 创建并添加任务到线程池
    pthread_t pthRequest;
    pthread_create(&pthRequest, NULL, get_pthread_playurl, mcl_pdeqplaylist_x);
    thread_pool_add_task(get_pthread_playurl, mcl_pdeqplaylist_x);

    // 等待任务执行完成
    pthread_join(pthRequest, NULL);

    // 销毁线程池
    thread_pool_destroy();



    return 0;
}

