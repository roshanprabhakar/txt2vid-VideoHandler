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
#define MAX_SECONDS_PER_CHUNK 20
#define PIXEL_SIZE 3

/*
 * Special type to handle a usable driver video.
 */
struct chunk {
	char* buffer;
	signed int rows;
	signed int cols;
	size_t frame_size;
	size_t capacity; // capacity in bytes
	unsigned int num_frames;
	float fps; // record rate
	size_t wp; 	// write pointer
	
	int chunk_start;
	int chunk_end;
	
	chunk(int rows, int cols);
	~chunk();
	void ingest_frame(cv::Mat& frame);
	inline void* get_frame(unsigned int f) {
		return (void*) ((char*) buffer + frame_size * f);
	}
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
