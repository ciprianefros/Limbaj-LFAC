# Limbaj-LFAC

## Overview

This language is a custom-designed programming language built as a project for the Formal Languages, Automata and Compilers Course during our first semester in year II. The language has itw own unique syntax and supports several built-in functions, like Print and TypeOf. This project was developed by Efros Ciprian and Chelba Sergiu-Mihai as an academin exercise to learn the fundamentals of language design, parsing and compilation.

## Features

- Custom syntax defined for the language.
- Support for built-in functions:
  - **TypeOf:** Returns the type of a variable or expression.
  - **Print:** Outputs a value to the console.
- Demonstrates key concepts in compiler construction including lexical analysis and parsing.

## Usage

To run the Limbaj LFAC project, follow these steps:

1. **Navigate to the project firectory:**
`cd Limbaj-LFAC`
2. **Make sure the build script is executable:**
Run `chmod +x compile.sh`
3. **Build the project** by executing the build script with the target executable name (`limbaj`):
`./compile.sh limbaj` or `bash compile.sh limbaj`
4. **Run the executable** by providing a file from the `resources` folder
`./limbaj resources/input.txt` or `./limbaj resources/inputOk.txt`

You can also write any other program and try to execute it using the following format:
`./limbaj path-to-program`


## Additional Notes
This project was developed as part of the coursework for the Formal Languages, Automata, and Compilers class. Any resemblance to production-level languages or compilers is purely coincidental and for educational purposes only.
