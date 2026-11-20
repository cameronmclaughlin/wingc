# Dev Notes for Wingc

## Stages
- Lexing
	- Converts source code to a token
	- Preprocessing
		- Identifies preprocessing macros and applies them to the context
- Parsing
	- Makes a token list from the lexing stage ready for conversion
- Conversion
	- Converts a token list from the parsing stage into an assembly file
- Assembler
	- Converts assembly files from the conversion stage into a single executable
## Lexing
Lexing is kinda beginner but I'll improve later. Just loops through every character in source ignoring whitespaces. Checks for keywords, operators, condtionals, and types. If it's not in the token category but it starts with an alphabet character, it see's it as a string. If starts with a digit, it see's it as a digit.
### Preprocessing
Determines what get's passed into the lexer and what doesn't. Also configures certain aspects of the compiler. Practically its own mini lexer. Uses the ! keyword. Preprocessors:
- bits
	- Specifies the bit count
	- usage: !bits i!
	- possible inputs
		- i
			- 16
			- 32
			- 64	
- os
	- Specifies target OS
	- usage: !os s!
	- possible inputs
		- s
			- lnx / linux
			- bsd / freebsd
			- mac / macos / mach / apple
			- win / windows
			- bios
			- uefi
- arch
	- Specifies target CPU architecture
	- usage: !arch s!
	- possible inputs
		- s
			- amd
			- arm
- endif
	- Ends an if
	- usage: !endif!
- ifbits
	- Executes code between it and !endif! if the condition matches
	- usage: !ifbits i!
	- possible inputs
		- i
			- 16
			- 32
			- 64
## Parsing
Parsing is a lil more complex. Not as complete.
Parsing Steps:
- Scoping
	- Goes through tokens, creates scopes. Assigns the current scope to each token it see's.
- Ordering
	- Goes through tokens, make sure things are in order. For example, the source code
	```
	fn main none(){
		exit(1);
	}
	```
	Would be caught as a syntax error on line 1, since line one turns into the token list
	```
	KEYWORD fn
	NAME main
	TYPE none
	KEYWORD (
	KEYWORD )
	KEYWORD {
	```
	And the parser sees `KEYWORD fn`, it expects it to have an order of
	```
	KEYWORD fn
	TYPE
	NAME
	KEYWORD (
	``` 
	But it doesn't, so syntax error.
- Contextualization
	- Goes through tokens after ordering, adds context flags to certain tokens. Example: A source line of
	`exit(1);`
	Is lexed into a token list of
	```
	NAME exit
	KEYWORD (
	NUMBER 1
	KEYWORD )
	KEYWORD ;
	```
	Which, through contextualization, is converted into a simple
	```
	NAME exit (flag CALL)
	KEYWORD (
	NUMBER 1 (flag ARGUMENT)
	KEYWORD )
	KEYWORD ;
	```
	And that gives context for the conversion stage
- Name Tabling
	- Goes through tokens after contextualization and creates a name table.
	- A name table entry looks like this:
	```
	struct NameTableEntry{
		char* name;
		int argc; // MAX 128
		enum ParsingTokenType[128];	
	};
	```
- FPEMDAS
	- Creates a binary tree for each math statement

## Convention
The calling convention is custom because I didn't want to re-learn GCC conventions after ChOS.
Okay so when a function is called, it pushes rbp then sets it to rsp to create the new stack frame. After that it does sub rsp, (8 * varc) + (8 * argc).
Going to need to keep stack alignment to not fuck up special instructions.
### Variables
#### Local (scope > 0)
For local variables, they are kept behind %rbp
#### Global (scope == 0)
For global variables, theya are kept in the .data section