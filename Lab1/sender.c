#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <sys/sem.h>
#include <errno.h>
#include "sender.h"

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

void send(message_t message, mailbox_t* mailbox_ptr) {
    if (mailbox_ptr->flag == 1) {
        // message passing
        P(mailbox_ptr->semid, 0);
        clock_gettime(CLOCK_MONOTONIC, &start);

        if (msgsnd(mailbox_ptr->storage.msqid, &message, sizeof(message.msgText), 0) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
        V(mailbox_ptr->semid, 1);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = time_diff(start, end);
        total_time += time_taken;

    } else if (mailbox_ptr->flag == 2) {
        // shared memory
        char* shm_ptr = mailbox_ptr->storage.shm_addr;
        
        // P(empty)
        P(mailbox_ptr->semid, 0);

        clock_gettime(CLOCK_MONOTONIC, &start);

        strncpy(shm_ptr, message.msgText, 1024);
        shm_ptr[1023] = '\0';

        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken = time_diff(start, end);
        total_time += time_taken;

        // V(full)
        V(mailbox_ptr->semid, 1);
    }
}

int main(int argc, char* argv[]) {
    
    int flag = atoi(argv[1]);
    const char* filename = argv[2];
    
    mailbox_t mailbox;
    mailbox.flag = flag;
    
    if (flag == 1) {

        key_t key = ftok(".", 65);
        mailbox.storage.msqid = msgget(key, 0666 | IPC_CREAT);
        if (mailbox.storage.msqid == -1) { perror("msgget failed"); return 1; }

        struct msqid_ds buf;
        if (msgctl(mailbox.storage.msqid, IPC_STAT, &buf) == -1) { perror("IPC_STAT"); exit(1); }
        buf.msg_qbytes = 1024;
        if (msgctl(mailbox.storage.msqid, IPC_SET, &buf) == -1) { perror("IPC_SET"); exit(1); }

        // semaphore set index : empty -> 0 , full -> 1
        key_t sem_key = ftok(".", 67);
        mailbox.semid = semget(sem_key, 2, 0666 | IPC_CREAT);
        if (mailbox.semid == -1) { perror("semget failed"); return 1; }
        
        // 初始化 semaphores
        union semun arg;
        arg.val = 1;  // empty semaphore val = 1
        if (semctl(mailbox.semid, 0, SETVAL, arg) == -1) {
            perror("semctl empty failed");
            return 1;
        }
        
        arg.val = 0;  // full semaphore val =  0
        if (semctl(mailbox.semid, 1, SETVAL, arg) == -1) {
            perror("semctl full failed");
            return 1;
        }
        
        printf("Message Passing\n");

    } else if (flag == 2) {

        key_t key = ftok(".", 66);
        int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
        if (shmid == -1) { perror("shmget failed"); return 1; }
        
        mailbox.storage.shm_addr = (char*)shmat(shmid, NULL, 0);
        if (mailbox.storage.shm_addr == (char*)-1) { perror("shmat failed"); return 1; }
        memset(mailbox.storage.shm_addr, 0, 1024);
        
        // semaphore set index : empty -> 0 , full -> 1
        key_t sem_key = ftok(".", 67);
        mailbox.semid = semget(sem_key, 2, 0666 | IPC_CREAT);
        if (mailbox.semid == -1) { perror("semget failed"); return 1; }
        
        // 初始化 semaphores
        union semun arg;
        arg.val = 1;  // empty semaphore val = 1
        if (semctl(mailbox.semid, 0, SETVAL, arg) == -1) {
            perror("semctl empty failed");
            return 1;
        }
        
        arg.val = 0;  // full semaphore val =  0
        if (semctl(mailbox.semid, 1, SETVAL, arg) == -1) {
            perror("semctl full failed");
            return 1;
        }
        
        printf("Shared Memory\n");
    }
    
    FILE* fp = fopen(filename, "r");
    
    message_t message;
    
    while (fgets(message.msgText, sizeof(message.msgText), fp)) {
        message.msgText[strcspn(message.msgText, "\n")] = '\0';
        message.mType = 1;

        send(message, &mailbox);
        
        printf("Sending message: %s\n", message.msgText); 
    }
    
    fclose(fp);
    
    // EXIT_MSG
    strncpy(message.msgText, EXIT_MSG, sizeof(message.msgText)-1);
    message.msgText[sizeof(message.msgText)-1] = '\0';
    message.mType = 1;
    send(message, &mailbox);
    
    printf("End of input file! exit!\n");
    printf("Time taken in sending msg: %.6lf s\n", total_time);

    return 0;
}