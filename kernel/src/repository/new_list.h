#ifndef KERNEL_NEW_LIST_REPOSITORY_H
#define KERNEL_NEW_LIST_REPOSITORY_H

#include "lists/lists.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

bool initialize_repository_new();
bool destroy_repository_new();

bool find_and_lock_new_list(int);
void create_new(int);
bool delete_new(int);
void unlock_new_list();

#endif