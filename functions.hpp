//
//  functions.hpp
//  Assignment1
//
//  Created by Charles Bucher on 2/23/17.
//  Copyright © 2017 Charles Bucher. All rights reserved.
//

#ifndef functions_hpp
#define functions_hpp

#include <iostream>
#include <sys/msg.h> //Used for message queue calls

char * GenerateData (int);
bool verifyData(char *, int);
void cleanup(int);
int get_mailbox_id(const char *, int);
void check_error(int, const char *);

#endif /* functions_hpp */
