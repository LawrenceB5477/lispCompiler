
## lisp is a family of languages where computation is represented by lists

# Chapter 3

## Programs
  - Programs consist of only structure and function definitions
  - You can use the functions and types of other libraries (header files)
  - C programs start in main function, it calls other functions it needs

## Variables
  - A variable is a named piece of data
  - Variables have types, ones built in or ones we make
  - Statement terminated with ;

## Function Declaration
  - A function is a computation, it performs a series of tasks and
  optionally manipulates the state of the program. It returns a value
  or it has a side effect. It takes in variables as input, and optionaly returns
  some output.

## Structure Declarations
  - Structures are our own types, they are essentially bundles of variables
  - Put structure definitions above functions that would use them

## Pointer
  - Essentially variables whose values are addresses of other variables,
  instead of some actual value

## String
  - Textual data, represented in c as an array of characters
  terminated with null character '\0'
  - Represented by char*

## Conditionals
  - Allow you to make decisions in your code, to execute certain blocks
  of statements based on the truthiness of some condition.
  - Inside a conditional statement's condition, any value that is
  not zero is true

## Loops
  - Allow you to repeat code over and over again, with a different program
  state each time. The loop repeats as long as the condition is true

# Chapter 4 An interactive Prompt

  - We want to make an interactive prompt that returns output to the user
  when given input so we can dynamically play with the language
  - Called a read evaluate print loop, used in languages like python
  - We start by making a prompt that echos back input to the user, we will extend it
  to actually process the input

## An interactive prompt
 - fgets reads user input to a new line
 - editline allows you to edit input using arrow keys on mac and linux
 - readline gets data input from a prompt while being able to edit the input
 - add_history allows us to record your history of input so you can use the arrow keys to get them
 - we need to delete the input given to us by readline, it returns a new pointer
 instead of reading to an existing pointer, in other words it allocates memory
 for the string it reads in, and we need to release the memory to prevent shit
 tons of variables from being stored in memory each time the loop runs
- linking process allows compiler to call functions from library
use -ledit to link your program to headers

# Chapter 5 Languages

## What is a programming language?
  - A programming language defines instructions that you can give to a computer to
  perform some useful task. A programming language has a structure and a syntax, rules
  for what is valid input.
  - An important observation is that languages are made up of recursive and
  repeating structures (nested)
  - This ocurrs in programming. Structures are nested inside of each other,
  statements are nested within each other "vertically" using blocks, and expressions
  are nested "horizontally" with the use of operators to combine smaller expressions
  to create larger ones. As an example, an if statement contains statements inside of it,
  each of which could themselves be an if statements.
  - The set of rules that define how structures can be nested and replaced with
  each other are called re-write rules. The set of re-write rules that defines
  a language is called a grammar.
  - An important consequence of this observation is that any piece of a language,
  no matter how complex, can be understood and processed using a finite number of
  rewrite rules. We simply recursively analyze and break down the language into
  smaller components that can be processed.

  - To write a compiler for our lisp, we need to write a grammar that describes it.
  We use this grammar to validate input, and we can also use it to create
  a representation of it in memory, which will allow us to process it and
  evaluate the instructions specified by the language.

## Parser combinator
  - A parser is a program that reads, understands, and processes a language
  - A parser combinator is a program that generates a parser given a
  grammar definition. You tell it the rules the language should follow, it
  reads in the language for you.
  - mpc is what I'm using, you can specify the grammar in a big string or
  you can use code that looks like a grammar

## Coding Grammars
   - in specifying doge, we look for zero or more parsers, this means we
   can processes input of any length. Using a finite amount of re-write rules,
   we can process an infinitely complex piece of a language
   - using mpc functions, we could slowly build up more complex languages, or
   we can specify the grammar directly using a string

## Natural Grammar
  - This is closer to a textual specification of a grammar. We write out the grammar
  as a series of write out rules, specifying what each construct can be made up of,
  including other construct
  - mpc_new defines a re-write rule, mpc_lang defines them and the grammar

# Ch 6 Parsing

## Polish Notation
  - we're going to implement a grammar called polish notation, it's a mathematical
  subset of lisp, operators come before operands
  - operators always come first, followed by operands or other expressions in paranthesis
  - a program is an operator followed by an expression, where a expression is a
  number or in parantheses, an operator followed by an expression

## - Textual description of grammar -
- Program - the start of input, an operator, one or more expresions, end of input
- Expression - a number or "(", an Operator, one or more expressions, and ")"
- Operator - + or - or * or /
- Number - and optional 1, and one or more 0 - 9

## Regular Expressions
- Allow you to create more precise and fine rewrite rules at the cost
of being able to nest rules

### Regex rules
  . - one of any character is required
  a - a is required
  [abcdef] - one of any character in the set is required
  [a-f] - a character from a to f is required
  a? - a is optional
  a* - 0 or more a
  a+ - 1 or more a
  ^ - the start of input
  $ - the end of input

  - Number - /-?[0-9]+/

## Installing mpc
  - "" searches current directory for header files first, <> searches
  system locations first
  - mpc_cleanup() deletes the parsers, pass them the pointer variables to the
  parsers you made

## Parsing user input
  - We made the parser, now we need to use it on the input each iteration
  - We need to parse the input instead of simply echoing it
  mpc_parse("stdin", input string, parser, result to output it to),
  copies result to mpc_output_t r, returns 0 on failure, 1 on success
  - on success an internal structure is copied into r on the field output, so it's a
  structure, we can print it out then delete it

# Ch 7 Evaluating

- Yay! Now we're gonna process instructions!
## Trees
  - We have the input read and structured, now we need to evaluate it and perform
  the actions encoded within
  - Abstract Syntax Tree - what we saw earlier, it represents the structure of the
  program based on the user's input
  - make a grammar - make a parser to read the input - create an abstract syntax tree
  to represent the structure of the program
  - at the leaves are numbers and operators, the data, at the branches are rules
  to process the tree, information on how to evaluate it
  - branches - how to evaluate, leaves - data to evaluate
  - we get mpc_ast_t from the parse, a type representing an abstract syntax tree
  - The struct has many fields
    - tag - the info that preceeds the node, contains a list of all re-write rules to
    parse the node, it tells us what rules where used to make the node
    - contents - the actual data, like an operator or number. This is filled for
    leaves, empty for branches
    - state - the state the parser was in when it made the node, like what row and column
    it was at
    - children_num tells us how many children a node has
    - mpc_ast_t** - an array of the the children
    - mpc_ast_t* is a pointer to a struct, so to access its fields we use ->,
    to access the fields of pointer types, use ->
## Recursion
  - The abstract syntax tree is a recursive representation of our program, it
  contains repeated and nested structures (ie it is made up of tree structures
    that have trees as their children)
  - Just like our grammar and language contains repeated structures, so does the
  Abstract syntax tree
  - We can use this property of the data repeating itself and recursion, which is
  a function calling itself. Essentially, we create a function that handles one node,
  and calls itself to handle the children of that node. The end result is a function that
  breaks down processing the tree into smaller pieces, processes these small pieces, and
  reasembles the pieces into a result that is returned

## Evaluation
  - if the node has the number tag, it is literally just a leaf with a number
  - if node is tagged expr and is not a number, look at it's second child for
  operator, then apply this operator to the *evaluation* of its third and fourth children
  This is the recursive case
  - String functions
    - atoi() - converts a char to an int
    - strcmp - returns 0 if two strings are equal
    - strstr - takes in two strings, returns a pointer to the location of the second
    string in the first or 0 if second string is not in first

# Chapter 8 Error Handling
  - We need some way to report errors in the evaluation of expresions
  - We can make errors a possible result of evaluating an expression
  - let's make a structure that can act as an error or a meaningful result
  - make a struct with fields represeting each thing that can be represetned,
  and a special type field to tell us which field we should access

## Enumerations
  - Type and err are ints, we assign meaning to int values to specify
  what the structure is. If the type field is a certain number, then
  it means that the whole structure is a number or an error or whatever
  - We used named constants that have these values so we know what numbers
  mean what
  - to make these name constants, we use enums
  enum {LVAL_NUM, LVAL_ERR};
  - Enums are declerations of variables that that are assigned integer
  values under the hood
  - switch statements compare the value of an expression to other values called cases
  and executes the code under the case till a break statement
  - we should use the strtol function to convert from string to long, we
  check a variable called errno to make sure the conversion happened
