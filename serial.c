#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define FILENAME "dataFile.txt"

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

typedef double MathFunc_t(double);

double gaussian(double x)
{
    double returnValue = exp(-(x*x)/2) / (sqrt(2 * M_PI));
    return returnValue;
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
double integrateTrap(MathFunc_t* func, double rangeStart, double rangeEnd, size_t numSteps)
{
    double rangeSize = rangeEnd - rangeStart;
    double dx = rangeSize / numSteps;

    double area = 0;
    for (size_t i = 0; i < numSteps; i++) {
        double smallx = rangeStart + i*dx;
        double bigx = rangeStart + (i+1)*dx;

        area += dx * ( func(smallx) + func(bigx) ) / 2; //Would be more efficient to multiply area by dx once at the end.
    }

    return area;
}

struct Link* parseFile() {

    int start;
    int finish;
    int step;
    int function;

    struct Link *first = (struct Link*)malloc(sizeof(struct Link));
    struct Link *next_Head;
    //struct Link *next_Head = (struct Link*)malloc(sizeof(struct Link));

    FILE *file = fopen(FILENAME, "r");
    //FILE *closingFile = fopen("results.txt", "w");

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
