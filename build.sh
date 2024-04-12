CFLAGS="-Werror -fsanitize=address -fanalyzer -fsanitize=undefined"
BINARY_NAME="pool_sim"

gcc src/*.c $CFLAGS -o $BINARY_NAME

if [[ $? == 0 ]]; then
	echo "Compiled $BINARY_NAME successfully"
else
	echo "Failed to compile $BINARY_NAME"
fi
