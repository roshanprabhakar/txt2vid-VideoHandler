#include <stdio.h>
#include <math.h>

#include <chrono>
#include <opencv2/imgproc/imgproc.hpp>

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

chunk::chunk(int rows, int cols): 
	wp(0), 
	frame_size(rows * cols * PIXEL_SIZE), 
	capacity(frame_size * MAX_SECONDS_PER_CHUNK * CV_FPS), num_frames(0), 
	rows(rows), cols(cols) {

	printf("chunk init: cap = %ld\n", capacity);
	printf("expecting frame size: %ld\n", frame_size);

	buffer = new unsigned char[capacity];
	deltas = new unsigned char[capacity];
}

chunk::~chunk() {
	delete [] buffer;
	delete [] deltas;
}

// for some reason can't pass m by ref
void chunk::mark_spot(
		unsigned char* buf,
		int row, int col,
		byte r, byte g, byte b) {

	for (int dr = -1; dr < 1; dr++) {
		for (int dc = -1; dc < 1; dc++) {
		
			if (row + dr < 0 || row + dr >= rows ||
					col + dc < 0 || col + dc >= cols) continue;

			buf[(row + dr) * row_size * PIXEL_SIZE + (col + dc) * PIXEL_SIZE + 0] = b;
			buf[(row + dr) * row_size * PIXEL_SIZE + (col + dc) * PIXEL_SIZE + 1] = g;
			buf[(row + dr) * row_size * PIXEL_SIZE + (col + dc) * PIXEL_SIZE + 2] = r;
		}
	}
}

void chunk::draw_vertical_line(
		unsigned char* buffer,
		int col,
		byte r, byte g, byte b
		) {
	
	for (int row = 0; row < col_size; row++) {
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 0] = b;
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 1] = g;
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 2] = r;
	}
}

void chunk::draw_horizontal_line(
		unsigned char* buffer,
		int row,
		byte r, byte g, byte b
		) {

	for (int col = 0; col < row_size; col++) {
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 0] = b;
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 1] = g;
		buffer[row * row_size * PIXEL_SIZE + col * PIXEL_SIZE + 2] = r;
	}
}

/*
 * Custom routine, for some reason CV_COLORBGR2GREY
 * 9-plexes the image.
 */
void chunk::convert_to_grey(unsigned char* buffer) {
	register unsigned int c1, c2, c3, avg;
	for (size_t i = 0; i < frame_size; i += 3) {

		c1 = (unsigned int) *(buffer + i + 0);
		c2 = (unsigned int) *(buffer + i + 1);
		c3 = (unsigned int) *(buffer + i + 2);
		avg = (c1 + c2 + c3) / 3;

		buffer[i + 0] = avg;
		buffer[i + 1] = avg;
		buffer[i + 2] = avg;
	}
}

inline char transform(int diff) {
	return (char) (510 * (1.0 / (1.0 + exp(-0.3 * diff)) - 0.5));
}

void chunk::isolate_delta(byte* init, byte* cur, byte* res) {
	for (size_t i = 0; i < frame_size; i += PIXEL_SIZE) {
		int diff = *(cur + i) - *(init + i);
		diff = (diff < 0) ? -1 * diff : diff;
		char tdiff = transform(diff);
		*(res + i + 0) = (byte) diff;
		*(res + i + 1) = (byte) diff;
		*(res + i + 2) = (byte) diff;
	}
}

void chunk::ingest_frame(cv::Mat& m) {
	if (wp == capacity) { fprintf(stderr, "chunk full\n"); return; }
	if (!m.isContinuous()) { fprintf(stderr, "frame not continuous\n"); return; }


	// easier to analyze single channeled image
	convert_to_grey(m.data);

	// isolate frame deltas
	if (wp != 0) {
		isolate_delta(buffer + wp - frame_size, m.data, deltas + wp);
	}

	// draw row com for ever row in delta frame
	int row_coms[rows];
	for (int r = 0; r < rows; r++) {
		
		int row_mass = 0;
		for (int c = 0; c < row_size; c++) {
			row_mass += (deltas + wp)[r * row_size * PIXEL_SIZE + c * PIXEL_SIZE];
		}

		double row_com = 0;
		for (int c = 0; c < row_size; c++) {
			byte sig = (deltas + wp)[r * row_size * PIXEL_SIZE + c * PIXEL_SIZE];
			row_com += (double) sig / row_mass * c;
		}

		row_coms[r] = (int) row_com;
	}

	// draw col com for every col in delta frame
	int col_coms[cols];
	for (int c = 0; c < cols; c++) {

		int col_mass = 0;
		for (int r = 0; r < col_size; r++) {
			col_mass += (deltas + wp)[r * row_size * PIXEL_SIZE + c * PIXEL_SIZE];
		}

		double col_com = 0;
		for (int r = 0; r < col_size; r++) {
			byte sig = (deltas + wp)[r * row_size * PIXEL_SIZE + c * PIXEL_SIZE];
			col_com += (double) sig / col_mass * r;
		}

		col_coms[c] = (int) col_com;
	}

	for (int r = 0; r < rows; r++) {
		mark_spot(deltas + wp, r, row_coms[r], (byte) 255, 0, 0);
	}
	for (int c = 0; c < cols; c++) {
		mark_spot(deltas + wp, col_coms[c], c, (byte) 255, 0, 0);
	}


	memcpy((char*) buffer + wp, m.data, frame_size);
	wp += frame_size;
	num_frames++; 

	return;
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
	
	return c;
}





