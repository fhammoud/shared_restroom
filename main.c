/*
* CSE 430 Assignment 3 - 21 October 2014
* Restroom Sharing
*/
//========Included Packages=========//
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>

//========Forward Declaration======//
void *thread_fmain(void *);
void *thread_mmain(void *);
void man_leave();
void man_enter();
void woman_leave();
void woman_enter();
void use_rr();
void do_other_stuff();
int get_simple_tid(pthread_t);

//=========Global variables========//

#define FCOUNT 2
#define MCOUNT 2
#define RR_CAP 2
#define RR_MAXSLEEP 10
#define OTHER_SLEEP 10

//Semaphores and Mutexes
sem_t capacity_semaphore;
pthread_mutex_t semcheck = PTHREAD_MUTEX_INITIALIZER;

//Counters and arrays
int gender; //-1 = female 0 = neutral 1 = male
pthread_t threadIDs[FCOUNT + MCOUNT];

//====Begin Main Method====//
int main()
{
	//Initialize semaphore, mutex, and counter
	sem_init(&capacity_semaphore, 0, RR_CAP);
	pthread_mutex_init(&semcheck, 0);
	gender = 0;

	//Initialize threading 
	pthread_attr_t attribs;
	int i;
	int tids[FCOUNT + MCOUNT];
	pthread_t pthreadids[FCOUNT + MCOUNT];

	srand(time(NULL) % (time_t)RAND_MAX);

	pthread_attr_init(&attribs);
	for (i = 0; i < FCOUNT; i++)
	{
		tids[i] = i;
		pthread_create(&(pthreadids[i]), &attribs, thread_fmain, &(tids[i]));
	}

	for (i = 0; i < MCOUNT; i++)
	{
		tids[i + FCOUNT] = i + FCOUNT;
		pthread_create(&(pthreadids[i]), &attribs, thread_mmain, &(tids[i + FCOUNT]));
	}

	for (i = 0; i < FCOUNT + MCOUNT; i++)
		pthread_join(pthreadids[i], NULL);

	return 0;
}

//Female thread execution
void *thread_fmain(void * arg)
{
	int tid = *((int *)arg);
	threadIDs[tid] = pthread_self();

	while (1 == 1)
	{
		do_other_stuff();
		woman_enter();
		use_rr();
		woman_leave();
	}
}

//Male thread execution
void *thread_mmain(void * arg)
{
	int tid = *((int *)arg);
	threadIDs[tid] = pthread_self();

	while (1 == 1)
	{
		do_other_stuff();
		man_enter();
		use_rr();
		man_leave();
	}
}

//Female tries to enter restroom
//Waits in line if restroom for men only or if female restroom is full
//Enters otherwise
void woman_enter()
{
	int value;
	int id = get_simple_tid(pthread_self());

	int shouldQuit = 0;
	while (shouldQuit == 0) { //Keep checking until gender is allowed in restroom
		pthread_mutex_lock(&semcheck); //Lock resources
		if (gender != 1) { 	// Wait until men are out of the restroom
			if (gender == 0) {
				gender = -1; //restroom now belongs to females
			}
			shouldQuit = 1;
		}
		pthread_mutex_unlock(&semcheck); //Unlock resources
	}
	sem_wait(&capacity_semaphore); //"Enter restroom"
	sem_getvalue(&capacity_semaphore, &value); //Get people in restroom
	printf("Woman %d entering, there are %d women now in the restroom\n", id, (2 - value));
}

//After using the restroom, female will exit
//If last one to leave, switch sign to say empty
void woman_leave()
{
	int value;
	int id = get_simple_tid(pthread_self());

	pthread_mutex_lock(&semcheck);	//Lock resources
	sem_post(&capacity_semaphore); //"Exit restroom"
	sem_getvalue(&capacity_semaphore, &value);
	if (value == 2) //Zero people in the semaphore
		gender = 0; //Restroom is up for grabs
	printf("Woman %d leaving, there are %d women left in the restroom\n", id, (2 - value));
	pthread_mutex_unlock(&semcheck); //Unlock resources
}

//Male tries to enter restroom
//Waits in line if restroom belongs to females or if male restroom is full
//Enters otherwise
void man_enter()
{
	int value;
	int id = get_simple_tid(pthread_self());

	int shouldQuit = 0;
	while (shouldQuit == 0) { //Keep checking until males allowed in restroom
		pthread_mutex_lock(&semcheck); //Lock resources
		if (gender != -1) { //If the restroom does not belong to females
			if (gender == 0) {
				gender = 1; //restroom now belongs to males
			}
			shouldQuit = 1;
		}
		pthread_mutex_unlock(&semcheck); //Unlock resources
	}
	sem_wait(&capacity_semaphore); //Enter restroom
	sem_getvalue(&capacity_semaphore, &value);
	printf("Man %d entering, there are %d men now in the restroom\n", id, (2 - value));
}

//Males leaves restroom after using it
//If last male, switch sign to empty
void man_leave()
{
	int value;
	int id = get_simple_tid(pthread_self());
	pthread_mutex_lock(&semcheck); //Lock resources
	sem_post(&capacity_semaphore); //"Exit res
	sem_getvalue(&capacity_semaphore, &value);
	if (value == 2) //If no more men in the restroom
		gender = 0; //Sign says empty - either gender may enter first
	printf("Man %d leaving, there are %d men left in the restroom\n", id, (2 - value));
	pthread_mutex_unlock(&semcheck); //Unlock resources
}

//Male and female threads use restroom for random amount of time
void use_rr()
{
	struct timespec req, rem;
	double usetime;
	usetime = RR_MAXSLEEP * (rand() / (1.0*(double)((unsigned long)RAND_MAX)));
	req.tv_sec = (int)floor(usetime);
	req.tv_nsec = (unsigned int)((usetime - (int)floor(usetime)) * 1000000000);
	printf("Thread %d using restroom for %lf time\n", get_simple_tid(pthread_self()), usetime);
	nanosleep(&req, &rem);
}

//Threads "work" before needing restroom
void do_other_stuff()
{
	struct timespec req, rem;
	double worktime;
	worktime = OTHER_SLEEP * (rand() / (1.0*(double)((unsigned long)RAND_MAX)));
	req.tv_sec = (int)floor(worktime);
	req.tv_nsec = (unsigned int)((worktime - (int)floor(worktime)) * 1000000000);
	printf("Thread %d working for %lf time\n", get_simple_tid(pthread_self()), worktime);
	nanosleep(&req, &rem);
}

//Returns id of thread
int get_simple_tid(pthread_t lid)
{
	for (int i = 0; i < FCOUNT + MCOUNT; i++)
		if (pthread_equal(lid, threadIDs[i]))
			return i;
	printf("Oops! did not find a tid for %lu\n", lid);
	_exit(-1);
}