TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Wall -Wvla -Werror -O0 -g -std=c11
ASAN_FLAGS = -fsanitize=address
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

src2 = testing.c
obj2 = $(src2:.c=.o)
arg = testing
arg1 = shift
arg2 = add_2_numbers
arg3 = add_2_numbers_withfunc
arg4 = printing_h

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) $(ASAN_FLAGS) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $(ASAN_FLAGS) $<

run:
	./$(TARGET) examples/$(arg1)/$(arg1).mi

test:
	echo what are we testing?!

clean:
	rm -f *.o *.obj $(TARGET)
	rm -f *.txt
	rm -f a.out
	rm -f testing

gdb:
	gdb --args ./$(arg) examples/$(arg1)/$(arg1).mi

dump:
	xxd -b -c 4 examples/$(arg1)/$(arg1).mi > binaries/$(arg1).txt
	xxd -b -c 4 examples/$(arg2)/$(arg2).mi > binaries/$(arg2).txt
	xxd -b -c 4 examples/$(arg3)/$(arg3).mi > binaries/$(arg3).txt
	xxd -b -c 4 examples/$(arg4)/$(arg4).mi > binaries/$(arg4).txt

$(arg):$(obj2)
	$(CC) $(ASAN_FLAGS) -o $@ $(obj2)

run1:
	./$(arg) examples/$(arg1)/$(arg1).mi > $(arg1).txt
run2:
	./$(arg) examples/$(arg2)/$(arg2).mi > $(arg2).txt

run3:
	./$(arg) examples/$(arg3)/$(arg3).mi > $(arg3).txt

run4:
	./$(arg) examples/$(arg4)/$(arg4).mi > $(arg4).txt

submit:
	git add .
	git commit -m "commit"
	git push
