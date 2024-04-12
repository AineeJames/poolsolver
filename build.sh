BINARY_NAME="pool_sim"
CFLAGS="-Werror -Wall -fsanitize=address -fsanitize=undefined `pkg-config --cflags raylib`"
LIBS="-lm `pkg-config --libs raylib`"

clang $CFLAGS src/*c -o $BINARY_NAME $LIBS

if [[ $? == 0 ]]; then
	echo "Compiled $BINARY_NAME successfully"
else
	echo "Failed to compile $BINARY_NAME"
fi
