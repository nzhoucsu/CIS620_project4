#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include "ldshr.h"
#include <pthread.h>

#define MACHINE_NUM 4
CLIENT *cl[MACHINE_NUM];
int min_idx[2];
char *srvname[] = {
  "bach", 
  //"davinci",
  "degas",
  "arthur",
  "chopin"
};

struct load_passing{
  CLIENT *cl;
  char *srvname;
  double load;
};

struct gpu_passing{
  CLIENT *cl;
  struct gpu_struct *p;
  double *dp;
};

void check_input(int argc, char *argv[]);
void check_load();
void *getload(void *p);
void *gpu(struct gpu_passing *p);
void run_gpu(char *arg[], int idx1, int idx2);


main(argc, argv)
  int             argc;
  char           *argv[];
{
  char *srvrun[2];
  // Check user input.
  check_input(argc, argv);
  // Get workload of remote servers.
  check_load();
  // Run gpt
  if(strcmp(argv[1], "-gpu")==0){
    run_gpu(argv, min_idx[0], min_idx[1]);
  }  
}


void check_load(){
  pthread_t p_id[MACHINE_NUM];
  int i;
  double tmp;
  struct load_passing load_para[MACHINE_NUM];
  // Create communication.
  for (i=0; i<MACHINE_NUM; i++)
  {
    if (!(cl[i] = clnt_create(srvname[i], RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(srvname[i]);
      exit(1);
    }
  }
  // Get load of remote servers.
  for (i=0; i<MACHINE_NUM; i++)
  {
    double a = -1;
    load_para[i].cl = cl[i];
    load_para[i].srvname = srvname[i];
    load_para[i].load = -1;
    if(pthread_create(&p_id[i], NULL, getload, (void *)&load_para[i]) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
    } 
  }
  for (i=0; i<MACHINE_NUM; i++)
  {
    pthread_join(p_id[i], NULL);
  }
  // Pick out two servers with minimum workload.
  tmp = load_para[0].load;
  min_idx[0] = 0;
  for (i=1; i<MACHINE_NUM; i++){
    if(load_para[i].load < tmp){
      tmp = load_para[i].load;
      min_idx[0] = i;
    }
  }
  if(min_idx[0] == 0){
    tmp = load_para[1].load;
    min_idx[1] = 1;
  }
  else{
    tmp = load_para[0].load;
    min_idx[1] = 0;
  }
  for (i=0; i<MACHINE_NUM; i++){
    if(i == min_idx[0]){
      continue;
    }
    if(load_para[i].load < tmp){
      tmp = load_para[i].load;
      min_idx[i] = i;
    }
  }  
  // Print results.
  for (i=0; i<MACHINE_NUM; i++)
  {
    printf("%s %.3f\t", srvname[i], load_para[i].load);
  }
  printf("\n");
  printf("Execute on %s and %s\n", srvname[min_idx[0]], srvname[min_idx[1]]);
}


void *getload(void *p){ 
  struct load_passing *ptr = (struct load_passing *)p;
  ptr->load = *(getload_1(&(ptr->srvname), ptr->cl));
  return;
}


void run_gpu(char *argv[], int idx1, int idx2){
  int N = atoi(argv[2]);
  int mean = atoi(argv[3]);
  int seed_1 = atoi(argv[4]);
  int seed_2 = atoi(argv[5]);
  struct gpu_struct g1, g2;
  struct gpu_passing p1, p2;
  pthread_t p_id1, p_id2;
  g1.N = N-1;
  g1.mean = mean;
  g1.seed = seed_1;
  p1.cl = cl[idx1];
  p1.p = &g1;
  p1.dp = (double *)malloc(sizeof(double*));
  g2.N = N-1;
  g2.mean = mean;
  g2.seed = seed_2;
  p2.cl = cl[idx2];
  p2.p = &g2;
  p2.dp = (double *)malloc(sizeof(double*));
  if(pthread_create(&p_id1, NULL, gpu, (void *)&p1) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
  }     
  if(pthread_create(&p_id2, NULL, gpu, (void *)&p2) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
  } 
  pthread_join(p_id1, NULL);
  pthread_join(p_id2, NULL);
  printf("%s returns %.3f, %s returns %.3f, sum is %.3f\n", srvname[idx1], *(p1.dp), srvname[idx2], *(p2.dp), *(p1.dp)+*(p2.dp));
}


void *gpu(struct gpu_passing *p){ 
  p->dp = sumqroot_gpu_1(p->p, p->cl);
}


void check_input(int argc, char *argv[]){
  if(argc!=3 && argc!=6){
    printf("Wrong number for parameters!\n");
    exit(0);
  }
  if(strcmp(argv[1], "-gpu") != 0 && strcmp(argv[1], "-lst") != 0 ){
    printf("Wrong command!\n");
    exit(0);
  }
  if(strcmp(argv[1], "-gpu")==0){
    if(argc!=6)
      printf("Wrong parameter input\n");
  }
  if(strcmp(argv[1], "-lst")==0){
    if(argc!=3)
      printf("Wrong parameter input\n");
  }
}






















