# Compiler stuff
CC ?= gcc
CXX ?= g++
FLEX ?= flex
BISON ?= bison
AR ?= ar
DOT ?= dot
DIFF ?= diff
LUA ?= lua
CFLAGS := $(CFLAGS) -I$(SRC) -I$(OBJ) -g
CXXFLAGS := $(CXXFLAGS) $(CFLAGS)
LFLAGS :=
ARFLAGS := rcs

# == Fun color output ==
ifdef NO_COLOR
OFF =
BOLD =
BLUE =
GREEN =
RED =
YELLOW =
else
OFF = \033[1;0m
BOLD = \033[1;1m
BLUE = $(BOLD)\033[1;34m
GREEN = $(BOLD)\033[1;32m
RED = $(BOLD)\033[1;31m
YELLOW = $(BOLD)\033[1;33m
override CFLAGS += -fdiagnostics-color=always
override CXXFLAGS += -fdiagnostics-color=always
endif

BEG =	echo -e -n "$(1)$(2)$(OFF) $(BOLD)$(3)...$(OFF)" ; echo -n > .build-errors
END =	if [[ -s .build-errors ]] ; then \
		if cut -d':' -f4 .build-errors | grep -q error || cut -d':' -f3 .build-errors | grep -q error || grep -qv warning .build-errors; then \
			echo -e -n "\r$(RED)$(2)$(OFF) $(BOLD)$(3)   $(OFF)\n"; \
			cat .build-errors; \
			rm .build-errors || true; \
			exit 1; \
		else \
			echo -e -n "\r$(YELLOW)$(2)$(OFF) $(BOLD)$(3)   $(OFF)\n"; \
			cat .build-errors; \
			rm .build-errors || true; \
		fi \
	else \
		echo -e -n "\r$(1)$(2)$(OFF) $(BOLD)$(3)$(OFF)\033[K\n"; \
	fi

INFO = echo -e -n "$(GREEN)$(1) $(2)$(OFF)\n"

ERRORS = 2>>.build-errors || true
ERRORSS = >>.build-errors || true

.PHONY: all clean init test

# Prevents Make from removing intermediary files on failure
.SECONDARY:
# Disable built-in rules
.SUFFIXES:

$(OBJ)%.tab.cpp $(OBJ)%.tab.hpp $(OBJ)%.dot: $(SRC)%.yy
	@$(call BEG,$(BLUE),"  -\> BISON","$@ \<-- $<")
	@mkdir -p $(dir $@)
	@$(BISON) -y --report=state --graph --warning=all --report=all --feature=all -d -o $@ $^ $(ERRORS)
	@$(call END,$(BLUE),"  -\> BISON","$@ \<-- $<")

$(OBJ)%.yy.cpp: $(SRC)%.ll
	@$(call BEG,$(BLUE),"  -\> FLEX","$@ \<-- $<")
	@mkdir -p $(dir $@)
	@$(FLEX) -o $@ $^ $(ERRORS)
	@$(call END,$(BLUE),"  -\> FLEX","$@ \<-- $<")

$(OBJ)%.o: $(SRC)%.cpp
	@$(call BEG,$(BLUE),"  -\> CXX","$@ \<-- $<")
	@mkdir -p $(dir $@)
	@$(CXX) -c $< -o $@ $(CXXFLAGS) $(ERRORS)
	@$(call END,$(BLUE),"  -\> CXX","$@ \<-- $<")

$(OBJ)%.o: $(OBJ)%.cpp
	@$(call BEG,$(BLUE),"  -\> CXX","$@ \<-- $<")
	@mkdir -p $(dir $@)
	@$(CXX) -c $< -o $@ $(CXXFLAGS) $(ERRORS)
	@$(call END,$(BLUE),"  -\> CXX","$@ \<-- $<")
