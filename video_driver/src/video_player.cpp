#include <stdio.h>
#include <assert.h>

#include "video_player.h"

video_player::video_player(cv::Mat& ref, string const& name, float fps): 
	rows(ref.rows), cols(ref.cols), label(name), frame(ref), mspf(1 / fps * 1000) {

	cv::namedWindow(name);
	assert(frame.isContinuous());
}

void video_player::load_chunk(chunk* c) {
	assert(frame.isContinuous());
	printf("total frames: %d\n", c->num_frames);
	for (int f = 0;; f++) {
		// printf("%d\n", f);
		// printf("frame: %d; size: %ld\n", f, c->frame_size);
		memcpy(frame.data, c->get_delta_frame(f % c->num_frames), c->frame_size);
		// printf("copy complete\n");
		cv::imshow(label, frame);
		// printf("show complete\n");
		cv::waitKey(mspf);
		// printf("wait complete\n");
		if (f == c->num_frames) f = 0;
	}
}
