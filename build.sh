BINARY_NAME="pool_sim"
CFLAGS="-Werror -Wall -fsanitize=address -fsanitize=undefined $(pkg-config --cflags raylib)"
LIBS="-lm $(pkg-config --libs raylib)"

clang $CFLAGS src/*c -o $BINARY_NAME $LIBS

if [[ $? == 0 ]]; then
	echo "Compiled $BINARY_NAME successfully!"
	if [[ "$1" = "run" ]]; then
		./$BINARY_NAME
	elif [[ "$1" = "wasm" ]]; then
		echo "Compiling wasm build"
		emcc -o html_build/pool.html src/*.c -Os -Wall html_build/libraylib.a -Ihtmlbuild/ -I/usr/include/ -Lhtmlbuild/ -s ASYNCIFY -s USE_GLFW=3 -DPLATFORM_WEB --preload-file assets $(pkg-config --cflags raylib)
	fi
else
	echo "Failed to compile $BINARY_NAME!"
fi
