clear
clear
rm -rf final
g++ final.cpp -o final -std=c++11 -O2 -Wall -Wextra -Wshadow -fsanitize=undefined -fsanitize=address -D_GLIBCXX_DEBUG
./final < input.txt
