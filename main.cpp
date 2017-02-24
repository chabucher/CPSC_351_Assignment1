#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include "functions.hpp"
using namespace std;

#define myFIFO "CPSC351-PIPE"

struct msg_buff
{
    short mType; //1 for pipes, 2 for shared memory
    int mSize; //Size of the character array
} msg;

int msgq_id;
int array_size;
char * data;

void parent();
void child();

int main(int argc, char **argv)
{
    //Ensure user passed in correct number of arguments
    if (argc < 2)
    {
        cerr << "Not enough arguments!\n";
        return 1;
    }
    else if (argc == 2)
    {
        array_size = atoi(argv[1]);
        data = GenerateData(array_size);
    }
    else
    {
        cerr << "Too many argument!\n";
        return 1;
    }
    
    //Create mailbox!
    msgq_id = msgget(1003, 0666 | IPC_CREAT);
    
    if (msgq_id < 0 )
    {
        cerr << "Failed to create message queue\n";
        return 1;
    }

    //Create parent process & fork to child
    pid_t pid = fork();
    
    //Ensure process forked sucessfully
    if (pid < 0)
    {
        cerr << "Child failed to launch!\n";
        exit(1);
    }
    else if (pid == 0)
    {
        child(); //Call child
    }
    else
    {
        cout << "[Parent] created string of " << array_size << " bytes\n";
        parent(); //Call parent
    }
    
    return 0;
}


void parent()
{
    
    cout << "[Parent] pid: " << getpid() <<endl;
    
    //Set values
    msg.mType = 1; //1 for named pipes
    msg.mSize = array_size;
    
    long status = msgsnd(msgq_id, &msg, sizeof(msg), 0);
    if (status < 0)
    {
        cerr << "Message failed to send!\n";
        exit(1);
    }

    if (msg.mType == 1) {
        
        int fifo, status;
        long num;
        char temp[32];
        char string[]="Testing...";
        
        if ((status = mkfifo(myFIFO, 0666)) < 0) {
            cerr << "Pipe made unsuccessfully\n";
            exit(1);
        }
        
        if ((fifo = open(myFIFO, O_WRONLY) < 0)) {
            cerr << "Pipe opened unsuccessfully\n";
            exit(1);
        }
        
        if ((num = write(fifo, string, strlen(string)) < 0)) {
            cerr << "Pipe written unsuccessfully\n";
            exit(1);
        }
        
        if ((fifo = open(myFIFO, O_RDONLY)) < 0) {
            cerr << "Pipe opened unsuccessfully\n";
            exit(1);
        }
        
        if ((num = read(fifo, temp, sizeof(temp))) < 0) {
            cerr << "Pipe read unsuccessfully\n";
            exit(1);
        }
        
        printf("In FIFO is %s \n", temp);
        
        
    }
    
    wait(NULL);
    exit(0); //End parent process
    
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
    cout << "[Child] pid: " << getpid() <<endl;
    
    bool continue_loop = true;
    
    while(continue_loop)
    {
        long status = msgrcv(msgq_id, &msg, sizeof(msg), 0, 0);
        if (status < 0)
        {
            cerr << "Message not received!\n";
            exit(1);
        }
        continue_loop = false;
    }
    exit(0); //End child process
    
    // TODO: Setup POSIX message queues for the parent and child processes. (5 pts)
        
    // TODO: Wait to receive a message from the parent process. Ready the
    // child process to receive data using the method specified in the message. (5 pts)
        
    // TODO: Receive data from the parent using a named pipe. (5 pts)
        
    // TODO: Receive data from the parent using shared memory. (15 pts)
        
    // TODO: Verify the received data by comparing it with a generated string of
    // the same size. Send a message to the parent process that indicates the
    // result. (5 pts)
}
