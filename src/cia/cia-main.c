/*
 cia-main.c
initial: 2.7.2013 bo
*/

//shared memory stuff
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

char *shm;
#define  SHB_KEY 1122;
int main(int argc,char *argv[]){
//shared memory stuff
  int shmid;
  size_t shmsz =  16 * (8+1) * sizeof(char);
  /*
   * Locate the segment.
   */
  key_t key = SHBus_KEY;
  if ((shmid = shmget(key, shmsz, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  
  /*
    Now we attach the segment to our data space.
  */
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  shm[0]

    while(true){

    }
}
