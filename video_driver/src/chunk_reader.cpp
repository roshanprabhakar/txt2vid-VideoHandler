#include <stdio.h>
#include <math.h>

#include <chrono>

#include "chunk_reader.h"

///////////// UTILS /////////////

using TimeStamp = std::chrono::high_resolution_clock::time_point;

inline TimeStamp currentTime() {
	return std::chrono::high_resolution_clock::now();
}

float getTimeDifference(TimeStamp end, TimeStamp start) {
	std::chrono::duration<float> seconds = end - start;
	float delta = seconds.count();
	return delta;
}

/* default compare function */
/* ISSUES
 * - still needs a way to capture how fast video is changing
 *   	simply taking deltas of adjacent frame not enough, as 
 *   	deltas are not necessarily parallel in image space.
 */

double compare1(void* f1, void* f2, size_t frame_size) {
	double out = 0;
	for (size_t i = 0; i < frame_size; i++) {
		out += abs(((char*) f1) [i] - ((char*) f2) [i]);
	}
	return out;
}

///////////// CHUNK /////////////

chunk::chunk(int rows, int cols): wp(0), frame_size(rows * cols * PIXEL_SIZE),
capacity(frame_size * MAX_SECONDS_PER_CHUNK * CV_FPS), num_frames(0) {
	printf("chunk init: cap = %ld\n", capacity);
	printf("expecting frame size: %ld\n", frame_size);

	buffer = new char[capacity];
}

chunk::~chunk() {
	delete [] buffer;
}

void chunk::ingest_frame(cv::Mat& m) {
	if (wp == capacity) { fprintf(stderr, "chunk full\n"); return; }
	if (!m.isContinuous()) { fprintf(stderr, "frame not continuous\n"); return; }

 	// process frame
	// printf("num pixels: %ld, pixel size: %ld;\t", m.total(), m.elemSize());
	// printf("expecting num pixels: %ld\n", frame_size / PIXEL_SIZE);

	size_t n = m.total() * m.elemSize();
	memcpy((char*) buffer + wp, m.data, n);
	wp += n;

	num_frames++; 
}

////////// CHUNK_READER //////////

chunk_reader::chunk_reader(): cap(0) {
	if (!cap.isOpened()) {
		fprintf(stderr, "reader cannot open camera!\n");
		return;
	}
	cap.read(cap_frame);
	rows = cap_frame.rows;
	cols = cap_frame.cols;
}

cv::Mat chunk_reader::get_reference_frame() {
	assert(cap.isOpened());
	cap.read(cap_frame);
	return cap_frame;
}

/*
 * Record for bounded time.
 *
 * After recording, note all positions x where x - init frame
 * is small, as well as where x - final frame is small. splice
 * buffer at the smallest valued x.
 *
 * returned chunk must be freed.
 */
chunk* chunk_reader::generate_chunk() {
	assert(cap.isOpened());
	chunk* c = new chunk(rows, cols);

	// read in data
	TimeStamp cur = currentTime();

	for (int f = 0; f < CV_FPS * MAX_SECONDS_PER_CHUNK; f++) {
		cap.read(cap_frame);
		cv::waitKey(1);
		c->ingest_frame(cap_frame);
	}

	TimeStamp fin = currentTime();
	c->fps = (float) c->num_frames / getTimeDifference(fin, cur); 

	printf("slicing...\n");

	// slice loopable frames
	int slice_start;
	int slice_end = CV_FPS * MAX_SECONDS_PER_CHUNK - 1;
	double slice_diff = DBL_MAX;
	double diff;

	/* compare every frame with every other frame 
	 * too expensive */
	/*
	for (int start = 0; start < c->num_frames - 1; start++) {
		printf("%f\n", (double) start / c->num_frames);
		for (int end = start + 1; end < c->num_frames; end++) {
			diff = compare1(
					c->get_frame(start), 
					c->get_frame(end), 
					c->frame_size
			);
			if (diff < slice_diff) {
				slice_diff = diff;
				slice_start = start;
				slice_end = end;
			}
		}
	}
	*/

	// otherwise closest is simply second last frame
	for (int f = slice_end; f >= 0; f--) {
		diff = compare1(
				c->get_frame(f),
				c->get_frame(slice_end),
				c->frame_size
		);
		if (diff < slice_diff && f < slice_end - CV_FPS * 4 /*sec*/) {
			slice_diff = diff;
			slice_start = f;
		}
	}
	
	printf("!!!!!!! start: %d, end: %d\n", slice_start, slice_end);
	c->chunk_start = slice_start;
	c->chunk_end = slice_end;

	printf("!!! start: %d, end: %d\n", c->chunk_start, c->chunk_end);

	return c;
}





