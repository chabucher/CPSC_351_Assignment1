#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <util/util.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
using namespace std;

#define MY_MQ_NAME "/my_mq"

pthread_t thread1;
pthread_t thread2;

static struct mq_attr my_mq_attr;
static mqd_t my_mq; //Message Queue descriptor

static unsigned int counter; //Information to be passed between threads

void thread1_main(void); //Start thread 1 process
void thread2_main(void); //Start thread 2 process

// Generates a string of ASCII characters for use as data
// Takes in an integer that specifies how many bytes/chars the string contains
char * GenerateData (int numberOfBytes)
{
    char * result = new char[numberOfBytes];
    
    char value = 65;
    for (int i = 0; i < numberOfBytes; i++)
    {
        result[i] = value++;
        
        if (value > 126)
            value = 65;
    }
    
    result[numberOfBytes-1] = '\0';
    
    return result;
}


//Used for ^C exiting
void sig_handler(int signum) {
    if (signum != SIGINT) {
        printf("Received invalid signum = %d in sig_handler()\n", signum);
    }
    
    printf("Received SIGINT. Exiting Application\n");
    
    pthread_cancel(thread1);
    pthread_cancel(thread2);
    
    mq_close(my_mq);
    mq_unlink(MY_MQ_NAME);
    
    exit(0);
}

class Parent
{
private:
    const int p_id;
    
public:
    
    Parent(const int p_id): p_id(p_id) {}
    
    ~Parent() {};
    
    int get_p_id()
    {
        return p_id;
    }
    
    
};

void parent ()
{
    
    // TODO: Setup POSIX message queues for the parent and child processes. (5 pts)
    
    // TODO: Transfer the data string to the child process. Send a message to the child process using
    // the message queue setup earlier. The message should communicate the size of the data being
    // transferred and the IPC mode the parent is using. This transfer will be over a Named Pipe.
    // Call the Named Pipe CPSC351-PIPE (10 pts)
    
    // TODO: Transfer the data string to the child process. Send a message to the child process using
    // the message queue setup earlier. The message should communicate the size of the data being
    // transferred and the IPC mode the parent is using. This transfer will use shared memory.
    // Use a bounded buffer to synchronize the transfer between the parent and child processes. (20 pts)
    
    // TODO: Terminate the child process and then proceed to cleanly
    // end the parent process. (5 pts)
    
    // TODO: Wait for a message from the child process. The message will
    // indicate if the transfer was successful. Display the result
    // to the terminal for the user to see. (10 pts)
}

void child ()
{
    // TODO: Setup POSIX message queues for the parent and child processes. (5 pts)
    
    // TODO: Wait to receive a message from the parent process. Ready the
    // child process to receive data using the method specified in the message. (5 pts)
    
    // TODO: Receive data from the parent using a named pipe. (5 pts)
    
    // TODO: Receive data from the parent using shared memory. (15 pts)
    
    // TODO: Verify the received data by comparing it with a generated string of
    // the same size. Send a message to the parent process that indicates the
    // result. (5 pts)
}

/*
int main(int argc, char **argv)
{
    
    signal(SIGINT, sig_handler);
    
    int size; // Where cmd line argument will be stored
    
    if (argc < 2) // Ensure user passed in correct number of arguments
    {
        cerr << "Nothing was passed in.\nPlease enter the size " <<
        "in bytes, of the string that will be sent to the " <<
        "child process using IPC.\n";
        return 0; //Exit
    }
    else if (argc == 2)
    {
        size = atoi(argv[1]);
    }
    else {
        cerr << "Too many arguments were passed in.\n";
        return 0; //Exit
    }
    
    char * char_string = GenerateData(size); // Create string of characters 'size' long
    
    
    // TODO: Launch the child process. Display the pid of the parent to the
    // to the terminal. The child process should also display its pid after
    // launch. (5 pts)
    
    return 0;
}
 */

int main(void) {
    pthread_attr_t attr;
    int status;
    
    signal(SIGINT, sig_handler);
    
    counter = 0;
    
    my_mq_attr.mq_maxmsg = 10;
    my_mq_attr.mq_msgsize = sizeof(counter);
    
    my_mq = mq_open(MY_MQ_NAME, \
                    O_CREAT | O_RDWR | O_NONBLOCK, \
                    0666, \
                    &my_mq_attr);
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);
    
    printf("Creating thread1\n");
    status = pthread_create(&thread1, &attr, (void*)&thread1_main, NULL);
    if (status != 0) {
        printf("Failed to create thread1 with status = %d\n", status);
    }
    
    printf("Creating thread2\n");
    status = pthread_create(&thread2, &attr, (void*)&thread2_main, NULL);
    if (status != 0) {
        printf("Failed to create thread2 with status = %d\n", status);
    }
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    sig_handler(SIGINT);
    
    return 0;
}

void thread1_main(void) {
    unsigned int exec_period_usecs;
    int status;
    
    exec_period_usecs = 1000000; /*in micro-seconds*/
    
    printf("Thread 1 started. Execution period = %d uSecs\n",\
           exec_period_usecs);
    while(1) {
        status = mq_send(my_mq, (const char*)&counter, sizeof(counter), 1);
        usleep(exec_period_usecs);
    }
}


void thread2_main(void) {
    
    unsigned int exec_period_usecs;
    int status;
    int recv_counter;
    
    exec_period_usecs = 10000; /*in micro-seconds*/
    
    printf("Thread 2 started. Execution period = %d uSecs\n",\
           exec_period_usecs);
    
    while(1) {
        status = mq_receive(my_mq, (char*)&recv_counter, \
                            sizeof(recv_counter), NULL);
        
        if (status > 0) {
            printf("RECVd MSG in THRD_2: %d\n", recv_counter);
            counter += 1;
        }
        
        usleep(exec_period_usecs);
    }
}
