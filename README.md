# Wing programming language development document
## Features
- C inspired
- Cross compatible compilation and stdlib
- Can change target CPU architecture
    - amd
    - arm
- Can change target byte architecture
    - 64 bit
    - 32 bit
    - 16 bit
- Can change target OS
    - Linux
    - Mac
    - ChOS
    - BIOS
    - EFI
## Syntax
- Curly braces for scope
- Rounded braces for arguments
- C conditionals
- Tilde for not conditional
- Exclamation point for preprocessors
## Keywords
- fn - Define function
- if - If statement
- switch / case - Switch case statement
- for - For loop
- while - While loop
- break - Break out of loop
- skip - Skip one loop
- ret - Return
## Built-in Types
- i8 - 1 byte integer
- i16 - 2 byte integer
- i32 - 4 byte integer
- i64 - 8 byte integer
- ch - char (equivalent to i8)
- point - Pointer
- addr - Address of
- arr - Array
- none - Void