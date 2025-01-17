#include <stdio.h>

void loop_print_ints(int *input_array, int array_size);
void loop_print_string(char *input_array, int array_size);

int main()
{
    // let's start with a simple literal int array - the compiler allocates exactly the right amount of memory
    // because this array was defined as a literal, it is stored in the read-only section of the memory
    int digits[] = {100, 101, 102, 103};

    // CONCEPT: show "decay" of digits var
    // TODO

    loop_print_ints(digits, 4);

    // CONCEPT: char arrays (there are no strings!)

    return 0;
}

void loop_print_ints(int *input_array, int array_size)
{
    for (int i = 0; i < array_size; i++)
    {
        // CONCEPT: offset addressing
        // TODO

        // CONCEPT: array index addressing
        // TODO

        // CONCEPT: pointer ARGUMENT increment addressing
        // TODO
        //? why is this ok? does it have any side effects outside the func? (hint: in C, args are passed by _____?)
    }

    // CONCEPT: create local pointer addressing
    // CONCEPT: 1-line pointer increment and use (only because it's common in the wild)
    // TODO
    //? when is this useful? what does it provide?
}