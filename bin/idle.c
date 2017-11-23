#include <stdlib.h>

void foo(){
	int x = 10;
	return;
}
int main(){
	for(int i = 0;i<10;++i){
		printf("Process 8 -> i = %d\n",i);
		if(i==7)
			fork();
	}
	exit(0);
}
