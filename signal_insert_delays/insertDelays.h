#ifndef insertDelays_h
#define insertDelays_h

// Used for exiting
#define errExit(msg)	do { close_insertDelays(); perror(msg); exit(EXIT_FAILURE); } while (0)
#define normExit()	do { close_insertDelays(); exit(EXIT_SUCCESS); } while (0)

// Initial values
#define INITIAL_PERIOD 0.1
#define INITIAL_DUTY 0.5

#define MODE_NO_PID 0
#define MODE_LIMIT 1
#define MODE_PID_DIRTY 2

#define MAX_DUTY 0.99
#define MIN_DUTY 0.01

// Called at the beginning. Sets the default values.
void setup_insertDelays(void);

// Called at the end. Frees allocated memory, etc.
void close_insertDelays(void);


void update_duty(double d);
void do_work(void);
void update_period(double p, timer_t timerid);
void stop_timer(timer_t timerid);

void start_timer(timer_t timerid);
int set_pid(int p,timer_t timerid);
void usage();
int set_default_parents(void);
int cmdline(int argc, char **argv);

#endif
