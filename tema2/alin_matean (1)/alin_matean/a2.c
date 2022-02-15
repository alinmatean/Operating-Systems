#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "a2_helper.h"

pid_t pid1, pid2, pid3, pid4, pid5, pid6, pid7, pid8;
pthread_mutex_t lock5_1_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5_4_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
int var514 = 0;
int var541 = 0;

pthread_mutex_t lockP2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP2 = PTHREAD_COND_INITIALIZER;
int cnt = 0;
 

sem_t *logSem = NULL;
sem_t *logSem2 = NULL;

sem_t *logSemP2 = NULL;
//sem_t *sem12 = NULL;

typedef struct{
    pid_t processId;
    int threadId;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
}TH_STRUCT;

void *thread_fn(TH_STRUCT *param)
{
    if(param->processId == 2){
        sem_wait(logSemP2);
        info(BEGIN, param->processId, param->threadId);
        info(END, param->processId, param->threadId);
        sem_post(logSemP2);
    }
    
    else if(param->processId == 8){
        
        if(param->threadId == 5){
            logSem = sem_open("semaphore2", O_EXCL);
            sem_wait(logSem2);
        
            sem_unlink("semaphore2");
            sem_close(logSem2);
            sem_destroy(logSem2);
        }

        info(BEGIN, param->processId, param->threadId);
        info(END, param->processId, param->threadId);   

        if(param->threadId == 2){
            logSem = sem_open("semaphore", O_EXCL);
            sem_post(logSem);
        } 
    }

    else if(param->processId == 5){
        if(param->threadId== 4){
            while(var514 == 0){
                pthread_cond_wait(&cond2, &lock5_4_1);
            }
        }

        if(param->threadId == 5){
            logSem = sem_open("semaphore", O_EXCL);
            sem_wait(logSem);

            sem_unlink("semaphore");
            sem_close(logSem);
            sem_destroy(logSem);
        }
        
        info(BEGIN, param->processId, param->threadId);

        if(param->threadId == 1){
            var514 = 1;
            pthread_cond_signal(&cond2);
            while(var541 == 0){
                pthread_cond_wait(&cond, &lock5_1_4);
            }
        }
        
        info(END, param->processId, param->threadId);

        if(param->threadId == 5){
            logSem = sem_open("semaphore2", O_EXCL);
            sem_post(logSem2);
        }

        if(param->threadId == 4){
            var541 = 1;
            pthread_cond_signal(&cond);
        }
        
    }

    return NULL;
}

pid_t createProcess(int id)
{
    pid_t pid;
    int status = 0;
    pid = fork();
    if(pid == -1)
    {
        perror("Could not create child process");
        return -1;
    }else if(pid == 0){
        info(BEGIN, id, 0);
        
        if(id == 2){
            pid3 = createProcess(3);
            //waitpid(pid3, &status, 0);
            pid4 = createProcess(4);
            //waitpid(pid4, &status, 0);
            pid8 = createProcess(8);
            waitpid(pid3, &status, 0);
            waitpid(pid4, &status, 0);
            waitpid(pid8, &status, 0);

            TH_STRUCT threads[50];
            pthread_t tid[50];
            for(int i = 1; i <= 50; i++)
            {
                threads[i].processId = 2;
                threads[i].threadId = i;
            }
            for(int i = 1; i <= 50; i++)
            {
                pthread_create(&tid[i], NULL, (void *)thread_fn, &threads[i]);
            }
            for(int i = 1; i <= 50; i++)
            {
                pthread_join(tid[i], NULL);
            }

        }
        if(id == 5){
            pid6 = createProcess(6);
            waitpid(pid6, &status, 0);
            
            TH_STRUCT threads[5];
            pthread_t tid[5];
            for(int i = 1; i <= 5; i++)
            {
                threads[i].processId = 5;
                threads[i].threadId = i;
                //threads[i].lock = &lock5_1_4;
                //threads[i].cond = &cond5_1_4;
            }
            for(int i = 1; i <= 5; i++)
            {
                pthread_create(&tid[i], NULL, (void *)thread_fn, &threads[i]);
            }
            for(int i = 1; i <= 5; i++)
            {
                pthread_join(tid[i], NULL);
            }
        }

        if(id == 8){
            TH_STRUCT threads[5];
            pthread_t tid[5];
            for(int i = 1; i <= 5; i++)
            {
                threads[i].processId = 8;
                threads[i].threadId = i;
            }
            for(int i = 1; i <= 5; i++)
            {
                pthread_create(&tid[i], NULL, (void *)thread_fn, &threads[i]);
            }
            for(int i = 1; i <= 5; i++)
            {
                pthread_join(tid[i], NULL);
            }
        }

        info(END, id, 0);
        exit(0);
    }
    else
        return pid;
}

int main(void){
    init();
    logSem = sem_open("semaphore", O_CREAT, 0644, 0);
    if(logSem == NULL){
        perror("sem error");
        return -1;
    }

    logSem2 = sem_open("semaphore2", O_CREAT, 0644, 0);
    if(logSem2 == NULL){
        perror("sem error");
        return -1;
    }
    
    logSemP2 = sem_open("semaphore_proc_2", O_CREAT, 0644, 4);
    if(logSemP2 == NULL){
        perror("sem error");
        return -1;
    }

    info(BEGIN, 1, 0);
    pid2 = createProcess(2);
    //waitpid(pid2, 0, 0);
    pid5 = createProcess(5);
    //waitpid(pid5, 0, 0);
    pid7 = createProcess(7);
    
    waitpid(pid2, 0, 0);
    waitpid(pid5, 0, 0);
    waitpid(pid7, 0, 0);

    info(END, 1, 0);

    pthread_mutex_destroy(&lock5_1_4);
    pthread_mutex_destroy(&lock5_1_4);
    pthread_mutex_destroy(&lockP2);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    pthread_cond_destroy(&cond2);
    
    sem_unlink("semaphore");
    sem_close(logSem);
    sem_destroy(logSem);

    sem_unlink("semaphore2");
    sem_close(logSem2);
    sem_destroy(logSem2);

    sem_unlink("semaphore_proc_2");
    sem_close(logSemP2);
    sem_destroy(logSemP2);
    
    return 0;
}
