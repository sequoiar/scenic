// test with python module ctype to call c functions from python code

#include <stdlib.h>

struct toto{
	char* name;
};

struct toto* titi;

void test( void ){
	titi = (struct toto*)malloc(sizeof(struct toto));
	titi->name = "tata";
	printf("test successfull: %s\n" , titi->name);
}
