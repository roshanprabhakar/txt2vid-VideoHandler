
# build script

CC="g++"
CC_FLAGS="-std=c++11 -Wno-deprecated-register"
LINK_FLAGS="$(pkg-config --cflags --libs /opt/homebrew/Cellar/opencv/4.5.5_2/lib/pkgconfig/opencv4.pc)"

# path to opencv installation
CV_PATH="/opt/homebrew/opt/opencv/include/opencv4"

# independent classes
blist="chunk_reader video_player cam_reader chunk_harness"


if [ "$1" == "dist" ]; then
	echo "producing distributable binaries"

	# compile
	for i in $blist; do
		echo "$CC -I$CV_PATH $CC_FLAGS -c src/$i.cpp -o bin/obj/$i.o"
		$CC -I$CV_PATH $CC_FLAGS -c src/$i.cpp -o bin/obj/$i.o
	done

	# link
	echo "$CC {CV_LINK_FLAGS} $CC_FLAGS bin/obj/chunk_reader.o bin/obj/chunk_harness.o bin/obj/video_player.o -o bin/dist/exec"
	$CC $LINK_FLAGS $CC_FLAGS bin/obj/chunk_reader.o bin/obj/chunk_harness.o bin/obj/video_player.o -o bin/dist/exec


elif [ "$1" == "dev" ]; then
	echo "producing dev binaries"

	# compile
	echo "$CC src/exp_weighted_sum.cpp -o bin/dist/dev"
	$CC src/exp_weighted_sum.cpp -o bin/dist/dev

else
	echo "specify target"

fi
