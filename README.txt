ParserCodegen

Instructions:

1. Place your Tiny PL/0 code into 
    input.txt

2. Compile the compiler: 
    gcc compiler.c

3. Run the compiler: 
    ./a.out [FILENAME] [ARGS]

* Args are optional, and are the following compiler directives:

  -l : print the list of lexemes/tokens (scanner output) to the screen

  -a : print the generated assembly code (parser/codegen output) to the screen

  -v : print virtual machine execution trace (virtual machine output) to the screen
  
