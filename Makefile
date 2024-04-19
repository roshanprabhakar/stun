CC = clang++

INC := $(shell find src -name "*.hpp")
SRC := $(shell find src -name "*.cpp")
OBJ := $(patsubst src/%.cpp, build/%.o, $(SRC))

CPP_FLAGS = -I/opt/homebrew/opt/openssl@3/include -std=c++14
# LD_FLAGS = -L/opt/homebrew/opt/openssl@3/lib

out: $(OBJ)
	# $(CC) -g $^ -o build/out $(LD_FLAGS) -lssl -lcrypto
	$(CC) -g $^ -o build/out

build/%.o: src/%.cpp
	mkdir -p build/ && $(CC) $(CPP_FLAGS) -g -c $< -o $@

clean:
	rm -rf build/
