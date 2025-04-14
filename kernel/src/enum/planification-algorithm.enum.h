#ifndef ENUM_PLANIFICATION_ALGORITHM
#define ENUM_PLANIFICATION_ALGORITHM

#define PLANIFICATION_ENUM_SIZE 3

typedef enum {
    FIFO,
    SJF,
    SRT
} t_planification_algorithm;

t_planification_algorithm planification_from_string(char*);

#endif