#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include "ldshr.h"
#include <pthread.h>

#define READLOAD(ldp) {\
  tmp = *ldp;\
}

#define MACHINE_NUM 3
// double tmp;
struct para{
  char *srvname;
  double *loadavg;
};

struct gpu_passing{
  CLIENT *cl;
  struct gpu_struct *p;
  double *dp;
  int flag;
};

void *gpu(struct gpu_passing *p);

static void *pthd_act(void *arg1){
  CLIENT *cl;
  double tmp;
  struct para *ipt_para = (struct para *)arg1;
  char *srvname = ipt_para->srvname;
    if (!(cl = clnt_create(srvname, RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(srvname);
      exit(1);
    }
    else{
      READLOAD(getload_1(&srvname, cl));
      *(ipt_para->loadavg) = tmp;
    }
}

main(argc, argv)
  int             argc;
  char           *argv[];
{
  // Check user input
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

  // Define variables
  CLIENT         *cl[MACHINE_NUM];    /* a client handle */
  char *srvname[] = {
    "bach", 
    "chopin",
    //"davinci",
    // "degas",
    "arthur"
  };
  char *srvrun[2];
  double loadavg[MACHINE_NUM];
  int i, min_idx[2];
  double tmp;
  struct para ipt_para;
  pthread_t p_id[MACHINE_NUM];

  // Get remote server load
  for (i = 0; i < MACHINE_NUM; i++)
  {
    loadavg[i] = -1;
    ipt_para.srvname = srvname[i];
    ipt_para.loadavg = &loadavg[i];
    if(pthread_create(&p_id[i], NULL, pthd_act, (void *)&ipt_para) !=0){
        printf("Create new thread failed! Program terminates!\n");
        exit(0);
    }   
    pthread_join(p_id[i], NULL); 
  }
  for (i = 0; i < MACHINE_NUM; i++)
  {
    printf("%s: %.3f\t", srvname[i], loadavg[i]);
  }
  printf("\n");
  // Find out two servers with minmum workload
  tmp = loadavg[0];
  min_idx[0] = 0;
  for (i=1; i<MACHINE_NUM; i++){
    if(loadavg[i] < tmp){
      tmp = loadavg[i];
      min_idx[0] = i;
    }
  }
  if(min_idx[0] == 0){
    tmp = loadavg[1];
    min_idx[1] = 1;
  }
  else{
    tmp = loadavg[0];
    min_idx[1] = 0;
  }
  for (i=0; i<MACHINE_NUM; i++){
    if(i == min_idx[0]){
      continue;
    }
    if(loadavg[i] < tmp){
      tmp = loadavg[i];
      min_idx[i] = i;
    }
  }  
  srvrun[0] = srvname[min_idx[0]];
  srvrun[1] = srvname[min_idx[1]];
  printf("Execute on %s and %s\n", srvrun[0], srvrun[1]);

  if (!(cl[0] = clnt_create(srvrun[0], RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(srvrun[0]);
      exit(1);
    }
  if (!(cl[1] = clnt_create(srvrun[1], RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(srvrun[1]);
      exit(1);
    }
  // Run gpt
  if(strcmp(argv[1], "-gpu")==0){
    int N = atoi(argv[2]);
    int mean = atoi(argv[3]);
    int seed_1 = atoi(argv[4]);
    int seed_2 = atoi(argv[5]);
    struct gpu_struct g1, g2;
    struct gpu_passing p1, p2;
    g1.N = N-1;
    g1.mean = mean;
    g1.seed = seed_1;
    p1.cl = cl[0];
    p1.p = &g1;
    p1.dp = (double *)malloc(sizeof(double*));
    p1.flag = 1;
    g2.N = N-1;
    g2.mean = mean;
    g2.seed = seed_2;
    p2.cl = cl[1];
    p2.p = &g2;
    p2.dp = (double *)malloc(sizeof(double*));
    p2.flag = 2;
    if(pthread_create(&p_id[0], NULL, gpu, (void *)&p1) !=0){
        printf("Create new thread failed! Program terminates!\n");
        exit(0);
    } 
    
    if(pthread_create(&p_id[1], NULL, gpu, (void *)&p2) !=0){
        printf("Create new thread failed! Program terminates!\n");
        exit(0);
    } 
    pthread_join(p_id[0], NULL);
    pthread_join(p_id[1], NULL);
    printf("%.3f, %.3f, %.3f\n", *(p1.dp), *(p2.dp), *(p1.dp)+*(p2.dp));
  }  
}


void *gpu(struct gpu_passing *p){ 
  printf("%d thread creates\n", p->flag);
  p->dp = sumqroot_gpu_1(p->p, p->cl);
}























