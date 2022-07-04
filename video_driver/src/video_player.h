#include <string>
#include <opencv2/opencv.hpp>

#include "chunk_reader.h"

using std::string;

class video_player {
	public:
		video_player(cv::Mat& m, string const& name, float spf);
		void load_chunk(chunk* c);
	private:
		string const& label;
		int rows, cols;
		float mspf; // ms per frame

		cv::Mat& frame;
};
