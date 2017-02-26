#include <iostream>
#include <sys/msg.h> //Used for message queue calls
#include <unistd.h> //Used for process calls
#include <sys/stat.h> //Used for umask(0) in FIFO section
#include <sys/mman.h> //Used for POSIX shared memory
#include <fcntl.h> //Used for system flags
#include "functions.hpp"
using namespace std;

#define MY_FIFO "CPSC351-PIPE" //Named pipe name
#define MY_PATH "/tmp" //Path to create unique mailbox key
#define BSIZE 10

//Message struct
struct msg_buff {
    long mType;
    int mSize;
};

struct MyStruct
{
    char s_buffer[BSIZE];
};

//Messages to send & receive via message queue
msg_buff msg_send;
msg_buff msg_recv;

int msgq_id; //Mailbox id

int data_size; //Size of data array
char * data; //Data array

//Process prototypes
void parent(int);
void child(int);
int consumer(int);
void producer();


int main(int argc, const char * argv[]) {
    cout << "----Beginning of program-----\n";
    
    //Ensure user passed in correct number of arguments
    if (argc < 2) {
        cerr << "Not enough arguments!\n";
        return 1;
    }
    else if (argc == 2) {
        data_size = atoi(argv[1]);
        data = GenerateData(data_size);
    }
    else {
        cerr << "Too many argument!\n";
        return 1;
    }
    
    try {
        msgq_id = msgget(get_mailbox_id(MY_PATH, 2), 0666 | IPC_CREAT);
        if(msgq_id < 0 ){
            throw "Child message queue creation failure\n";
        }
        
        //Create parent process and fork child process
        pid_t pid = fork();
        check_error(pid, "[Main] Child failed to launch!\n");
        
        //Ensure process forked sucessfully
        if (pid == 0) {
            child(msgq_id); //Call child
        }
        else {
            cout << "[Parent] created string of " << data_size << " bytes\n";
            parent(msgq_id); //Call parent
        }
        
    } catch (const char * e) {
        cleanup(msgq_id); //Clear out message queues before exiting
        cerr << "Exception: " << e; //Display thrown error message
        return 1;
    } catch (...) {
        cout << "Some other error!\n";
        return 1;
    }
    
    cleanup(msgq_id); //Clear out message queues before exiting.
    cout << "----End of program-----------\n\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

//                                PARENT PROCESS                             //

///////////////////////////////////////////////////////////////////////////////


//Parent process: takes the parent & child mailboxes
void parent(int mailbox) {
    cout << "[Parent] pid: " << getpid() <<endl; //Display we started.
    int iStat; //int ret val erro checking
    long lStat; //long ret val error checking
    bool success; //Child received data correctly
    
    
    //                       SEND MESSAGE - NAMED PIPES                      //
    msg_send.mType = 1;
    msg_send.mSize = data_size;
    
    iStat = msgsnd(mailbox, &msg_send, sizeof(msg_send), 0);
    check_error(iStat, "[Parent] Message failed to send!\n");
    cout << "[Parent] Sent message to child, mode "
         << msg_send.mType<< " (Name pipes)\n";
    
    
    //                             WRITE TO FIFO                             //
    FILE *fp;
    fp = fopen(MY_FIFO, "w");
    if(fp == NULL) {
        umask(0);
        if(mknod(MY_FIFO, 0666 | S_IFIFO,0)) {
            throw "[Parent] Pipe created unsuccessfully";
        }
        fp = fopen(MY_FIFO, "w");
    }
    fwrite(data, sizeof(char), data_size, fp); //Write data to the pipe
    fclose(fp); //Close the pipe connection
    sleep(1); //Parent finished writing and will wait 2 seconds for child
    
    
    
    //                        RECEIVE SUCCESS REPORT                         //
    lStat = msgrcv(mailbox, &msg_recv, sizeof(msg_recv),0,0);
    check_error((int)lStat, "[Parent] Failed to receive message!\n");
    success = msg_recv.mSize;
    cout << "[Parent] Data Verification: "
         << (success ? "PASSED" : "FAILED") << endl;
    
    
    //                      SEND MESSAGE - SHARED MEMORY                     //
    msg_send.mType = 2;
    msg_send.mSize = data_size;
    
    iStat = msgsnd(mailbox, &msg_send, sizeof(msg_send), 0);
    check_error(iStat, "[Parent] Message failed to send!\n");
    cout << "[Parent] Sent message to child, mode "
         << msg_send.mType<< " (Shared memory)\n";
    
    
    //                        SEND DATA - SHARED MEMORY                      //
    producer();
    wait(NULL); //Wait for child process to finish before receiving message back
    
    
    //                        RECEIVE SUCCESS REPORT                         //
    lStat = msgrcv(mailbox, &msg_recv, sizeof(msg_recv),0,0);
    check_error((int)lStat, "[Parent] Failed to receive message!\n");
    success = msg_recv.mSize;
    cout << "[Parent] Data Verification: "
         << (success ? "PASSED" : "FAILED") << endl;
    
    exit(0); //Exit process
}

///////////////////////////////////////////////////////////////////////////////

//                                CHILD PROCESS                              //

///////////////////////////////////////////////////////////////////////////////

//Child process: takes the parent & child mailboxes
void child(int mailbox) {
    cout << "[Child] pid: " << getpid() <<endl;  //Display we started.
    int iStat; //int ret value error checking
    long lStat; //long ret value error checking
    int size; //Data size to be received via msg queue
    bool success; //Data validation success to be sent via msg queue
    
    
    //                    RECEIVE MESSAGE - NAMED PIPES                      //
    lStat = msgrcv(mailbox, &msg_recv, sizeof(msg_recv),0,0);
    check_error((int)lStat, "[Child] Failed to receive message!\n");
    
    //Save message data to local variables
    size = msg_recv.mSize;
    cout << "[Child] Message received, mode 1 (Name pipes)\n";
    
    
    //                            READ FROM FIFO                             //
    FILE * fp; //File pointer the the named pipe
    long lSize; //Size of the named pipe buffer
    char * buffer; //Buffer to write to
    size_t result; //Size of data written
    
    //Open pipe for reading
    fp = fopen(MY_FIFO, "r");
    if (fp == NULL) {
        throw "[Child] Pipe opened unsuccessfully\n";
    }
    
    // obtain pipe file size:
    fseek (fp , 0 , SEEK_END);
    lSize = ftell (fp);
    rewind (fp);
    
    // allocate memory to contain the size of the pipe file:
    buffer = new char[lSize];
    if (buffer == NULL) {
        throw "[Child] Pipe writing buffer memory error\n";
    }
    
    // copy the data from the pipe into the buffer:
    result = fread (buffer, 1, lSize, fp);
    if (result != lSize) {
        throw "[Child] Pipe reading error\n";
    }
    
    //Report success (or failure)
    success = verifyData(buffer, (int)lSize);
    
    // terminate name pipe & free up memory
    fclose (fp);
    delete buffer;
    
    
    //                          SEND SUCCESS REPORT                          //
    msg_send.mType = 1; //Ensure > 0
    msg_send.mSize = success; //Success value
    
    iStat = msgsnd(mailbox, &msg_send, sizeof(int), 0);
    check_error(iStat, "[Child] Message failed to send!\n");
    sleep(5); //Child sent verification and will wait 1 seconds.
    
    
    //                   RECEIVE MESSAGE - SHARED MEMORY                     //
    lStat = msgrcv(mailbox, &msg_recv, sizeof(msg_recv),0,0);
    check_error((int)lStat, "[Child] Failed to receive message!\n");
    
    //Save message data to local variables
    size = msg_recv.mSize;
    cout << "[Child] Message received, mode 2 (Shared memory)\n";
    
    
    //                     RECEIVE DATA - SHARED MEMORY                      //
    success = consumer(size);
    
    
    //                          SEND SUCCESS REPORT                          //
    msg_send.mType = 1; //Ensure > 0
    msg_send.mSize = success; //Success value
    
    iStat = msgsnd(mailbox, &msg_send, sizeof(int), 0);
    check_error(iStat, "[Child] Message failed to send!\n");

    exit(0);
}

void producer() {
    int shm_fd;
    
    shm_fd = shm_open(MY_PATH, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, data_size);
    void * mem = mmap(0, data_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    struct MyStruct * shm_p = (struct MyStruct*)mem;
    
    strcpy(shm_p->s_buffer, data);
}

int consumer(int size) {
    int shm_fd;
    
    shm_fd = shm_open(MY_PATH, O_RDONLY, 0666);
    void * mem = mmap(0, size, PROT_READ, MAP_SHARED, shm_fd, 0);
    struct MyStruct* shm_p = (struct MyStruct*)mem;
    sleep(3);
    bool success = verifyData(shm_p->s_buffer, size);
    
    shm_unlink(MY_PATH);
    
    return success;
}
