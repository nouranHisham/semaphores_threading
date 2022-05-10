#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include <limits.h>

//global variables
int counter;
int buffer;
const int sizeofbuffer = 5;
sem_t sharedInteger; //semaphore on counter
sem_t sharedBuffer; //semaphore on buffer
sem_t n;
sem_t e;
pthread_t counterThread[7];

//A structure to represent a queue
struct Queue {
    int front, rear, size;
    int capacity;
    int* array;
};

struct Queue* queue;

//thread functions declration
void *counterFunction(void *arg);
void *monitorFunction(void *arg);
void *collectorFunction(void *arg);

//queue functions declration
struct Queue* createQueue(int capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int position(struct Queue* queue);

//main
int main()
{
int i;

//queue for the buffer
queue = createQueue(sizeofbuffer);

//initializing semaphores
sem_init(&sharedInteger,0,1);
sem_init(&sharedBuffer,0,1);
sem_init(&n,0,0);
sem_init(&e,0,sizeofbuffer);

pthread_t collectorThread;
pthread_create(&collectorThread, NULL, &collectorFunction, NULL);

pthread_t monitorThread;
pthread_create(&monitorThread, NULL, &monitorFunction, NULL);

for(i=0; i<7; i++)
{
pthread_create(&counterThread[i], NULL, &counterFunction, NULL);
}

pthread_join(collectorThread, NULL);
pthread_join(monitorThread, NULL); 
for(i=0; i<7; i++)
{
pthread_join(counterThread[i], NULL);
}

return 0;
}

void *counterFunction(void *arg)
{
pthread_t tid;
tid = pthread_self();

int threadNumber;

for(int i=0; i<7; i++){
if(pthread_equal(tid,counterThread[i]))
threadNumber = i+1;
}

while(1){
printf("\n");
printf("Counter thread %d waiting to write...\n", threadNumber);
sem_wait(&sharedInteger);
counter++;
printf("Counter thread %d now adding to counter... counter value = %d\n", threadNumber, counter);
sem_post(&sharedInteger);

int randomnumber = rand() % 4;
sleep(randomnumber);

printf("Counter thread %d received a message!\n", threadNumber);
}
}

void *monitorFunction(void *arg)
{
while(1){
printf("\n");
printf("Monitor thread: waiting to read counter\n");
sem_wait(&sharedInteger);
buffer = counter;
printf("Monitor thread: reading a count value of counter %d\n", counter);
counter = 0;
sem_post(&sharedInteger);
if(sem_trywait(&e) != 0)
{
printf("Monitor thread: buffer full!!!\n");
sem_wait(&e);
}
sem_wait(&sharedBuffer);
enqueue(queue, buffer);
int pos = position(queue);
printf("Monitor thread: writing to buffer at position %d\n", pos);
sem_post(&sharedBuffer);
sem_post(&n);

int randomnumber = rand() % 6;
sleep(randomnumber);
}
}

void *collectorFunction(void *arg)
{
while(1){
printf("\n");
if(sem_trywait(&n) != 0)
{
printf("Collector thread: nothing is in the buffer!!!\n");
sem_wait(&n);
}
sem_wait(&sharedBuffer);
int pos = position(queue);
dequeue(queue);
printf("Collector thread: reading from the buffer at position %d\n", pos);
sem_post(&sharedBuffer);
sem_post(&e);

int randomnumber = rand() % 10;
sleep(randomnumber);
}
}
 
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(int capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
 
// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

int position(struct Queue* queue)
{
int size = queue->size;
return size;
}
