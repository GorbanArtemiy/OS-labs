PARENT = parent
CHILD = child
OUTPUT = 1.txt

all: $(PARENT) $(CHILD) $(OUTPUT)

$(PARENT): parent.c
	gcc -Wall -Wextra -pedantic -Werror -g parent.c -o $(PARENT)

$(CHILD): child.c
	gcc -Wall -Wextra -pedantic -Werror -g child.c -o $(CHILD)

$(OUTPUT):
	touch $(OUTPUT)

run: all
	./$(PARENT)

clean:
	rm -f $(PARENT) $(CHILD) $(OUTPUT)
