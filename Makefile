CXX := g++
LEX := flex
YACC := bison
CFLAGS := -MMD
EFILE := ui
OBJS := base/base.o base/color.o base/error.o base/manager.o base/timer.o \
       buffer/buffer.o catalog/catalog.o command/command.o index/index.o \
	   interpreter/interpreter.o interpreter/parser.o interpreter/scanner.o $(EFILE).o
DEPS := $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .cpp

debug: OPT := -g
debug: $(EFILE)
release: OPT := -O2
release: $(EFILE)

$(OBJS): interpreter/interpreter_prepare

$(EFILE): $(OBJS)
	g++ $(CFLAGS) $(OPT) -o $@ $^

-include $(DEPS)

interpreter/parser.cpp interpreter/scanner.cpp: interpreter/interpreter_prepare

interpreter/interpreter_prepare: interpreter/parser.y interpreter/scanner.l
	$(LEX) -o $(<D)/scanner.cpp $(<D)/scanner.l
	$(YACC) -o $(<D)/parser.cpp $(<D)/parser.y
	touch interpreter/interpreter_prepare

%.o: %.cpp
	$(CXX) $(CFLAGS) $(OPT) -c -o $@ $<

clean:
	rm -rf $(OBJS)
	rm -rf $(DEPS)
	rm -rf $(EFILE)
	rm -rf interpreter/scanner.cpp interpreter/parser.cpp \
           interpreter/parser.hpp interpreter/*.hh \
		   interpreter/interpreter_prepare