#include "mergesort_Fork.h"

void myMerge(int *left, int length_left, int *right, int length_right){

  //initialize the counter of the indices in left and right.
  int i = 0, j = 0;

  
  //initialize temporary memory blocks storing left and right.
  int *temp_left = (int *) malloc(length_left * sizeof(int));
  int *temp_right = (int *) malloc(length_right * sizeof(int));

  int *result = left;

  
  //copy left and right data into temporary memory blocks.
  memcpy(temp_left, left, length_left * sizeof(int));
  memcpy(temp_right, right, length_right * sizeof(int));

  while (i < length_left && j < length_right){
    if (temp_left[i] <= temp_right[j]){
      *result = temp_left[i];
      i++;
    }
    else{
      *result = temp_right[j];
      j++;
    }
    ++result;
  }
  
  //Appending the rest of elements into result array
  if(i < length_left){
    while (i < length_left){
      *result = temp_left[i];
      ++result;
      i++;
    }
  }
  else {
    while (j < length_right){
      *result = temp_right[j];
      ++result;
      j++;
    }
  }
  free(temp_left);
  free(temp_right);
}



void myMergesort(int array[], int length){
  int middle;
  int *left, *right;
  int length_left;
  int status;
  int pid_left = -1;
  int pid_right = -1;

  if (length <= 1){
    return;
  }

  middle = length / 2;
  length_left = length - middle;

  //Set pointers to left and right segments
  left = array;
  right = array + length_left;

  if ((pid_left = fork()) < 0){
    perror("fork() error");
    exit(1);
  }

  if (pid_left == 0){
    myMergesort(left, length_left);
    exit(0);
  }

  else{
    if ((pid_right = fork()) < 0){
      perror("fork() error");
      exit(1);
    }
    if (pid_right == 0) {
      myMergesort(right,middle);
      exit(0);
    }
  }

  //wait until child processes finish.
  waitpid(pid_left, &status, 0);
  waitpid(pid_right, &status, 0);
  myMerge(left, length_left, right, middle);
}


//To print out as required.
void printOut(int array[], int length){
  int i;
  for (i = 0; i < length; i++){
    printf(" %d",array[i]);
  }
  printf("\n");
}



int main(int argc, char *argv[]){
  int length = 0;
  FILE *test_file;
  int data;
  int *shmem_array;
  int shm_id;
  size_t shmem_size;
  int i;
  int *array = NULL;

  test_file = fopen(argv[1], "r");
  if (test_file == NULL){
    printf("Please enter a valid file name\n");
    return 0;
  }


  //int *array = (int *) malloc(length * sizeof(int));

  while (fscanf(test_file, "%d", &data) != EOF){
    ++length;
    array = (int *) realloc(array, length * sizeof(int));
    array[length - 1] = data;
  }

  fclose(test_file);
  printf("%d elements read\n", length);


  shmem_size = length * sizeof(int);


  //Creating shared memory segment.
  if ((shm_id = shmget(IPC_PRIVATE, shmem_size, IPC_CREAT | 0666)) == -1){
    perror("shmget");
    exit(1);
  }


  //Attach to the shared memory segment.
  if ((shmem_array = shmat(shm_id, NULL, 0)) == (int *) -1){
    perror("shmat");
    exit(1);
  }


  //copy the data.
  for (i = 0; i < length; i++){
    shmem_array[i] = array[i];
  }

  printf("Input Numbers:\n");
  printOut(shmem_array, length);
  myMergesort(shmem_array, length);
  printf("Sorted Numbers:\n");
  printOut(shmem_array, length);

  //Detach
  if (shmdt(shmem_array) == -1){
    perror("shmdt");
    exit(1);
  }

  //free(array);

  return 0;
}
