intest:
	cd interpreter && \
	flex -o scanner.cpp scanner.l && \
	bison -o parser.cpp parser.y && \
	cd .. && \
	g++ -g ui.cpp base/base.cpp base/color.cpp base/error.cpp base/manager.cpp catalog/catalog.cpp command/command.cpp interpreter/scanner.cpp interpreter/parser.cpp interpreter/interpreter.cpp -o ui 

clean:
	cd interpreter && \
	rm -rf scanner.cpp && \
	rm -rf parser.cpp parser.hpp location.hh position.hh stack.hh && \
	cd .. && \
	rm -rf ui