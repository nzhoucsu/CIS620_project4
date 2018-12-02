#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include "ldshr.h"

FILE           *fp = NULL;
static double *ldp = NULL;

double reduction(int x, int y, int z);

double *
getload_1_svc(char **srvname, struct svc_req *rqp){
  double loadavg[3] = {-1, -1, -1};
  if (getloadavg(loadavg, 3) != -1)
  {
    ldp = (double *) malloc(sizeof(double*));
    ldp = &loadavg[0];
    return ((double *)ldp);
  }
}

double *
sumqroot_gpu_1_svc(struct gpu_struct *param, struct svc_req *rqp){
	double  *result = malloc(sizeof(double *));
    *result =reduction(param->N, param->mean, param->seed);
  	return result;
}
