#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 1

typedef double MathFunc_t(double);

#define FILENAME "dataFile.txt"

double gaussian(double x)
{
    return exp(-(x*x)/2) / (sqrt(2 * M_PI));
}

double chargeDecay(double x)
{
    if (x < 0) {
        return 0;
    } else if (x < 1) {
        return 1 - exp(-5*x);
    } else {
        return exp(-(x-1));
    }
}

#define NUM_FUNCS 3
static MathFunc_t* const FUNCS[NUM_FUNCS] = {&sin, &gaussian, &chargeDecay};

//Integrate using the trapezoid method.



typedef struct {
    //What does the worker Need??
    double *area;
    double local_Area;
    double start;
    double finish;
    double dx;
    size_t steps;
    MathFunc_t* func;
    pthread_mutex_t *lock;
    pthread_t thread;

} Worker;


void *runFunction(void *ptr)
{
    //Cast pointer information to Worker class
    Worker *worker = (Worker*)ptr;

    for (size_t i = 0; i < worker->steps; i++) {
        double smallx = worker->start + i*worker->dx;
        double bigx = worker->start + (i+1)*worker->dx;

        worker->local_Area += worker->dx * (  worker->func(smallx) + worker->func(bigx) ) / 2; //Would be more efficient to multiply area by dx once at the end.
    }

    //Mutex Lock around the Memory of area to prevent breaks or deadlocks
    pthread_mutex_lock(worker->lock);
    *worker->area += worker->local_Area;
    pthread_mutex_unlock(worker->lock);

    return NULL;
}

//Integrate using the trapezoid method.
double integrateTrap(MathFunc_t* func, double rangeStart, double rangeEnd, size_t numSteps)
{
    double rangeSize = rangeEnd - rangeStart;
    double dx = rangeSize / numSteps;
    double threadCheckSize = rangeSize / MAX_THREADS;
    size_t workerSteps = numSteps/MAX_THREADS;

    //Globally define Area
    double area = 0;

    Worker workers[MAX_THREADS];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    for (int i = 0; i < MAX_THREADS; i++) {

        Worker *worker = &workers[i];


        worker->area = &area; // Pass the global total into each thread
        worker->local_Area = 0;
        worker->start = rangeStart + threadCheckSize * i;
        worker->finish = rangeStart + threadCheckSize * (i+1);
        worker->dx = dx;
        worker->steps = workerSteps;
        worker->func = func;
        worker->lock = &lock;

        pthread_create(&worker->thread, NULL, runFunction, worker);
    }

    for (int i = 0; i < MAX_THREADS; ++i) {
        // Wait for ith thread to finish
        pthread_join(workers[i].thread, NULL);
    }



    return area;
}


/*
 * Linked list section
 */

//Linked list struct
struct Link {
    struct Link *next;
    int start;
    int finish;
    int step;
    int type;
};

struct Link *append(int start_, int finish_, int step_, int type_, struct Link *head) {
    //Malloc the next Link to create space
    struct Link *next_Head = (struct Link*)malloc(sizeof(struct Link));

    //Save data to points
    next_Head->next = head;
    next_Head->start = start_;
    next_Head->finish = finish_;
    next_Head->step = step_;
    next_Head->type = type_;

    return next_Head;
}

struct Link* parseFile() {

    int start;
    int finish;
    int step;
    int function;

    struct Link *first = (struct Link*)malloc(sizeof(struct Link));
    struct Link *next_Head;

    FILE *file = fopen(FILENAME, "r");

    //Check for Success EXIT on failure
    if (file == NULL) {
        perror("File Failed to open");
        exit(2);
    }

    //Create first link, while loop fails without this step
    if (fscanf(file, "%i %i %i %i", &start, &finish, &step, &function) == 4) {
        next_Head = append(start, finish, step, function, first);
    }

    //while through it
    while(fscanf(file, "%i %i %i %i", &start, &finish, &step, &function) == 4) {
        next_Head = append(start, finish, step, function, next_Head);
        first = next_Head;
    }

    return first;
}

void free_list(struct Link *link_List) {

    //For loop free each element in the liked list while transitioning
    for (struct Link *l = link_List; l != NULL; l = l->next) {
        //Save the location of the next pointer
        struct Link *next = l->next;

        //Burn it
        free(l);

        //Set l pointer to next
        l = next;
    }
}

int main(void)
{
    //Get the initial time for results
    time_t mytime = time(NULL);
    int count = 0;
    double result;

    struct Link *linkedList = parseFile();

    for (struct Link *l = linkedList; l != NULL; l = l->next) {
        //Complete each function
        printf("%i %i %i %i \n", l->start, l->finish, l->step, l->type);
        result = integrateTrap(FUNCS[l->type], l->start, l->finish, l->step);
        count++;
        time_t execution_Time = time(NULL) - mytime;
        printf("Executing %i equations took: %ld seconds \n", count, execution_Time);
        printf("Result was: %f \n", result);
    }
    free_list(linkedList);
    exit(0);
}
