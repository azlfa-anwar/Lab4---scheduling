#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

int numofjobs = 0;

struct job {
    int id;
    int arrival;
    int length;
    int tickets;
    int start_time;
    int completion_time;
    int remaining_time;
    struct job *next;
};

struct job *head = NULL;

void policy_FIFO();
void fifo_analysis();
void policy_SJF();
void policy_LT(int slice);
void policy_STCF();
void policy_RR(int slice);
void stcf_analysis();
void rr_analysis();

void append_to(struct job **head_pointer, int arrival, int length, int tickets) {
    struct job *new_job =(struct job *)malloc(sizeof(struct job));
    new_job->id = numofjobs++;
    new_job->arrival = arrival;
    new_job->length = length;
    new_job->tickets = tickets;
    new_job->remaining_time = length;
    new_job->start_time = 0;
    new_job->completion_time = 0;
    new_job->next = NULL;

    if (*head_pointer == NULL) {
        *head_pointer = new_job;
    } else {
        struct job* current = *head_pointer;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_job;
    }
}

void read_job_config(const char* filename) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int tickets = 0;

    char* delim = ",";
    char *arrival = NULL;
    char *length = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if( line[read-1] == '\n' )
            line[read-1] = 0;
        arrival = strtok(line, delim);
        length = strtok(NULL, delim);
        tickets += 100;

        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    fclose(fp);
    if (line) free(line);
}

void policy_STCF() {
    printf("Execution trace with STCF:\n");
    int current_time = 0;

    struct job *current = head;
    while (current != NULL) {
        current->remaining_time = current->length;
        current = current->next;
    }

    int jobs_completed = 0;
    
    while (jobs_completed < numofjobs) {
        struct job *shortest_job = NULL;
        int shortest_length = INT_MAX;
        
        struct job *temp = head;
        while (temp != NULL) {
            if (temp->arrival <= current_time && temp->remaining_time > 0 && temp->remaining_time < shortest_length) {
                shortest_length = temp->remaining_time;
                shortest_job = temp;
            }
            temp = temp->next;
        }

        if (shortest_job == NULL) {
            int next_arrival = INT_MAX;
            temp = head;
            while (temp != NULL) {
                if (temp->arrival > current_time) {
                    next_arrival = min(next_arrival, temp->arrival);
                }
                temp = temp->next;
            }
            current_time = next_arrival;
            continue;
        }

        int time_to_next_arrival = INT_MAX;
        temp = head;
        while (temp != NULL) {
            if (temp->arrival > current_time) {
                time_to_next_arrival = min(time_to_next_arrival, temp->arrival - current_time);
            }
            temp = temp->next;
        }
        
        int run_time = min(time_to_next_arrival, shortest_job->remaining_time);

        if (shortest_job->start_time == 0) {
            shortest_job->start_time = current_time;
        }

        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
               current_time, shortest_job->id, shortest_job->arrival, run_time);

        shortest_job->remaining_time -= run_time;
        current_time += run_time;

        if (shortest_job->remaining_time == 0) {
            shortest_job->completion_time = current_time;
            jobs_completed++;
        }
    }

    printf("End of execution with STCF.\n");
}

void policy_RR(int slice) {
    printf("Execution trace with Round Robin:\n");

    int current_time = 0;
    int jobs_completed = 0;

    struct job *queue[numofjobs];
    int queue_size = 0;

    struct job *current = head;
    while (current != NULL) {
        current->remaining_time = current->length;
        current = current->next;
    }

    while (jobs_completed < numofjobs) {
        current = head;
        while (current != NULL) {
            if (current->arrival <= current_time && current->remaining_time > 0) {
                int already_in_queue = 0;
                for (int i = 0; i < queue_size; i++) {
                    if (queue[i] == current) {
                        already_in_queue = 1;
                        break;
                    }
                }
                if (!already_in_queue) {
                    queue[queue_size++] = current;
                }
            }
            current = current->next;
        }

        if (queue_size == 0) {
            int next_arrival = INT_MAX;
            current = head;
            while (current != NULL) {
                if (current->arrival > current_time) {
                    next_arrival = min(next_arrival, current->arrival);
                }
                current = current->next;
            }
            current_time = next_arrival;
            continue;
        }

        struct job *job_to_run = queue[0];
        
        for (int i = 1; i < queue_size; i++) {
            queue[i - 1] = queue[i];
        }
        queue_size--;

        int run_time = min(slice, job_to_run->remaining_time);

        if (job_to_run->start_time == 0) {
            job_to_run->start_time = current_time;
        }

        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
               current_time, job_to_run->id, job_to_run->arrival, run_time);

        job_to_run->remaining_time -= run_time;
        current_time += run_time;

        if (job_to_run->remaining_time == 0) {
            job_to_run->completion_time = current_time;
            jobs_completed++;
        } else {
            queue[queue_size++] = job_to_run;
        }
    }

    printf("End of execution with Round Robin.\n");
}

void rr_analysis() {
    printf("Begin analyzing Round Robin:\n");
    struct job *current = head;
    int total_turnaround_time = 0;
    int total_waiting_time = 0;
    int total_response_time = 0;

    while (current != NULL) {
        int turnaround_time = current->completion_time - current->arrival;
        int waiting_time = turnaround_time - current->length;
        int response_time = current->start_time - current->arrival;

        printf("Job %d: turnaround time = %d, waiting time = %d, response time = %d\n", 
               current->id, turnaround_time, waiting_time, response_time);

        total_turnaround_time += turnaround_time;
        total_waiting_time += waiting_time;
        total_response_time += response_time;

        current = current->next;
    }

    printf("Average turnaround time = %f\n", (float)total_turnaround_time / numofjobs);
    printf("Average waiting time = %f\n", (float)total_waiting_time / numofjobs);
    printf("Average response time = %f\n", (float)total_response_time / numofjobs);
    printf("End of analysis with Round Robin.\n");
}

void stcf_analysis() {
    printf("Begin analyzing STCF:\n");
    struct job *current = head;
    int total_turnaround_time = 0;
    int total_waiting_time = 0;
    int total_response_time = 0;

    while (current != NULL) {
        int turnaround_time = current->completion_time - current->arrival;
        int waiting_time = turnaround_time - current->length;
        int response_time = current->start_time - current->arrival;

        printf("Job %d: turnaround time = %d, waiting time = %d, response time = %d\n", 
               current->id, turnaround_time, waiting_time, response_time);

        total_turnaround_time += turnaround_time;
        total_waiting_time += waiting_time;
        total_response_time += response_time;

        current = current->next;
    }

    printf("Average turnaround time = %f\n", (float)total_turnaround_time / numofjobs);
    printf("Average waiting time = %f\n", (float)total_waiting_time / numofjobs);
    printf("Average response time = %f\n", (float)total_response_time / numofjobs);
    printf("End of analysis with STCF.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <analysis> <policy> <slice> <job_config>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int analysis = atoi(argv[1]);
    char *pname = argv[2];
    int slice = atoi(argv[3]);
    char *job_config = argv[4];

    read_job_config(job_config);

    if (strcmp(pname, "STCF") == 0) {
        policy_STCF();
        if (analysis == 1) {
            stcf_analysis();
        }
    } else if (strcmp(pname, "RR") == 0) {
        policy_RR(slice);
        if (analysis == 1) {
            rr_analysis();
        }
    }

    return 0;
}
