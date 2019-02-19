PROJECT := DV1584-Assignment1
SHELL = bash

# == Directories ==

SRC=src/
OBJ=obj/
BIN=bin/

ASS1 := ./int
ASS1_OBJECTS :=$(OBJ)lua.tab.o $(OBJ)lua.yy.o $(OBJ)main.o

TARGETS := $(ASS1)
TESTS := $(OBJ)tests/base.svg
# $(foreach testProgram,$(shell find tests -iname "*.lua"), $(OBJ)$(testProgram:.lua=.svg))

CXXFLAGS := -std=c++17

include utils.mk

# == Build rules ==
all: init $(TARGETS)
	@$(call INFO,"::","Building $(PROJECT)... Done!");

init:
	@$(call INFO,"::","Building $(PROJECT)...");

test: all $(TESTS)

$(ASS1): $(ASS1_OBJECTS)
	@$(call BEG,$(BLUE),"  -\> LD","$@ \<-- $^")
	@mkdir -p $(dir $@)
	@$(CXX) $^ -o $@ $(LFLAGS) $(ERRORS)
	@$(call END,$(BLUE),"  -\> LD","$@ \<-- $^")

$(OBJ)tests/%.svg: tests/%.lua $(INT)
	@$(call INFO,"::","Testing $(patsubst tests/%,%.calc,$@)...");
	@mkdir -p $(dir $@);
	@$(call BEG,$(BLUE),"  -\>","Running LUA interpreter...");
	@$(LUA) $< > $(@:.svg=.lua-output) $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Running LUA interpreter...");
	@$(call BEG,$(BLUE),"  -\>","Running $(ASS1) interpreter...");
	@$(ASS1) $< --parse-tree $(@:.svg=.dot) > $(@:.svg=.int-output) $(ERRORS);
	@$(call END,$(BLUE),"  -\>","Running $(ASS1) interpreter...");
	@#$(call BEG,$(BLUE),"  -\>","Generating dot-graph...");
	@#$(DOT) -tsvg -O$@ $(@:.dot=.txt) $(ERRORS);
	@#$(call END,$(BLUE),"  -\>","Generating dot-graph...");
	@$(call BEG,$(BLUE),"  -\>","Checking differance...");
	@$(DIFF) $(@:.svg=).{lua,int}-output -y $(ERRORSS);
	@$(call END,$(BLUE),"  -\>","Checking differance...");


clean:
	@$(call INFO,"::","Removing generated files...");
	@$(call BEG,$(BLUE),"  -\> RM","$(TARGETS)")
	@$(RM) -rf $(TARGETS)
	@$(call END,$(BLUE),"  -\> RM","$(TARGETS)")
	@$(call BEG,$(BLUE),"  -\> RM","$(OBJ)")
	@$(RM) -rf $(OBJ)
	@$(call END,$(BLUE),"  -\> RM","$(OBJ)")
