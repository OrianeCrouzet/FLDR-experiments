all: main.out main.out.opt preprocess.out preprocess.out.opt test histogram.out measure_sizes.out measure_sizes.out.opt

SRC_C = flip.c main.c macros.c vector_int.c readio.c sample.c construct.c
MEASURE_SIZES_SRCS = measure_sizes.c readio.c sizes.c flip.c vector_int.c construct.c
SRC_CPP = main.cpp macros.c readio.c readio.cpp sample.cpp

GSL_FLAGS = $(shell pkg-config --cflags --libs gsl)  # --cflags
CFLAGS = -g -fsanitize=address -O0

main.out: $(SRC_C)
	gcc -pg -lgmp -o main.out $^ $(GSL_FLAGS) 

main.out.opt: $(SRC_C)
	gcc -O2 -Wno-unused-result -lgmp -o main.out.opt $^ $(GSL_FLAGS)

main.out.profile.out: $(SRC_C)
	gcc -pg -lgmp -o main.out.profile.out $^ $(GSL_FLAGS)

measure_sizes.out: $(MEASURE_SIZES_SRCS)
	gcc -pg -lgmp -o measure_sizes.out $^ $(GSL_FLAGS) 

measure_sizes.out.opt: $(MEASURE_SIZES_SRCS)
	gcc -O2 -Wno-unused-result -lgmp -o measure_sizes.out.opt $^ $(GSL_FLAGS)

preprocess.out: flip.c vector_int.c readio.c preprocess.c construct.c
	gcc -lgmp -o preprocess.out $^ $(GSL_FLAGS)

preprocess.out.opt: flip.c vector_int.c readio.c preprocess.c construct.c
	gcc -O2 -Wno-unused-result -lgmp -o preprocess.out.opt $^ $(GSL_FLAGS)

histogram.out: histogram.c flip.c vector_int.c readio.c sample.c construct.c
	gcc -O2 -o histogram.out $^ $(GSL_FLAGS)

SAMPLERS = alias.exact ky.enc alias.integers aldr alias.rust alias.fractions

test: main.out.opt
	for sampler in $(SAMPLERS); do \
		./main.out.opt 1 1000000 $${sampler} d/d.00050.$${sampler}; \
	done

.PHONY: clean
clean:
	rm -rf *.out