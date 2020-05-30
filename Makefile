intest:
	cd interpreter && \
	flex -o scanner.cpp scanner.l && \
	bison -o parser.cpp parser.y && \
	g++ -g ../test.cpp scanner.cpp parser.cpp interpreter.cpp command.cpp -o ../intest && \
	cd ..

clean:
	cd interpreter && \
	rm -rf scanner.cpp && \
	rm -rf parser.cpp parser.hpp location.hh position.hh stack.hh && \
	cd .. && \
	rm -rf a.out