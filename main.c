#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "config.h"

// Types and structs
typedef char *str;

typedef struct {
    bool enabled;
    int write;
    int move;
    int next;
} state_case_t;

typedef struct {
    state_case_t *cases;
    bool accepting;
} state_t;

typedef struct {
    char *symbols;
    int symbol_nb;
    state_t *states;
    int state_nb;
    int start_state;
} turing_machine_t;

// Global vars
int line = 0;
FILE *file = NULL;
char buffer[BUFFER_SIZE + 1];
int buffer_length = 0;
char c;

// Throw exception and quit
void failwith(str msg) {
    printf("Error : ");
    if (file != NULL)
        printf("line %d\n", line + 1);
    printf("%s\n", msg);
    if (file != NULL)
        fclose(file);
    exit(EXIT_FAILURE);
}

// Return the id of a state
int get_state_id(str name, str *states, int state_nb) {
    int id = 0;
    // Ignore starting / accepting flags
    if (name[0] == '>' || name[0] == '*')
        name++;
    while (id < state_nb && strcmp(name, states[id]) != 0) {
        id++;
    }
    if (id >= state_nb)
        return -1;
    return id;
}

// Return the id of a symbol
int get_symbol_id(char s, char *symbols, int symbol_nb) {
    int id = 0;
    while (id < symbol_nb && s != symbols[id]) {
        id++;
    }
    if (id >= symbol_nb)
        return -1;
    return id;
}

// Update buffer with current char c (fail if buffer overflows)
void update_buffer() {
    if (buffer_length >= BUFFER_SIZE)
        failwith("Buffer overflow (see config file to increase limit)");
    buffer[buffer_length] = c;
    buffer_length++;
    buffer[buffer_length] = '\0';
}

turing_machine_t *parse() {
    // Scan bools
    bool comment = false;
    bool scan_set = false;
    bool scan_state = false;

    // Value and state names
    char *symbols = (char *)malloc(sizeof(char)*MAX_SYMBOL_NUMBER);
    int symbol_nb = 0;

    str *states = (str *)malloc(sizeof(str)*MAX_STATE_NUMBER);
    for (int i = 0; i < MAX_SYMBOL_NUMBER; i++)
        states[i] = (str)malloc(sizeof(char)*(BUFFER_SIZE + 1));
    int state_nb = 0;

    // Parse source file
    line = 0;
    while(fscanf(file, "%c", &c) != EOF) {

        if (c == '\n') {
            buffer_length = 0;
            line++;
            comment = false;
        }

        else if (c == '/')
            comment = true;

        else if (!comment) {
            if (scan_set) {
                if (c == ',' || c == '}') {
                    if (buffer_length == 0)
                        failwith("Invalid syntax : empty element while scanning set");
                    if (buffer_length > 1)
                        failwith("A symbol must be one character long");
                    else if (symbol_nb >= MAX_SYMBOL_NUMBER)
                        failwith("Too many symbols in the set (see config file to increase limit)");
                    else {
                        buffer[buffer_length] = '\0';
                        symbols[symbol_nb] = buffer[0];
                        symbol_nb++;
                        buffer_length = 0;
                    }
                    if (c == '}') {
                        buffer_length = 0;
                        scan_set = false;
                    }
                }
                else if (c != ' ') {
                    update_buffer();
                }
            }

            else if (c == '{')
                scan_set = true;
            
            else if (!scan_state && c == '(') {
                scan_state = true;
                if (buffer_length == 0)
                    failwith("Invalid syntax : each state must have a name");
                else if (state_nb >= MAX_STATE_NUMBER)
                    failwith("Too many states (see config file to increase limit)");
                else {
                    buffer[buffer_length] = '\0';
                    strcpy(states[state_nb], buffer);
                    state_nb++;
                    buffer_length = 0;
                }
            }

            else if (c == ')')
                scan_state = false;

            else if (c != ' ' && c != '>' && c != '*')
                update_buffer();
        }
    }

    // Read states' content to build turing machine
    turing_machine_t *machine = (turing_machine_t *)malloc(sizeof(turing_machine_t));
    machine->symbol_nb = symbol_nb;
    machine->symbols = (char *)malloc(sizeof(char)*symbol_nb);
    for (int s = 0; s < MAX_SYMBOL_NUMBER; s++)
        if (s < machine->symbol_nb)
            machine->symbols[s] = symbols[s];
    machine->state_nb = state_nb;
    machine->states = (state_t *)malloc(sizeof(state_t)*state_nb);
    for (int s = 0; s < state_nb; s++)
        machine->states[s].cases = (state_case_t *)malloc(sizeof(state_case_t)*symbol_nb);
    for (int s = 0; s < state_nb; s++) {
        machine->states[s].accepting = false;
        for (int i = 0; i < symbol_nb; i++)
            machine->states[s].cases[i].enabled = false;
    }
    machine->start_state = 0;

    int current_state = 0;
    int current_state_case = 0;
    int current_state_case_entry = 0;

    comment = false;
    scan_set = false;
    scan_state = false;

    rewind(file);
    line = 0;
    while(fscanf(file, "%c", &c) != EOF) {
        if (c == '\n') {
            line++;
            comment = false;
            buffer_length = 0;
        }

        else if (c == '/')
            comment = true;

        else if (!comment) {
            if (scan_state) {
                if (c == ')')
                    scan_state = false;
        
                else if (c == ',' || c == ';') {
                    switch (current_state_case_entry) {
                        case 0: {
                            // Read
                            if (buffer_length > 1)
                                failwith("A symbol must be one character long");
                            int s = get_symbol_id(buffer[0], symbols, symbol_nb);
                            if (s == -1)
                                failwith("Bad read symbol matching : not a member of the set");
                            current_state_case = s;
                            machine->states[current_state].cases[current_state_case].enabled = true;
                            break;
                        }
                        case 1: {
                            // Write
                            if (buffer_length > 1)
                                failwith("A symbol must be one character long");
                            int s = get_symbol_id(buffer[0], symbols, symbol_nb);
                            if (s == -1)
                                failwith("Bad write symbol matching : not a member of the set");
                            machine->states[current_state].cases[current_state_case].write = s;
                            break;
                        }
                        case 2: {
                            // Direction
                            if (strcmp(buffer, ">") == 0)
                                machine->states[current_state].cases[current_state_case].move = 1;
                            else if (strcmp(buffer, "<") == 0)
                                machine->states[current_state].cases[current_state_case].move = -1;
                            else
                                failwith("Bad direction instruction, should be '<' to go left and '>' to go right");
                            break;
                        }
                        case 3: {
                            // Next state
                            int s = get_state_id(buffer, states, state_nb);
                            if (s == -1)
                                failwith("Bad next state matching : no states found with this name");
                            machine->states[current_state].cases[current_state_case].next = s;
                            break;
                        }
                    }
                    buffer_length = 0;
                    if (c == ';')
                        current_state_case_entry = 0;
                    else
                        current_state_case_entry++; 

                }
                else if (c != ' ') {
                    if (current_state >= MAX_STATE_NUMBER)
                        failwith("Too many states (see config file to increase limit)");
                    if (current_state_case >= MAX_SYMBOL_NUMBER)
                        failwith("Too many cases for this state (see config file to increase limit)");
                    if (current_state_case_entry >= 4)
                        failwith("Too many entries for this state case (expected format is [read], [write], [move direction], [next state])");
                    update_buffer();
                }
            }
            
            else if (c == '(') {
                scan_state = true;
                current_state = get_state_id(buffer, states, state_nb);
                assert(current_state >= 0);
                if (buffer[0] == '>')
                    machine->start_state = current_state; // Start state
                else if (buffer[0] == '*')
                    machine->states[current_state].accepting = true; // Accepting state
            }

            else if (c != ' ')
                update_buffer();
            
            else if (c == '}')
                buffer_length = 0;
        }
    }

    // Free temp data
    free(symbols);

    for (int i = 0; i < MAX_STATE_NUMBER; i++)
        free(states[i]);
    free(states);

    return machine;
}

void display_memory(turing_machine_t *machine, int *memory, int step, int pointer) {
    printf("%*d) ", 3, step);
    for (int i = 0; i < TAPE_SIZE; i++)
        printf("%c", machine->symbols[memory[i]]);
    printf("\n     ");
    for (int i = 0; i < TAPE_SIZE; i++)
        printf("%c", i == pointer ? '^' : ' ');
    printf("\n");
}

// Warning : depending on the source code, this function can run infinitely
bool run(turing_machine_t *machine, str input_mem) {
    int input_len = strlen(input_mem);
    int pointer = (TAPE_SIZE - input_len)/2;
    int state = machine->start_state;
    int step = 0;
    bool accepting = false;

    if (input_len >= TAPE_SIZE)
        failwith("Too long input sequence : exceeds max tape length (see config file to increase it)");

    // Create memory and initialize it
    int *memory = (int *)malloc(sizeof(int)*TAPE_SIZE);
    for (int i = 0; i < TAPE_SIZE; i++)
        memory[i] = 0; // Default symbol
    // Fill begining of memory with the given input
    for (int i = 0; i < input_len; i++) {
            int s = get_symbol_id(input_mem[i], machine->symbols, machine->symbol_nb);
            if (s == -1) {
                free(memory);
                failwith("Bad symbol matching in input sequence : not a member of the set");
            }
            memory[i + pointer] = s;
    }

    
    display_memory(machine, memory, step, pointer);
    while(machine->states[state].cases[memory[pointer]].enabled) {
        // Read
        state_case_t info = machine->states[state].cases[memory[pointer]];
        // Write
        memory[pointer] = info.write;
        // Move
        pointer += info.move;
        // Jump to next state
        state = info.next;
        // Display tape state
        step++;
        display_memory(machine, memory, step, pointer);

        if (machine->states[state].accepting) {
            accepting = true;
            break;
        }
        if (pointer < 0 || pointer > TAPE_SIZE) {
            free(memory);
            failwith("Error : tape memory overflow, tape size can be increased in config file");
        }
        if (step == MAX_STEPS) {
            free(memory);
            failwith("Steps number exceeded maximum, check if your program stops or increase max steps in config file\n");
        }
    }
    free(memory);

    return accepting;
}

// Free turing machine data
void free_machine(turing_machine_t *machine) {
    for (int s = 0; s < machine->state_nb; s++)
        free(machine->states[s].cases);
    free(machine->symbols);
    free(machine->states);
    free(machine);
}

void display_machine(turing_machine_t *machine) {
    printf("Symbols {");
    for (int s = 0; s < machine->symbol_nb; s++)
        printf("%c%s", machine->symbols[s], s == machine->symbol_nb - 1 ? "}\n" : ", ");
    printf("=====================\nStates :\n");
    for (int s = 0; s < machine->state_nb; s++) {
        if (s == machine->start_state)
            printf("[Start state]\n");
        for (int i = 0; i < machine->symbol_nb; i++)
            if (machine->states[s].cases[i].enabled)
                printf("r: %c, w: %c, %c, next: %d\n", machine->symbols[i], machine->symbols[machine->states[s].cases[i].write], machine->states[s].cases[i].move == 1 ? '>' : '<', machine->states[s].cases[i].next);
        printf("--------------------\n");
    }
}

int main(int argc, char **argv) {
    // Check for a source file in argument
    if (argc != 2 && argc != 3) {
        printf("Invalid input : expecting source file\n");
        return EXIT_FAILURE;
    }

    // Load source file
    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Failed to open file : '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Read source file
    turing_machine_t *machine = parse();
    fclose(file);
    file = NULL;

    // Run machine
    printf("Starting execution...\n");
    bool accepted = run(machine, argc == 3 ? argv[2] : "");
    printf("End of execution. Accepted : %s\n", accepted ? "YES" : "NO");

    // Free machine
    free_machine(machine);
    
    return EXIT_SUCCESS;
}