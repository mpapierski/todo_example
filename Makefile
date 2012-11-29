all:
	clang++ -stdlib=libc++ -std=c++11 -lweb -lsql -lsqlite3 src/main.cpp