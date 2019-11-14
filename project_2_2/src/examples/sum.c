#include<stdio.h>
#include"userprog/syscall.h"
int pibo1(int n);
int sumfour1(int a, int b, int c, int d);
int pibo1(int n){
	int previous=-1;
	int result=1;
	int now=0;
	int i=0;
	for(i=0;i<=n;++i){
		now = result+previous;
		previous = result;
		result = now;
	}
	return result;
}

int sumfour1(int a, int b, int c, int d){
	int result = 0;
	result = a+b+c+d;
	return result;
}

int main(int argc, char* argv[]){

	int a[4]={0};
	int i=0;

	for(i=0;i<argc;i++)
		a[i] = (int)argv[i+1][0];

	printf("%d %d\n",pibo1(a[0]),sumfour1(a[0],a[1],a[2],a[3]));

	return 1;
}
