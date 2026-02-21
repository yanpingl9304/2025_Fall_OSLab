#define _POSIX_C_SOURCE 199309L
#include "receiver.h"
#include <time.h>
#include <sys/sem.h>
#include <errno.h>

double time_taken = 0, total_time = 0.0;
struct timespec start, end;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

double time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
}

// semaphore P operation (wait)
void P(int semid, int sem_num) {
    struct sembuf op;
    op.sem_num = sem_num;
    op.sem_op = -1;  // P operation
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("P operation failed");
        exit(EXIT_FAILURE);
    }
}

// semaphore V operation (signal)
void V(int semid, int sem_num) {
    struct sembuf op;
    op.sem_num = sem_num;
    op.sem_op = 1;   // V operation
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("V operation failed");
        exit(EXIT_FAILURE);
    }
}

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr) {

    if (mailbox_ptr->flag == 1) {
        // Message Passing
        P(mailbox_ptr->semid, 1);
        clock_gettime(CLOCK_MONOTONIC, &start);

        if (msgrcv(mailbox_ptr->storage.msqid, message_ptr, sizeof(message_ptr->msgText), 1, 0) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = time_diff(start, end);
        total_time += time_taken;
        V(mailbox_ptr->semid, 0);

    } else if (mailbox_ptr->flag == 2) {
        // Shared Memory
        char* shm_addr = (char*)mailbox_ptr->storage.shm_addr;

        // P(full)
        P(mailbox_ptr->semid, 1);

        clock_gettime(CLOCK_MONOTONIC, &start);

        // Access shared memory
        strncpy(message_ptr->msgText, shm_addr, sizeof(message_ptr->msgText)-1);
        message_ptr->msgText[sizeof(message_ptr->msgText)-1] = '\0';

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = time_diff(start, end);
        total_time += time_taken;
        
        V(mailbox_ptr->semid, 0);
    }
}

int main(int argc, char* argv[]) {
    
    int flag = atoi(argv[1]);
    
    mailbox_t mailbox;
    message_t message;
    mailbox.flag = flag;
    
    if (flag == 1) {

        key_t key = ftok(".", 65);
        mailbox.storage.msqid = msgget(key, 0666 | IPC_CREAT);
        if (mailbox.storage.msqid == -1) { perror("msgget failed"); return 1; }

        // semaphore set index : empty -> 0 , full -> 1
        key_t sem_key = ftok(".", 67);
        mailbox.semid = semget(sem_key, 2, 0666 | IPC_CREAT | IPC_EXCL);
        
        if (mailbox.semid == -1) {
            if (errno == EEXIST) {
                // semaphore 已存在
                mailbox.semid = semget(sem_key, 0, 0666);
                if (mailbox.semid == -1) {
                    perror("semget failed");
                    return 1;
                }
            }
        } else {
            //初始化 semaphores
            union semun arg;
            arg.val = 1;  // empty semaphore val = 1
            if (semctl(mailbox.semid, 0, SETVAL, arg) == -1) {
                perror("semctl empty failed");
                return 1;
            }
            
            arg.val = 0;  // full semaphore val = 0
            if (semctl(mailbox.semid, 1, SETVAL, arg) == -1) {
                perror("semctl full failed");
                return 1;
            }
        }
        
        printf("Message Passing\n");

    } else if (flag == 2) {

        key_t key = ftok(".", 66);
        int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
        if (shmid == -1) { perror("shmget failed"); return 1; }
        
        mailbox.storage.shm_addr = (char*)shmat(shmid, NULL, 0);
        if (mailbox.storage.shm_addr == (char*)-1) { perror("shmat failed"); return 1; }
        
        // semaphore set index : empty -> 0 , full -> 1
        key_t sem_key = ftok(".", 67);
        mailbox.semid = semget(sem_key, 2, 0666 | IPC_CREAT | IPC_EXCL);
        
        if (mailbox.semid == -1) {
            if (errno == EEXIST) {
                // semaphore 已存在
                mailbox.semid = semget(sem_key, 0, 0666);
                if (mailbox.semid == -1) {
                    perror("semget failed");
                    return 1;
                }
            }
        } else {
            //初始化 semaphores
            union semun arg;
            arg.val = 1;  // empty semaphore val = 1
            if (semctl(mailbox.semid, 0, SETVAL, arg) == -1) {
                perror("semctl empty failed");
                return 1;
            }
            
            arg.val = 0;  // full semaphore val = 0
            if (semctl(mailbox.semid, 1, SETVAL, arg) == -1) {
                perror("semctl full failed");
                return 1;
            }
        }
        
        printf("Shared Memory\n");
    }
    
    while (1) {

        receive(&message, &mailbox);
        
        if (strcmp(message.msgText, EXIT_MSG) == 0) {
            printf("Sender exit!\n");
            printf("Time taken in receiving msg: %.6lf s\n", total_time);
            break;
        } else {
            printf("Receiving message: %s\n", message.msgText);
        }
    }
    
    if (flag == 1) {
        msgctl(mailbox.storage.msqid, IPC_RMID, NULL);
    } else if (flag == 2) {
        shmdt(mailbox.storage.shm_addr);
        int shmid = shmget(ftok(".", 66), 1024, 0666);
        shmctl(shmid, IPC_RMID, NULL);
        
        semctl(mailbox.semid, 0, IPC_RMID);
    }
    
    return 0;
}