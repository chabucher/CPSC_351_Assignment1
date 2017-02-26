//
//  functions.cpp
//  Assignment1
//
//  Created by Charles Bucher on 2/23/17.
//  Copyright © 2017 Charles Bucher. All rights reserved.
//

#include "functions.hpp"

// Generates a string of ASCII characters for use as data
// Takes in an integer that specifies how many bytes/chars the string contains
char * GenerateData (int numberOfBytes) {
    char * result = new char[numberOfBytes];
    
    char value = 65;
    for (int i = 0; i < numberOfBytes; i++) {
        result[i] = value++;
        
        if (value > 126)
            value = 65;
    }
    
    result[numberOfBytes-1] = '\0';
    
    return result;
}

//Verify that data in some char buffer matches what GenerateData would
//create for that same size buffer.
bool verifyData(char * data, int size) {
    char * correct = GenerateData(size);
    
    for(int i = 0; i < size; i++) {
        if (data[i] != correct[i]) {
            return false;
        }
    }
    
    return true;
}

void cleanup(int mailbox) {
    msgctl(mailbox, IPC_RMID, 0); //Remove everything from the mailbox
}

int get_mailbox_id(const char * path, int id) {

    int key = ftok(path, id); //Generate unique mailbox key
    
    return key;
}

void check_error(int status, const char * message) {
    if (status < 0) {
        throw message;
    }
}
