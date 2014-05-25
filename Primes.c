#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

unsigned long N;
unsigned int ind;
int* primes;
unsigned int number_threads;
//used to count the number of created threads
pthread_mutex_t mutex1 =PTHREAD_MUTEX_INITIALIZER;
//used to introduce the prime numbers into int* primes
pthread_mutex_t mutex2 =PTHREAD_MUTEX_INITIALIZER;
sem_t semaforo;


//------------------------------------------------------------------------------------------
// Type of the circular queue elements
typedef unsigned long QueueElem;

//------------------------------------------------------------------------------------------
// Struct for representing a "circular queue"
// Space for the queue elements will be allocated dinamically by queue_init()
typedef struct
{
	QueueElem *v;// pointer to the queue buffer
	unsigned int capacity; // queue capacity
	unsigned int first;// head of the queue
	unsigned int last;// tail of the queue
	sem_t empty;// semaphores and mutex for implementing the
	sem_t full;// producer-consumer paradigm
	pthread_mutex_t mutex;
} CircularQueue;


//------------------------------------------------------------------------------------------
// Allocates space for circular queue 'q' having 'capacity' number of elements
// Initializes semaphores & mutex needed to implement the producer-consumer paradigm
// Initializes indexes of the head and tail of the queue
// TO DO BY STUDENTS: ADD ERROR TESTS TO THE CALLS & RETURN a value INDICATING (UN)SUCESS
void queue_init(CircularQueue **q, unsigned int capacity) // TO DO: change return value
{
	*q = (CircularQueue *) malloc(sizeof(CircularQueue));
	sem_init(&((*q)->empty), 0, capacity);
	sem_init(&((*q)->full), 0, 0);
	pthread_mutex_init(&((*q)->mutex), NULL);
	(*q)->v = (QueueElem *) malloc(capacity * sizeof(QueueElem));
	(*q)->capacity = capacity;
	(*q)->first = 0;
	(*q)->last = 0;
}


//------------------------------------------------------------------------------------------
// Inserts 'value' at the tail of queue 'q'
void queue_put(CircularQueue *q, QueueElem value){
	//verifica se a circularQueue esta cheia
	sem_wait(&q->empty);
	pthread_mutex_lock(&q->mutex);
	q->v[q->last]=value;
	q->last++;
	if(q->last==10){
		q->last=0;
	}
	pthread_mutex_unlock(&q->mutex);
	sem_post(&q->full);
}


//------------------------------------------------------------------------------------------
// Removes element at the head of queue 'q' and returns its 'value'
QueueElem queue_get(CircularQueue *q){
	sem_wait(&q->full);
	pthread_mutex_lock(&q->mutex);
	QueueElem qtemp = q->v[q->first];
	q->first++;
	if(q->first==10){
		q->first=0;
	}
	pthread_mutex_unlock(&q->mutex);
	sem_post(&q->empty);
	return qtemp;
}


//------------------------------------------------------------------------------------------
// Frees space allocated for the queue elements and auxiliary management data
// Must be called when the queue is no more needed
void queue_destroy(CircularQueue *q){
	free(q->v);
	free(q);
}


void* Filter(void* q){
	QueueElem primo= queue_get(q);
	QueueElem temp= queue_get(q);
	//printf(" ler %d \n",temp);
	if(primo>ceil(sqrt(N))){
		while(temp!=0){
			// printf(" a ler finais : %d \n",temp);
			if(temp!=0){
				pthread_mutex_lock(&mutex2);
				primes[ind]=temp;
				ind++;
				pthread_mutex_unlock(&mutex2);
			}
			temp=queue_get(q);
		}
		pthread_mutex_lock(&mutex2);
		primes[ind]=primo;
		ind++;
		pthread_mutex_unlock(&mutex2);
		
		queue_destroy(q);
		sem_post(&semaforo);
	}
	else{
		CircularQueue* q2;
		queue_init(&q2,10);
		pthread_t ThreadPrimo;
		pthread_create(&ThreadPrimo,NULL,Filter,q2);

		// printf("thread created \n");
		pthread_mutex_lock(&mutex1);
		number_threads++;
		pthread_mutex_unlock(&mutex1);

		while(temp!=0){
			if(temp%primo!=0){
				queue_put(q2,temp);
				// printf(" por %d \n",temp);
			}	

			temp=queue_get(q);
			// printf(" ler %d \n",temp);

		}
		queue_put(q2,0);
		queue_destroy(q);

		pthread_mutex_lock(&mutex2);
		primes[ind]=primo;
		ind++;
		pthread_mutex_unlock(&mutex2);
	}

	pthread_mutex_lock(&mutex1);
	number_threads--;
	pthread_mutex_unlock(&mutex1);

	return NULL;
}


void * Initialize(void* q){
	CircularQueue* q2;
	queue_init(&q2,10);
	pthread_t ThreadPrimo;
	pthread_create(&ThreadPrimo,NULL,Filter,q2);
	
	pthread_mutex_lock(&mutex1);
	number_threads++;
	pthread_mutex_unlock(&mutex1);

	int i;
	for(i=3;i<=N;i+=2){
		queue_put(q2,i);
	}

	queue_put(q2,0);

	pthread_mutex_lock(&mutex2);
	primes[ind]=2;
	ind++;
	pthread_mutex_unlock(&mutex2);
	
	pthread_mutex_lock(&mutex1);
	number_threads--;
	pthread_mutex_unlock(&mutex1);

	return NULL;
}


int compare(const void* a, const void* b){
   return (*(int*)a - *(int*)b);
}


int main(int argc, char *argv[]){
	printf("Starting Program")
	//testar sem usar o string to long
	N = strtol(argv[1], NULL, 0);
	//count the number of threads to assure that the program doesn't finish before all the thread end.
	number_threads=0;
	//indice do array de primos
	ind=0;

	sem_init(&semaforo, 0, 0);
	//allocating memory to primes
	primes = (int*) malloc(ceil(1.2*N/log(N)*sizeof(int)));

	pthread_t mainThread;
	pthread_create(&mainThread,NULL,Initialize,NULL);
	number_threads++;
	sem_wait(&semaforo);

	//prints the prime numbers to teh console
	qsort(primes,ind,sizeof(int),compare);
	int n_primes=0;
	for(n_primes;n_primes<ind;n_primes++){
		printf("primo : %d \n",primes[n_primes]);
	}

	return 0;
}
