#include <math.h>

const int TABLE_SIZE = int(M_PI * 2000) + 2;
double sins[TABLE_SIZE];

double getSin(double i) {
	int idx = int(i * 1000) % TABLE_SIZE;

	if (idx < 0) {
		idx = TABLE_SIZE + idx;
	}

	return sins[idx];
}

double getCos(double i) {
	return getSin(i + M_PI_2);
}

void calcsins() {
	for (int i = 0; i < TABLE_SIZE; i++) {
		sins[i] = sin(double(i) / 1000);
	}
}
