#include<stdio.h>
#include<signal.h>
#include<unistd.h>

void sig_handler(int signo)
{
	if (signo == SIGWINCH)
	    printf("received SIGINT\n");
}

int main(void)
{
	if(signal(SIGWINCH, sig_handler) ==SIG_ERR)
		printf("error");
	// A long long wait so that we can easily issue a signal to this process
	while(1) 
		sleep(1);
	return 0;
}
