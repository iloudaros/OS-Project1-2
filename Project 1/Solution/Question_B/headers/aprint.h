#ifndef aprint_h
#define aprint_h


int aprint(int* a, int size)
{
  int i;
  printf("{");


  for (i = 0; i < size; ++i)
  {
   printf("%i",a[i]);
   if(i<size-1)printf(",");
  }

  printf("}");

  return a[0];
}


#endif
