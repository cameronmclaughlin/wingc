#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include "types.h"
#include "linked.h"

// fuckkk fpemdas parsing fuckkkk
/*
    okay outline
    fpemdas is basically just pemdas except f is for function so before everything it gets the function value.
    its made with a binary tree where the left side is argument 1 (e.g %rax for add) and the right is argument 2 (e.g %rbx for add)
    the parser goes through in reverse fpemdas order and separates it like that, so the top of the binary tree will be the final step
    so in an equation like 1-2+3 the tree would be
                    1-2+3
                      /\
                     /  \
                    1 -  2+3
                          /\
                         /  \
                         2 + 3
    also if it finds multipe of the same priority the first one found (the leftmost one) will take priority
    so pseudocode is like
    ```
    operator target = subtract
    binaryTree head
    binaryTree curTree = head;
    token match
    while(true):
        target = nextTargetOp()
        match = none
        for token in tokenList:
            if(
            (token.type == token_operator && token.operator == target ||
            (target == function && token.type == token_function)) &&
            match == none && 
            token.seen == false
            ):
                match = token
                token.seen = true
        
        binaryTree leftTree
        binaryTree rightTree
        split(binaryTree, leftTree, rightTree, tokenList, token)
    ```
*/