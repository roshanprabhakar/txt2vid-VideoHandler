#include <stdio.h>

#include "video_player.h"

int main() {
	printf("init chunk reader\n");
	chunk_reader cr;
	chunk* ch = cr.generate_chunk();

	printf("init video player\n");
	cv::Mat f = cr.get_reference_frame();
	video_player vp(f, "client", ch->fps);

	vp.load_chunk(ch);

	delete ch;
	return 0;
}
