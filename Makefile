CXX ?= g++
DEBUG ?= 1
RES = main.cpp http_conn.cpp
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2
endif

.PHONY: server

server:
	cd src && $(CXX) -o server $(RES) $^ $(CXXFLAGS) -lpthread
	mv ./src/server .
clean:
	rm  -r server
