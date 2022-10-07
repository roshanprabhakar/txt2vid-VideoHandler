#include <math.h>
#include <unistd.h>
#include <stdio.h>

#define PIXEL_SIZE 3

typedef unsigned char uchar;

inline double sig(int x) {
	return 1.0 / (1.0 + exp(-1 * x));
}

inline double dsig(int x) {
	return sig(x) * (1 - sig(x));
}

inline uchar transform(int x) {
	// return (uchar) (1020.0 * dsig(3 * (x - 7.0)));
	return 0;
}

int main() {
	
	uchar buf[10];
	
	/*
	for (int i = 0; i < sizeof(buf); i++) {
		*(buf + i) = transform(i);
		usleep(10000);
	}
	*/

	buf[0] = 200;
	buf[1] = 255;
	buf[2] = 255;
	buf[3] = 255;
	buf[4] = 200;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;
	buf[8] = 0;
	buf[9] = 255;

	double mass = 0;
	for (int i = 0; i < sizeof(buf); i++) mass += *(buf + i);

	double com = 0;
	printf("0");
	for (int i = 1; i <= sizeof(buf); i++) {
		double weight = *(buf + i - 1) / mass;
		printf(" + %f * %d", weight, i);
		com += weight * i;
	}	
	printf(" - 1");
	com--;
	printf(" = %f\n", com);

}
