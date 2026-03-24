all: main.out main.out.opt preprocess.out preprocess.out.opt test

SRC_C = flip.c main.c macros.c vector_int.c readio.c sample.c utils.c
SRC_CPP = main.cpp macros.c readio.c readio.cpp sample.cpp

GSL_FLAGS = $(shell pkg-config --libs gsl)  # --cflags
CFLAGS = -g -fsanitize=address -O0

main.out: $(SRC_C)
	gcc -pg -o main.out $^ $(GSL_FLAGS) 

main.out.opt: $(SRC_C)
	gcc -O3 -Wno-unused-result -o main.out.opt $^ $(GSL_FLAGS)

main.out.profile.out: $(SRC_C)
	gcc -pg -o main.out.profile.out $^ $(GSL_FLAGS)

preprocess.out: vector_int.c readio.c preprocess.c utils.c
	gcc -o preprocess.out $^ $(GSL_FLAGS)

preprocess.out.opt: vector_int.c readio.c preprocess.c utils.c
	gcc -O3 -Wno-unused-result -o preprocess.out.opt $^ $(GSL_FLAGS)

SAMPLERS = alias.exact interval ky.enc rej.binary rej.enc rej.matc rej.table rej.uniform alias.integers

test: main.out.opt
	for sampler in $(SAMPLERS); do \
		./main.out.opt 1 1000000 $${sampler} d/d.00050.$${sampler}; \
	done

.PHONY: clean
clean:
	rm -rf *.out