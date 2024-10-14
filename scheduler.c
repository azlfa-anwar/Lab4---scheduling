#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

// total jobs
int numofjobs = 0;

struct job {
    // job id is ordered by the arrival; jobs arrived first have smaller job id, always increment by 1
    int id;
    int arrival; // arrival time; safely assume the time unit has the minimal increment of 1
    int length;
    int tickets; // number of tickets for lottery scheduling
    // TODO: add any other metadata you need to track here
    struct job *next;
};

// the workload list
struct job *head = NULL;


void append_to(struct job **head_pointer, int arrival, int length, int tickets){
    // TODO: create a new job and init it with proper data

   
    //create a new job and initialize it:

    struct job *new_job =(struct job *)malloc(sizeof(struct job)); //allocate memory dynamically and cast the pointer returned by malloc as a struct job pointer
    new_job -> id = numofjobs++; //assign current numofjobs, then incremenet it
    //initialize rest of new_job's values
    new_job ->arrival = arrival;
    new_job ->length = length;
    new_job ->tickets = tickets;
    new_job -> next = NULL; //for now, set the next job as null (until we append new jobs to the list)

    //append new job to the end of the linked list:

    if (*head_pointer ==NULL){
        *head_pointer = new_job;
    }
    else{ //if not first element, append to the list
    struct job* current = *head_pointer;
    while (current->next != NULL){
        current = current->next; // traverse through list until we reach the end
    }
    current->next = new_job; //once we reached the end, append the new job

    }


    return;
}


void read_job_config(const char* filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int tickets  = 0;

    char* delim = ",";
    char *arrival = NULL;
    char *length = NULL;

    // TODO, error checking
    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // TODO: if the file is empty, we should just exit with error
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if( line[read-1] == '\n' )
            line[read-1] =0;
        arrival = strtok(line, delim);
        length = strtok(NULL, delim);
        tickets += 100;

        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    fclose(fp);
    if (line) free(line);
}


void policy_SJF()
{
    printf("Execution trace with SJF:\n");

    // TODO: implement SJF policy

    printf("End of execution with SJF.\n");

}


void policy_STCF()
{
    printf("Execution trace with STCF:\n");

    // TODO: implement STCF policy

    printf("End of execution with STCF.\n");
}


void policy_RR(int slice)
{
    printf("Execution trace with RR:\n");

    // TODO: implement RR policy

    printf("End of execution with RR.\n");
}


void policy_LT(int slice)
{
    printf("Execution trace with LT:\n");

    // Ensure deterministic behavior
    srand(42);

    // Calculate total tickets
    struct job *current = head;
    int total_tickets = 0;
    while (current != NULL) {
        total_tickets += current->tickets;
        current = current->next;
    }

    int t = 0; // Track time
    while (head != NULL) {  // Loop until all jobs are done

        // Pick a random winning ticket
        int winning_ticket = rand() % total_tickets;
        int ticket_counter = 0;
        current = head;
        struct job *prev = NULL;

        // Find the job that holds the winning ticket
        while (current != NULL) {
            ticket_counter += current->tickets;
            if (ticket_counter > winning_ticket) {
                // Execute job for min(slice, remaining time)
                int run_time = min(slice, current->length);
                printf("t=%d: [Job %d] won the lottery, running for %d ticks.\n",
                       t, current->id, run_time);
                sleep(run_time);  // Simulate execution time
                t += run_time;
                current->length -= run_time;

                // If the job is finished, remove it from the list
                if (current->length == 0) {
                    printf("t=%d: [Job %d] finished.\n", t, current->id);
                    total_tickets -= current->tickets;

                    // Remove job from the list
                    if (prev == NULL) {
                        head = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    free(current);
                }
                break;  // Exit loop after finding and processing the job
            }
            prev = current;
            current = current->next;
        }
    }

    printf("End of execution with LT.\n");
}
void lt_analysis() {
    struct job *current = head;
    int t = 0;  // Current time
    int total_response_time = 0;
    int total_turnaround_time = 0;
    int total_wait_time = 0;

    printf("Begin analyzing LT:\n");

    // Traverse through jobs and calculate response time, turnaround time, wait time
    while (current != NULL) {
        // Response time: time from job arrival to first execution (in lottery scheduling, this can vary)
        int response_time = t - current->arrival;
        if (response_time < 0) {
            response_time = 0;  // No negative response time
        }

        // Turnaround time: total time from arrival to completion
        int turnaround_time = response_time + current->length;

        // Wait time: total waiting time before being executed
        int wait_time = response_time;

        // Print metrics for each job
        printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n", 
               current->id, response_time, turnaround_time, wait_time);

        // Update totals
        total_response_time += response_time;
        total_turnaround_time += turnaround_time;
        total_wait_time += wait_time;

        // Update time 
        t += current->length;

        // Move to the next job
        current = current->next;
    }

    // Calculate averages
    int job_count = numofjobs;
    double avg_response = (double)total_response_time / job_count;
    double avg_turnaround = (double)total_turnaround_time / job_count;
    double avg_wait = (double)total_wait_time / job_count;

    printf("Average -- Response: %.2f Turnaround: %.2f Wait: %.2f\n", 
           avg_response, avg_turnaround, avg_wait);

    printf("End analyzing LT.\n");
}

void policy_FIFO(){
    printf("Execution trace with FIFO:\n");

    int t = 0;  //keep track of the time
    struct job* current = head;

    while (current != NULL){

        //to account for idle time
        if (current->arrival > t){ t= current->arrival;}

        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",t,current->id, current->arrival, current->length);
        sleep(current->length); //simulate the time the job ran for (execution time)
        t += current->length; //increment time depending on the length of executiion
        current = current->next;
    }



    // TODO: implement FIFO policy

    printf("End of execution with FIFO.\n");
}

// perform analysis function to easily reuse in main

void fifo_analysis(){
    struct job* current = head;
    int t=0; //current time
    int total_response_time =0;
    int total_turnaround_time =0;
    int total_wait_time = 0;

    printf("Begin analyzing FIFO:\n");

    while (current != NULL){
        if (current->arrival > t){t = current->arrival;}
        int response_time = t-current->arrival;
        if(response_time<0){response_time=0;} //set to zero if response time is negative
        int turnaround_time = response_time + current->length; //total time from arrival to completion
        int wait_time = response_time; //for fifo, wait time = response bcs there is no preemption

        //update total times

        total_response_time += response_time;
        total_turnaround_time += turnaround_time;
        total_wait_time += wait_time;

        //print metrics for current job

        printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n", current->id, response_time, turnaround_time, wait_time);

        //update time

        t += current->length;

        //move to the next job

        current = current->next;

    }

    //calculate total/average times

    int job_count = numofjobs;
    double avg_response = (double)total_response_time / job_count;
    double avg_turnaround = (double)total_turnaround_time / job_count;
    double avg_wait = (double)total_wait_time / job_count;

    printf("Average -- Response: %.2f Turnaround: %.2f Wait: %.2f\n", 
           avg_response, avg_turnaround, avg_wait);

    printf("End analyzing FIFO.\n");


}


int main(int argc, char **argv){

    static char usage[] = "usage: %s analysis policy slice trace\n";

    int analysis;
    char *pname;
    char *tname;
    int slice;


    if (argc < 5)
    {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, usage, argv[0]);
		exit(1);
    }

    // if 0, we don't analysis the performance
    analysis = atoi(argv[1]);

    // policy name
    pname = argv[2];

    // time slice, only valid for RR
    slice = atoi(argv[3]);

    // workload trace
    tname = argv[4];

    read_job_config(tname);

    if (strcmp(pname, "FIFO") == 0){
        policy_FIFO();
        if (analysis == 1){
            // TODO: perform analysis
            fifo_analysis();
            
        }
    }
    else if (strcmp(pname, "SJF") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "STCF") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "RR") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "LT") == 0)
    {
        policy_LT(slice);  // Call Lottery Scheduling policy with time slice
        if (analysis == 1) {
            lt_analysis();  // Perform Lottery Scheduling analysis
        }
    }

	exit(0);
}
