CFLAGS="-Werror -fsanitize=address -fsanitize=undefined -lraylib"
BINARY_NAME="pool_sim"

clang src/*.c $CFLAGS -o $BINARY_NAME

if [[ $? == 0 ]]; then
	echo "Compiled $BINARY_NAME successfully"
else
	echo "Failed to compile $BINARY_NAME"
fi
