#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int signal_number) {
    if(signal_number == 2)
    {
        char decision;
        printf("Interrupt SIGINT recieved\n");
        printf("Do you really want to quit (y|n)?\n");
        scanf(" %c",&decision);
        if(decision == 'y')
        {
            exit(0);
        }
    }
}

int main() {
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	while (1);
}