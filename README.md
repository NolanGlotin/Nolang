# Nolang

Nolang is a minimalistic programming language inspired by the Turing machine, with a very simple syntax and semantics. Each program outputs a boolean value indicating whether an input is accepted by the Turing machine or not.

## Syntax
The syntax of Nolang is extremely simple. The principle is to move a tape head left or right and to read and write symbols on the tape from a finite alphabet. To do this, the machine navigates between different states, and the instruction set is defined by a transition function that maps the current state and tape symbol to a new state, a new tape symbol, and a direction to move the tape head.

### Declaration of the alphabet
Every Nolang program starts with the declaration of the alphabet, which is a finite set of symbols that can be used on the tape. The alphabet is defined using curly braces `{}` and can contain any characters except for whitespace and commas. By default, the tape is filled with the first symbol in the alphabet. For example:
```
{_, a, b, c}
```
This declares an alphabet consisting of the symbols `_` (empty), `a`, `b`, and `c`.

### Declaration of states
After declaring the alphabet, you can declare the states of the Turing machine. States are defined by first declaring their name and then defining the transition function for each state between parentheses `()`. Each transition is defined by the current tape symbol that the machine reads, the new tape symbol to write, the direction to move the tape head (`<` for left, `>` for right), and the next state to transition to, separated by commas and ending with a semicolon `;`.
For example:
```
state_1 (
    a, b, >, state_2;
    b, a, <, state_3;
)
```

This defines a state called `state_1` with two transitions. If the machine reads `a`, it will write `b`, move the tape head to the right, and transition to `state_2`. If it reads `b`, it will write `a`, move the tape head to the left, and transition to `state_3`.

### Declaration of the intial and accepting states
Finally, you need to declare the initial state and the accepting states. The initial state is the state where the Turing machine starts, and the accepting states are the states where the machine halts and accepts the input. If it reaches a non-accepting state with no transition defined for the current tape symbol, it halts and returns failure.
The initial state is declared using the symbol `>`, followed by the name of the state. An accepting state is declared using the symbol `*` in front of its name. For example:
```
> init ()
* success ()
```


## Example
A simple example of a Nolang program that checks if a binary number is even or not:
```
{_, 0,1}

> init (
    0, 0, >, init;
    1, 1, >, init;
    _, _, <, check;
)

check (
    0, 0, <, success;
    1, 1, <, fail;
)

* success ()
fail ()
```

## Usage
First, build the Nolang interpreter using the Makefile:
```
make
```

Then run a Nolang program by providing the path to the source file and the input string as arguments:
```
./nolang program.ng input_string
```

Example:
```
./nolang examples/even.ng 1010
```
Output:
```
Starting execution...
  0) ________________________________________________1010________________________________________________
                                                     ^                                                   
  1) ________________________________________________1010________________________________________________
                                                      ^                                                  
  2) ________________________________________________1010________________________________________________
                                                       ^                                                 
  3) ________________________________________________1010________________________________________________
                                                        ^                                                
  4) ________________________________________________1010________________________________________________
                                                         ^                                               
  5) ________________________________________________1010________________________________________________
                                                        ^                                                
  6) ________________________________________________1010________________________________________________
                                                       ^                                                 
End of execution. Accepted : YES
```


## Credits
This project was developed by Nolan Glotin in 2025 as part of his studies in preparatory class. It is inspired by the Turing machine and the theory of computation. It is still experimental and may contain bugs or limitations.