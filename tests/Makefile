# Verbosity
export Q =
export V ?= 0
ifeq ($(V), 0)
        MAKEFLAGS += --silent
        Q = @
endif

SRC =	Device.cpp

OUT =	$(SRC:.cpp=.out)

all:	$(OUT)

%.out:	%.cpp
	$(Q)g++ $< -std=c++17 -I../include -I. -I../../claws/include -lvulkan -o $@
	$(A)./$@ && echo [$<] SUCCESS || echo [$<] FAIL

clean:
	$(Q)rm $(OUT)

.PHONY: all clean
