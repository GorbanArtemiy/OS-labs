PARENT = parent
CHILD = child
OUTPUT = output.txt

all: $(PARENT) $(CHILD) $(OUTPUT)

$(PARENT): parent.c
	gcc -Wall -Wextra -pedantic -Werror -g parent.c -o $(PARENT) -lrt

$(CHILD): child.c
	gcc -Wall -Wextra -pedantic -Werror -g child.c -o $(CHILD) -lrt

$(OUTPUT):
	touch $(OUTPUT)

run: all
	./$(PARENT)

clean:
	rm -f $(PARENT) $(CHILD) $(OUTPUT)
