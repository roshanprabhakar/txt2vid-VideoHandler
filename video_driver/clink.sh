
# build script

CC="g++"
CC_FLAGS="-std=c++11"
LINK_FLAGS="$(pkg-config --cflags --libs /opt/homebrew/Cellar/opencv/4.5.5_2/lib/pkgconfig/opencv4.pc)"

# path to opencv installation
CV_PATH="/opt/homebrew/opt/opencv/include/opencv4"

# independent classes
blist="chunk_reader video_player cam_reader chunk_harness"

# compile
for i in $blist; do
	echo "$CC -I$CV_PATH $CC_FLAGS -c src/$i.cpp -o bin/obj/$i.o"
	$CC -I$CV_PATH $CC_FLAGS -c src/$i.cpp -o bin/obj/$i.o
done

# link
 $CC $LINK_FLAGS $CC_FLAGS bin/obj/chunk_reader.o bin/obj/chunk_harness.o bin/obj/video_player.o -o bin/dist/exec


