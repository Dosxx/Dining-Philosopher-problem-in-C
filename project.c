/*Author: Kekeli Dos Akouete */
/*ICS462 Operating Systems */
/*Project #3 Dining Philosophers */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SLEEP_TIME 3


/* this global variable indicates the philosophers state 
	source from the textbook */
static enum {
		Thinking,
		Hungry,
		Eating
	} status[5];

/* Declaration of mutex lock and condition variable */
static pthread_mutex_t mutexLock;
static pthread_cond_t cond_Signal;
static unsigned randomSeed;
time_t now;
char * mytime;

/* Philosopher struct */
 struct Philosopher{
	int philosopher_number;
	char *name;
	char *state;
	int thinkCount;
	int thinkTime;
	int eatCount;
	int eatTime;
	int hungryCount;
	int hungryTime;
	struct Philosopher *next;
};

char *getTime() {
	struct tm * timeinfo;
	now = time(0);
	timeinfo = localtime(&now);
	sprintf(mytime, "%02d:%02d:%02d",(timeinfo->tm_hour) % 24, timeinfo->tm_min, timeinfo->tm_sec);
	return mytime;
}

/* test to see if both forks are acquired */
static int test_forks(int phil_numb) {
	if((status[(phil_numb + 4) % 5] != Eating) &&
			(status[phil_numb] == Hungry) &&
			(status[(phil_numb + 1) % 5] != Eating)) {
		status[phil_numb] = Eating;
		return(0);
	}
	else return(1);
}

/*Philosopher wishes to eat */
static void pickup_forks(void *Phil) {
	struct Philosopher *self = Phil;
	long start, end;
	pthread_mutex_lock(&mutexLock);

	/* Start the clock for hungry time */
	start = time(&now);

	status[self->philosopher_number] = Hungry;
  	self->state = strdup("Hungry");
	printf("%s: Philosopher %s: %s\n", getTime(), self->name, self->state);

	/* acquiring the forks to eat */
	while (test_forks(self->philosopher_number)) {
		pthread_cond_wait(&cond_Signal, &mutexLock);
	}
  	/*Record hungry time */
  	end = time(&now);
  	self->hungryCount += 1;
	pthread_mutex_unlock(&mutexLock);
	self->hungryTime += (int)(end - start);
  
	self->state = strdup("Eating");
	printf("%s: Philosopher %s: %s\n", getTime(), self->name, self->state);
}

/* philosopher finishes eating */
static void return_forks(void *Phil) {
	struct Philosopher *self = Phil;
	status[self->philosopher_number] = Thinking;
	// pthread_cond_signal(&cond_Signal);
  	self->state = strdup("Thinking");
	printf("%s: Philosopher %s: %s\n", getTime(), self->name, self->state);
	
}

/* philosopher takes turn at eating */
static void *run(void *phil) {
	struct Philosopher *self = phil;
	int sleepTime;

	printf("%s: Philosopher %s: %s\n", getTime(), self->name, self->state);
	sleepTime = (rand_r(&randomSeed) % SLEEP_TIME) + 1;
	sleep(sleepTime);
  	
	
	for(int counter = 0; counter < 5; counter++) {
		pickup_forks(self);
		/* Eat for a random few seconds */
	    sleepTime = (rand_r(&randomSeed) % SLEEP_TIME) + 1;
	    sleep(sleepTime);

	    /* Record eating time */
		self->eatCount += 1;
		self->eatTime += sleepTime;

	    /* Finish eating an return forks */
	    pthread_mutex_lock(&mutexLock);
		return_forks(self);
	    pthread_cond_broadcast(&cond_Signal);
	  	pthread_mutex_unlock(&mutexLock);
	  	
	    /* Think for a while before eating again */
	    sleepTime = (rand_r(&randomSeed) % SLEEP_TIME) + 1;
	    sleep(sleepTime);

	    /*Record thinking time */
	    self->thinkCount += 1;
	    self->thinkTime += sleepTime;
	}
	pthread_cond_broadcast(&cond_Signal);
	printf("%s: Philosopher %s: %s\n", getTime(), self->name, "is FULL");
	return (0);
}

/* Main program */
int main(int argc, char *argv[]){

	mytime = (char*)malloc(10 * sizeof(char));
	pthread_attr_t attr;
	for(int i = 0; i < 5; i ++) {
		status[i] = Thinking;
	}

	/* create 5 philosophers and initialize the status to Thinking */
	struct Philosopher *philosopher, *phil_temp;
	struct Philosopher *head = NULL;

	for(int i = 0; i < 5; i ++) {
		philosopher = malloc(sizeof(struct Philosopher));
		philosopher->philosopher_number = i;
		switch(i){
			case 0:{
				philosopher->name = strdup("Aristotle");
				break;
			}
			case 1:{
				philosopher->name = strdup("Heraclitus");
				break;
			}
			case 2:{
				philosopher->name = strdup("Pythagoras");
				break;
			}
			case 3:{
				philosopher->name = strdup("Plato");
				break;
			}
			case 4:{
				philosopher->name = strdup("Socrates");
				break;
			}
		}
		philosopher->state = strdup("Thinking");
		philosopher->thinkCount = 0;
		philosopher->thinkTime = 0;
		philosopher->eatCount = 0;
		philosopher->eatTime = 0;
		philosopher->hungryCount = 0;
		philosopher->hungryTime = 0;
		philosopher->next = head;
		head = philosopher;
	}

	pthread_t philosopher_thread[5];
	randomSeed = (int) time(NULL);
	srand(randomSeed);

	pthread_mutex_init(&mutexLock, NULL);
	pthread_cond_init(&cond_Signal, NULL);

	pthread_attr_init(&attr);
	int counter = 0;
	for(philosopher = head; philosopher != NULL; philosopher = philosopher->next){
		pthread_create(&philosopher_thread[counter++], &attr, run, (void *)philosopher);
	}

	/* Join all philosopher threads */
	for(int i = 0; i < 5; i++) {
		pthread_join(philosopher_thread[i], NULL);
	}
	printf("\n");
	for(philosopher = head; philosopher != NULL; philosopher = philosopher->next){
		printf("Philosopher %s\n\tEatCount = %d\n\tEatTime = %d\n\tHungryCount = %d\n\tHungryTime = %d\n\tThinkCount = %d\n\tThinkTime = %d\n\t\n",
				philosopher->name,
				philosopher->eatCount,
				philosopher->eatTime,
				philosopher->hungryCount,
				philosopher->hungryTime,
				philosopher->thinkCount,
				philosopher->thinkTime );
	}

	/*Free the memory up */
	for(philosopher = head; philosopher != NULL; philosopher = phil_temp) {
		phil_temp = philosopher->next;
		free(philosopher->name);
		free(philosopher->state);
		free(philosopher);
	}

	free(mytime);
	return (0);
}
