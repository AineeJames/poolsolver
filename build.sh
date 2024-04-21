BINARY_NAME="pool_sim"

CFLAGS="-O3 -Werror -Wall -fsanitize=address -fsanitize=undefined $(pkg-config --cflags raylib)"
#CFLAGS="-O3 -Werror -Wall $(pkg-config --cflags raylib)"
LIBS="-lm $(pkg-config --libs raylib)"

if [[ "$1" = "prod" ]]; then
	echo "Building for production!"
	CFLAGS="-D DONT_DRAW_PATHS $CFLAGS"
	echo $CFLAGS
	shift
fi

clang $CFLAGS src/*c -o $BINARY_NAME $LIBS

if [[ $? == 0 ]]; then
	echo "Compiled $BINARY_NAME successfully!"
	if [[ "$1" = "run" ]]; then
		shift
		./$BINARY_NAME $@
	elif [[ "$1" = "wasm" ]]; then
		echo "Compiling wasm build"
		emcc -o html_build/pool.html src/*.c -Os -g3 -Wall html_build/libraylib.a -Ihtmlbuild/ -I/usr/include/ -Lhtmlbuild/ -s ASYNCIFY -s USE_GLFW=3 -DPLATFORM_WEB --preload-file assets $(pkg-config --cflags raylib)
	fi
else
	echo "Failed to compile $BINARY_NAME!"
fi
