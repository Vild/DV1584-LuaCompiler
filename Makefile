PROJECT := DV1584-Assignment2
SHELL = bash

# == Directories ==

SRC=src/
OBJ=obj/

ASS2 := ./comp
ASS2_SOURCES := $(SRC)main.cpp $(SRC)controlflowgraph.cpp $(OBJ)lua.tab.cpp $(OBJ)lua.yy.cpp $(SRC)convert.cpp
ASS2_HEADERS := $(SRC)ast.hpp $(SRC)token.hpp $(SRC)controlflowgraph.hpp $(SRC)expect.hpp $(OBJ)lua.tab.hpp
ASS2_OBJECTS := $(OBJ)lua.tab.o $(OBJ)lua.yy.o $(OBJ)main.o $(OBJ)controlflowgraph.o $(OBJ)convert.o

TARGETS := $(ASS2)
TESTS := $(foreach testProgram,$(shell find tests -iname "*.lua"), $(OBJ)$(testProgram:.lua=.svg))

CXXFLAGS := -std=c++11 -Wall -Wextra -Wno-unused-parameter

include utils.mk

# == Build rules ==
all: init $(TARGETS)
	@$(call INFO,"::","Building $(PROJECT)... Done!");

init:
	@$(call INFO,"::","Building $(PROJECT)...");

test: all $(TESTS)

$(ASS2): $(ASS2_OBJECTS)
	@$(call BEG,$(BLUE),"  -\> LD","$@ \<-- $^")
	@mkdir -p $(dir $@)
	@$(CXX) $^ -o $@ $(LFLAGS) $(ERRORS)
	@$(call END,$(BLUE),"  -\> LD","$@ \<-- $^")

$(OBJ)tests/%.svg: tests/%.lua $(INT)
	@$(call INFO,"::","Testing $(patsubst tests/%,%.calc,$@)...");
	@mkdir -p $(dir $@);
	@$(call BEG,$(BLUE),"  -\>","Running LUA interpreter...");
	@$(LUA) $< > $(@:.svg=.lua-output) <$(<:.lua=-input.txt) $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Running LUA interpreter...");
	@$(call BEG,$(BLUE),"  -\>","Running $(ASS2) interpreter...");
	@$(ASS2) $< --parse-tree $(@:.svg=.dot) $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Running $(ASS2) interpreter...");
	@$(call BEG,$(BLUE),"  -\>","Compiling test...");
	@as target-raw.s -o target.o $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Compiling test...");
	@$(call BEG,$(BLUE),"  -\>","Linking test...");
	@ld target.o $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Linking test...");
	@$(call BEG,$(BLUE),"  -\>","Running test...");
	@./a.out <$(<:.lua=-input.txt) > $(@:.svg=.int-output) $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Running test...");
	@#$(call BEG,$(BLUE),"  -\>","Generating dot-graph...");
	@#$(DOT) -tsvg -O$@ $(@:.dot=.txt) $(ERRORS);
	@#$(call END,$(BLUE),"  -\>","Generating dot-graph...");
	@$(call BEG,$(BLUE),"  -\>","Checking differance...");
	@$(call END,$(BLUE),"  -\>","Checking differance...");
	@colordiff $(@:.svg=).int-output $(@:.svg=).lua-output -y || true

clean:
	@$(call INFO,"::","Removing generated files...");
	@$(call BEG,$(BLUE),"  -\> RM","$(TARGETS)")
	@$(RM) -rf $(TARGETS)
	@$(call END,$(BLUE),"  -\> RM","$(TARGETS)")
	@$(call BEG,$(BLUE),"  -\> RM","$(OBJ)")
	@$(RM) -rf $(OBJ)
	@$(call END,$(BLUE),"  -\> RM","$(OBJ)")

format: $(ASS2_SOURCES) $(ASS2_HEADERS)
	@$(call INFO,"::","Formatting...");
	@for file in $^; do \
	$(call BEG,$(BLUE),"  -\> clang-format","$$file"); \
	clang-format -i $$file $(ERRROR); \
	$(call END,$(BLUE),"  -\> clang-format","$$file"); \
	done

$(OBJ)location.hh $(OBJ)position.hh $(OBJ)stack.hh: $(OBJ)lua.tab.cpp

.depend: $(ASS2_SOURCES) $(ASS2_HEADERS)
	@$(call INFO,"::","Generating dependencies...");
	@$(call BEG,$(BLUE),"  -\> RM","$@")
	@$(RM) -rf ./$@
	@$(call END,$(BLUE),"  -\> RM","$@")
	@$(call BEG,$(BLUE),"  -\> makedepend","$@ \<-- $(ASS2_SOURCES)")
	@makedepend -- -Isrc -Iobj -- $(ASS2_SOURCES) -f- 2>/dev/null > $@
	@$(call END,$(BLUE),"  -\> makedepend","$@ \<-- $(ASS2_SOURCES)")
include .depend
