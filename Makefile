generate:
	clang gen_constants.cpp -o a.out
	./a.out > servo_constants.h
	rm a.out
