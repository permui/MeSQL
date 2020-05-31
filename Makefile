intest:
	cd interpreter && \
	flex -o scanner.cpp scanner.l && \
	bison -o parser.cpp parser.y && \
	g++ -g ../base/base.cpp ../ui.cpp ../base/color.cpp ../base/error.cpp scanner.cpp parser.cpp interpreter.cpp ../command/command.cpp -o ../ui && \
	cd ..

clean:
	cd interpreter && \
	rm -rf scanner.cpp && \
	rm -rf parser.cpp parser.hpp location.hh position.hh stack.hh && \
	cd .. && \
	rm -rf ui