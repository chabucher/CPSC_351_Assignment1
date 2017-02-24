//
//  functions.cpp
//  Assignment1
//
//  Created by Charles Bucher on 2/23/17.
//  Copyright © 2017 Charles Bucher. All rights reserved.
//

#include <iostream>
#include "functions.hpp"

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
