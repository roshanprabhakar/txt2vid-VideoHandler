/**
 * Utility file for handling driving video. Each chunk contains a 
 * buffer of loop-playable video data.
 */

#ifndef _CHUNK_READER_
#define _CHUNK_READER_

#include <string.h>
#include <assert.h>

// #include <opencv2/core/mat.hpp>
// #include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define CV_FPS 30 // replace for different systems
#define MAX_SECONDS_PER_CHUNK 4
#define PIXEL_SIZE 3

typedef unsigned char byte;
 
/*
 * Special type to handle a usable driver video.
 */
struct chunk {
	unsigned char* buffer;
	unsigned char* deltas;

	int rows; // in pixels NOT channels
	int cols; // in pixels NOT channels
#define row_size cols
#define col_size rows
	size_t frame_size;
	size_t capacity; // capacity in bytes
	unsigned int num_frames;
	float fps; // record rate
	size_t wp; 	// write pointer
	
	chunk(int rows, int cols);
	~chunk();
	void ingest_frame(cv::Mat& frame);
	inline void* get_frame(unsigned int f) {
		return (unsigned char*) buffer + frame_size * f;
	}
	inline unsigned char* get_delta_frame(unsigned int f) {
		return (unsigned char*) deltas + frame_size * f;
	}
	int row_com(int row, byte* frame);

	void convert_to_grey(unsigned char* buf);
	void mark_spot(unsigned char* buf, int row, int col, byte r, byte g, byte b);
	void draw_vertical_line(unsigned char* buf, int col, byte r, byte g, byte b);
	void draw_horizontal_line(unsigned char* buf, int row, byte r, byte g, byte b);
	void isolate_delta(byte* init, byte* cur, byte* res);

};

class chunk_reader {
	public:
		chunk_reader();
		chunk* generate_chunk();
		cv::Mat get_reference_frame();
	private:
		void render_chunk(chunk* c);

		int rows, cols;
		cv::Mat cap_frame;
		cv::VideoCapture cap;
};

#endif // _CHUNK_READER_
