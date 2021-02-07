#ifndef max_h
#define max_h

int max(int* a, int size)
{
	int i;

	// storing the largest number to a[0]
	for (i = 1; i < size; ++i) {
		if (a[0] < a[i])
			a[0] = a[i];
	}

	return a[0];
}
#endif
