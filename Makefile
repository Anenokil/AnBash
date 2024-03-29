CC = gcc -g -O0
MAIN = main
MODULS = colors strarr strarr_iter shelltree shellexec parse
TARGET = r

all: $(TARGET)

doit:
	$(foreach var, $(MODULS), $(CC) -c $(var).c -o $(var).o;)

$(TARGET): $(MAIN).o $(foreach var, $(MODULS), $(var).o)
	$(CC) $(foreach var, $(MODULS), $(var).o) $(MAIN).o -o $(TARGET)

clear:
	rm -rf *.o
cleart:
	rm -rf *.tst 'abc' 'a#c' 'a c' 'a\c' dir
