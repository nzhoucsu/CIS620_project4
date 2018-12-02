#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include "ldshr.h"
#include <math.h>

static double *ldp = NULL;

double reduction(int x, int y, int z);
double sqroot(double num);
double local_sum(double num1, double num2);
void map(double (*f)(double), struct node *list);
double reduce(double (*f)(double, double), struct node *list);


double *
getload_1_svc(char **srvname, struct svc_req *rqp){
  double loadavg[3] = {-1, -1, -1};
  if (getloadavg(loadavg, 3) != -1)
  {
    ldp = (double *) malloc(sizeof(double*));
    ldp = &loadavg[0];
    printf("%f\n", loadavg[0]);
    return ((double *)ldp);
  }
  else{
  	printf("get load failed\n");
  	return &(loadavg[0]);
  }
}


double *
sumqroot_gpu_1_svc(struct gpu_struct *param, struct svc_req *rqp){
	double  *result = malloc(sizeof(double *));
    *result =reduction(param->N, param->mean, param->seed);
  	return result;
}


double *
sumqroot_lst_1_svc(struct node *param, struct svc_req *rqp){
  double  *result = (double *)malloc(sizeof(double));
  struct node *local = param;
  struct node *tmp = param;
  // printf("\n received data: ");
  // while(tmp){
  //   printf("%.3f\t", tmp->num);
  //   tmp = tmp->next;
  // }

  // printf("\nsqrt data: ");
  map(sqroot, local);
  // tmp = local;
  // while(tmp){
  //   printf("%.3f\t", tmp->num);
  //   tmp = tmp->next;
  // }

  printf("\nstart reduce ......\n");
  *result = reduce(local_sum, local);
  printf("finish reduce ......\n");
  printf("sum of data: %.3f\n",   *result);
  return result;
}


double sqroot(double num){
  return sqrt(sqrt(num));
}


double local_sum(double num1, double num2){
  return num1 + num2;
}


void map(double (*f)(double), struct node *list){
  struct node *tmp = list;
  while(tmp){
    tmp->num = (*f)(tmp->num);
    tmp = tmp->next;
  }
}


double reduce(double (*f)(double, double), struct node *list){
  printf("\tenter reduce\n");
  struct node *tmp = list;
  double val = 0;
  while(tmp){
    // printf("\ntmp->num = %.3f, val = %.3f\n", tmp->num, val);
    val = (*f)(val, tmp->num);
    // printf("val_sum = %.3f\n", val);
    tmp = tmp->next;
  }
  printf("\treturned val is %.3f\n", val);
  return val;
}