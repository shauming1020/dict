TESTS = test_common

TEST_DATA = s Tai

CFLAGS = -O0 -Wall -Werror -g

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied

.PHONY: all clean

all: $(GIT_HOOKS) $(TESTS)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

OBJS_LIB = \
    tst.o bloom.o

OBJS := \
    $(OBJS_LIB) \
    test_common.o \

deps := $(OBJS:%.o=.%.o.d)

test_%: test_%.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<


test:  $(TESTS)
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	sudo perf stat --repeat 1000 \
                -e cache-misses,cache-references,instructions,cycles \
                ./test_common --bench CPY $(TEST_DATA)
	sudo perf stat --repeat 1000 \
                -e cache-misses,cache-references,instructions,cycles \
	        ./test_common --bench REF $(TEST_DATA)

bench: $(TESTS)
	@echo "COPY mechanism"
	@for test in $(TESTS); do \
	    echo -n "$$test => "; \
	    ./$$test --bench CPY $(TEST_DATA) | grep "searched prefix "; \
	done
	@echo "REFERENCE mechanism"
	@for test in $(TESTS); do \
	    echo -n "$$test => "; \
	    ./$$test --bench REF $(TEST_DATA) | grep "searched prefix "; \
	done

plot: $(TESTS)
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	sudo perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
                ./test_common --bench CPY $(TEST_DATA) \
		| grep 'ternary_tree, loaded 206849 words'\
		| grep -Eo '[0-9]+\.[0-9]+' > cpy_data.csv
	sudo perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
				./test_common --bench REF $(TEST_DATA)\
		| grep 'ternary_tree, loaded 206849 words'\
		| grep -Eo '[0-9]+\.[0-9]+' > ref_data.csv

experiment: $(TESTS)
	mkdir -p experiment;
	@echo "COPY mechanism"
	./$(TESTS) --bloom CPY "";		
	./$(TESTS) --bloom-wo CPY "";
	@echo "REFERENCE mechanism"
	./$(TESTS) --bloom REF "";
	./$(TESTS) --bloom-wo REF "";
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	sudo perf stat --repeat 500 \
		-e cache-misses,cache-references,instructions,cycles \
		./$(TESTS) --bloom CPY ""
	sudo perf stat --repeat 500 \
		-e cache-misses,cache-references,instructions,cycles \
		./$(TESTS) --bloom REF ""
#	sudo perf stat --repeat 500 \
                -e cache-misses,cache-references,instructions,cycles \
                ./$(TESTS) --bloom-wo CPY ""
#	sudo perf stat --repeat 500 \
                -e cache-misses,cache-references,instructions,cycles \
                ./$(TESTS) --bloom-wo REF ""


perf-search: $(TESTS)
	@for test in $(TESTS); do \
	echo 3 | sudo tee /proc/sys/vm/drop_caches; \
	sudo perf record \
	-e cache-misses \
	./$$test --bench REF $(TEST_DATA) perf report;
	sudo perf stat \
	-e cache-misses,cache-references,instructions,cycles \
	./$$test --bench REF $(TEST_DATA);
	
clean:
	$(RM) $(TESTS) $(OBJS)
	$(RM) $(deps)
	$(RM) bench_cpy.txt bench_ref.txt ref.txt cpy.txt
	$(RM) *.csv

-include $(deps)
