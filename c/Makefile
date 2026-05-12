all: main.out main.out.opt preprocess.out preprocess.out.opt test histogram.out

SRC_C = flip.c main.c macros.c vector_int.c readio.c sample.c alias_rust.c alias_fractions.c utils.c
SRC_CPP = main.cpp macros.c readio.c readio.cpp sample.cpp

GSL_FLAGS = $(shell pkg-config --cflags --libs gsl)  # --cflags
CFLAGS = -g -fsanitize=address -O0

main.out: $(SRC_C)
	gcc -pg -lgmp -o main.out $^ $(GSL_FLAGS) 

main.out.opt: $(SRC_C)
	gcc -O2 -Wno-unused-result -lgmp -o main.out.opt $^ $(GSL_FLAGS)

main.out.profile.out: $(SRC_C)
	gcc -pg -lgmp -o main.out.profile.out $^ $(GSL_FLAGS)

preprocess.out: flip.c vector_int.c readio.c preprocess.c alias_rust.c alias_fractions.c utils.c
	gcc -lgmp -o preprocess.out $^ $(GSL_FLAGS)

preprocess.out.opt: flip.c vector_int.c readio.c preprocess.c alias_rust.c alias_fractions.c utils.c
	gcc -O2 -Wno-unused-result -lgmp -o preprocess.out.opt $^ $(GSL_FLAGS)

histogram.out: histogram.c flip.c vector_int.c readio.c alias_rust.c alias_fractions.c utils.c sample.c
	gcc -O2 -o histogram.out $^ $(GSL_FLAGS)

SAMPLERS = alias.exact interval ky.enc rej.binary rej.enc rej.matc rej.table rej.uniform alias.integers aldr alias.rust alias.fractions

test: main.out.opt
	for sampler in $(SAMPLERS); do \
		./main.out.opt 1 1000000 $${sampler} d/d.00050.$${sampler}; \
	done

.PHONY: clean
clean:
	rm -rf *.out