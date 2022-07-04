#include <stdio.h>
#include <assert.h>

#include "video_player.h"

video_player::video_player(cv::Mat& ref, string const& name, float fps): 
	rows(ref.rows), cols(ref.cols), label(name), frame(ref), mspf(1 / fps * 1000) {

	cv::namedWindow(name);
	assert(frame.isContinuous());

	// printf("// video player config //\n");
	// printf("num pixels: %ld; pixel size: %ld\n", frame.total(), frame.elemSize());
}

void video_player::load_chunk(chunk* c) {
	assert(frame.isContinuous());
	printf("vp: chunk_start: %d, chunk_end: %d\n",
			c->chunk_start,
			c->chunk_end
	);
	printf("total frames: %d\n", c->num_frames);
	for (int f = c->chunk_start;; f++) {
		printf("%d\n", f);
		// printf("frame: %d; size: %ld\n", f, c->frame_size);
		memcpy(frame.data, c->get_frame(f % c->num_frames), c->frame_size);
		// printf("copy complete\n");
		cv::imshow(label, frame);
		// printf("show complete\n");
		cv::waitKey(mspf);
		// printf("wait complete\n");
		if (f == c->chunk_end) f = c->chunk_start;
	}
}
