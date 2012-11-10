#include "client.h"
#include "config.h"

#include<stdlib.h>
#include<time.h>
ClientConfigType clientConfig;
extern const gsl_rng_type *T;
extern gsl_rng *r;



int main(){
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	read_client_config("config/client.ini",clientConfig);
	ClientManager *clientMaster = new ClientManager(&clientConfig);
	srandom(time(NULL));
	clientMaster->Run();
	return 0;
}
