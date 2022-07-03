#include "lib.h"
#include "types.h"
#define N 5

int uEntry(void) {

    // 测试scanf
    int dec = 0;
    int hex = 0;
    char str[6];
    char cha = 0;
    int ret = 0;
    while (1) {
        printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
        ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
        printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
        if (ret == 4)
            break;
    }

    // 测试信号量
    int i = 4;
    sem_t sem;
    printf("Father Process %d: Semaphore Initializing.\n", get_pid());
    ret = sem_init(&sem, 0);
    if (ret == -1) {
        printf("Father Process %d: Semaphore Initializing Failed.\n", get_pid());
        exit();
    }

    ret = fork();
    if (ret == 0) {
        while (i != 0) {
            i--;
            printf("Child Process %d: Semaphore Waiting.\n", get_pid());
            sem_wait(&sem);
            printf("Child Process %d: In Critical Area.\n", get_pid());
        }
        printf("Child Process %d: Semaphore Destroying.\n", get_pid());
        sem_destroy(&sem);
        exit();
    } else if (ret != -1) {
        while (i != 0) {
            i--;
            printf("Father Process %d: Sleeping.\n", get_pid());
            sleep(128);
            printf("Father Process %d: Semaphore Posting.\n", get_pid());
            sem_post(&sem);
        }
        printf("Father Process %d: Semaphore Destroying.\n", get_pid());
        sem_destroy(&sem);
        exit();
    }

    // For lab4.3
    // TODO: You need to design and test the philosopher problem.
    // Note that you can create your own functions.
    // Requirements are demonstrated in the guide.

    //
    //哲学家
    //
    // sem_t forks[5];
    // sem_t mutex;
    // int ret;
    // for (int i = 0; i < 5; ++i) {
    //     ret = sem_init(&forks[i], 1);
    //     if (ret < 0) {
    //         printf("no avail\n");
    //     }
    // }
    // ret = sem_init(&mutex, 1);
    // if (ret < 0) {
    //     printf("no avail\n");
    // }
    // printf("God %d: initiate successfully\n", get_pid());
    // int philo = 0; //set six
    // pid_t pid;
    // for (; philo < 5; philo++) {
    //     pid = fork();
    //     if (pid == 0) {
    //         break;
    //     } else if (pid < 0) {
    //         printf(" no available pcb\n");
    //         exit();
    //     }
    // }

    // //  philosophers: 0 1 2 3 4
    // if (philo < 5) {
    //     printf("Philosopher %d: thinking\n", philo);
    //     sleep(128);
    //     sem_wait(&mutex);
    //     printf("Philosopher %d: pick up left\n", philo);
    //     sem_wait(&forks[philo]);
    //     printf("Philosopher %d: pick up right\n", philo);
    //     sem_wait(&forks[(philo + 1) % 5]);
    //     sem_post(&mutex);
    //     sleep(128);
    //     printf("Philosopher %d: eating\n", philo);
    //     sleep(128);
    //     printf("Philosopher %d: put down left\n", philo);
    //     sem_post(&forks[philo]);
    //     printf("Philosopher %d: put down right\n", philo);
    //     sem_post(&forks[(philo + 1) % 5]);

    //     printf("Process %d released\n", get_pid());
    //     sem_destroy(&forks[philo]);
    //     exit();

    // } else if (philo == 5) {

    //     sleep(128);
    //     printf("God %d: leaving\n", get_pid());
    // }

    //
    //生产者消费者问题
    //
    // sem_t fullBuffers;
    // sem_t emptyBuffers;
    // sem_t mutex;
    // int ret = -1;
    // ret = sem_init(&fullBuffers, 0);
    // if (ret < 0) printf("no avail\n");
    // ret = sem_init(&emptyBuffers, 2); //size of buffer = 2
    // if (ret < 0) printf("no avail\n");
    // ret = sem_init(&mutex, 1);
    // if (ret < 0) printf("no avail\n");
    // printf("God %d: initiate successfully\n", get_pid());
    // int idx = 0;
    // pid_t pid;
    // for (; idx < 5; idx++) {
    //     pid = fork();
    //     if (pid == 0) {
    //         break;
    //     } else if (pid < 0) {
    //         printf(" no available pcb\n");
    //         exit();
    //     }
    // }

    // int merchanCnt = 0;
    // if (idx == 0) {              //producer
    //     while (merchanCnt < 4) { //produce 4 merchandises
    //         sem_wait(&emptyBuffers);
    //         sem_wait(&mutex);
    //         printf("Producer 0: produce\n");
    //         merchanCnt++;
    //         sleep(128);
    //         sem_post(&mutex);
    //         sem_post(&fullBuffers);
    //     }
    //     sleep(256); //avoid advance destroy
    //     sem_destroy(&emptyBuffers);
    //     sem_destroy(&mutex);
    //     sem_destroy(&fullBuffers);
    //     exit();
    // } else if (idx < 5) { //1,2,3,4 consumers;
    //     sem_wait(&fullBuffers);
    //     sem_wait(&mutex);
    //     printf("Consumer %d: consume\n", idx);
    //     sleep(128);
    //     sem_post(&mutex);
    //     sem_post(&emptyBuffers);
    //     exit();
    // }

    //读者写者问题
    // sem_t writeMutex;
    // sem_t cntMutex;
    // int Rcount = 0;
    // store_global(Rcount);
    // int ret = -1;
    // ret = sem_init(&writeMutex, 1);
    // if (ret < 0) printf("no avail\n");
    // ret = sem_init(&cntMutex, 1);
    // if (ret < 0) printf("no avail\n");
    // printf("God %d: initiate successfully\n", get_pid());
    // int idx = 0;
    // pid_t pid;
    // for (; idx < 6; idx++) {
    //     pid = fork();
    //     if (pid == 0) {
    //         break;
    //     } else if (pid < 0) {
    //         printf(" no available pcb\n");
    //         exit();
    //     }
    // }

    // int writeTime = 0;
    // int readTime = 0;
    // if (idx < 3) {
    //     while (writeTime < 2) {
    //         sem_wait(&writeMutex);
    //         printf("Writer %d: write\n", idx);
    //         writeTime++;
    //         sleep(128);
    //         sem_post(&writeMutex);
    //     }
    //     exit();
    // } else if (idx < 6) {
    //     while (readTime < 3) {
    //         sem_wait(&cntMutex);
    //         fetch_global(&Rcount);
    //         if (Rcount == 0) sem_wait(&writeMutex);
    //         ++Rcount;
    //         store_global(Rcount);
    //         sem_post(&cntMutex);

    //         printf("Reader %d: read\n", idx);
    //         readTime++;
    //         printf("reader Rcount: %d\n", Rcount);
    //         sleep(128);

    //         sem_wait(&cntMutex);
    //         fetch_global(&Rcount);
    //         --Rcount;
    //         store_global(Rcount);
    //         if (Rcount == 0) sem_post(&writeMutex);
    //         sem_post(&cntMutex);
    //     }
    //     exit();
    // }

    exit();
    return 0;
}
