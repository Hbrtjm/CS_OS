#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dlfcn.h>

int main (int l_param, char * wparam[]){
  int i;
  int tab[21]={1,2,3,4,5,6,7,8,9,0,0,1,2,3,4,5,6,7,8,9,0};

/*
1) otworz biblioteke
2) przypisz wskaznikom f1 i f2 adresy funkcji z biblioteki sumuj i mediana
3) stworz Makefile kompilujacy biblioteke 'bibl1' ladowana dynamicznie oraz kompilujacy ten program
4) Stosowne pliki powinny znajdowac sie w folderach '.', './bin', './'lib'. Nalezy uzyc: LD_LIBRARY_PATH
5) W Makefile nalezy dodac: test:  xxxxxx
*/
  void *handler = dlopen("libbibl1.so", RTLD_LAZY);
 
  if(!handler)
  {
	  perror("Handler");
	  return 1;
  }

  int (*f2)(int*,int) = dlsym(handler,"sumuj");
  double (*f1)(int*,int) = dlsym(handler, "mediana");
  
  if(!(f1) || !(f2))
  {
	  perror("f1 or f2");
	  return 2;
  }

  for (i=0; i<5; i++) printf("Wynik: %lf, %d\n", f1(tab+2*i, 21-2*i), f2(tab+2*i, 21-2*i));
  dlclose(handler);
  return 0;
}
