# Minishell 🐚


Minishell is a custom implementation of a Unix shell that recreates essential functionalities like command execution, pipelines, and input/output redirection. It’s designed to provide a deeper understanding of how a shell processes and executes user commands, from parsing input to managing system resources. 

While simplified compared to a full-fledged shell, Minishell captures the core mechanics, making it both a practical challenge and an educational experience, offering insight into the inner workings of command-line tools.


# Parsing: Transforming Input into Structure

The parsing phase is at the heart of this shell's functionality, converting raw user input into a structured representation that enables efficient command execution. This process consists of two tightly coupled stages:

- **Lexical Analysis (Lexer)**: Breaking down the input string into discrete tokens.
- **Syntactic Analysis (Parser)**: Organizing these tokens into an Abstract Syntax Tree (AST) that reflects the hierarchical structure of commands and operations.

Together, the lexer and parser implement a **Recursive Descent LL(1)** approach, blending modularity with predictive and efficient processing.

## Key Design Principles: Recursive Descent LL(1)
The parsing system implements Recursive Descent LL(1) parsing, which is well-suited for shell grammar due to its simplicity and predictability:

- Recursive Descent: Parsing is modular, with each grammar rule mapped to a function (e.g., `parse_command()`, `parse_redirect()`).
- LL(1): A single-token lookahead enables the parser to decide the next step without backtracking.

This approach provides:

- Efficiency: Input is processed in a single pass.
- Modularity: Functions for specific grammar rules make the codebase easy to maintain.
- Robustness: Errors are handled gracefully, preserving the overall structure.

## Lexical Analysis: The Lexer

The lexer serves as the first step, dissecting the raw input into **tokens**, which are the fundamental building blocks of parsing. A token is a categorized piece of the input, such as a command, operator, or quoted string.

### How the Lexer Works

The lexer uses a **Finite State Machine (FSM)** to process input. An FSM transitions between a set of defined states based on the current character being analyzed, making it an ideal mechanism for tokenizing structured input like shell commands.

#### **Key Concepts of the FSM**

1. **States:**
    - The lexer transitions between predefined states based on input, such as:
      - `in_word`: Processing commands or arguments.
      - `in_pipe`: Recognizing the | character.
      - `in_dquote/in_squote`: Handling quoted strings.
      - `in_error`: Flagging invalid input.

2. **Transitions:**
    - State transitions are determined by input characters (e.g., | triggers in_pipe; << triggers in_dless for heredocs).

3. **Actions:**
    - States dictate actions, such as generating tokens (WORD, PIPE, REDIRECT) or skipping separators like whitespace.

4. **Error Handling:**
    - An in_error state captures invalid input for robust tokenization.


#### **Workflow of the Lexer**

1. **Reading the Input**:
   - The lexer scans the input string character by character.
   - It evaluates each character's role and updates the state accordingly.

2. **Generating Tokens**:
   - Based on the current state, the lexer produces tokens using helper functions like `command_token()`, `pipe_token()`, and `redirect_token()`.

3. **Handling Dynamic Input**:
   - To accommodate variable-length input, the lexer dynamically resizes its token array using functions like `realloc_token_array()`.

4. **Error Detection**:
   - Malformed input, such as unclosed quotes, triggers the `in_error` state, generating an `ERROR_TOKEN`.

#### Example:
For the input:
```bash
echo "Hello, world!" | grep Hello > output.txt
```
The lexer produces the following output:
```bash
[WORD("echo"), DQUOTE("Hello, world!"), PIPE, WORD("grep"), WORD("Hello"), GREAT, WORD("output.txt")]
```

## Syntactic Analysis: The Parser
The parser takes the token stream from the lexer and organizes it into an Abstract Syntax Tree (AST). The AST represents the logical relationships between commands, operators, and arguments, allowing the shell to execute the input correctly.

### How the Parser Works
- **Building the AST:**
  - The parser processes tokens recursively, starting with the highest-level construct (e.g., a complete command) and breaking it into smaller components.  
  - Each node in the AST corresponds to a syntactic element:
    - *Command Nodes*: Represent commands and their arguments.
    - *Pipeline Nodes*: Represent pipes (`|`) connecting commands.
    - *Redirection Nodes*: Represent input/output redirections (`<`, `>`, `>>`, `<<`).
- **Predictive Parsing:**
  - The parser uses a single-token lookahead (LL(1)) to decide how to proceed:
    - A `WORD` token starts a command.
    - A `PIPE` token links two commands.
    - A `LESS` or `GREAT` token indicates a redirection.
  - This predictive behavior avoids backtracking, ensuring efficient parsing.
- **Error Handling:**
  - If the token stream doesn't match the expected patterns, the parser generates error nodes to gracefully handle the malformed input.
- **Output:**
  - The resulting AST provides a hierarchical representation of the input, ready for execution.

### Example Workflow

For the input:
```bash
echo "Hello, world!" | grep Hello > output.txt
```
The parser generates an AST:
```bash
PIPE
├── COMMAND("echo")
│   └── PARAM("Hello, world!")
└── COMMAND("grep")
    ├── PARAM("Hello")
    └── OUTFILE("output.txt")
```

### Error Handling:

If the input is malformed, such as:
```bash
ls -l | | grep file
```
The parser wraps the problematic section in an error node, preserving structure and ensuring robust error reporting:
```bash
ERROR
└── PIPE
    ├── COMMAND("ls")
    │   └── PARAM("-l")
    └── NULL
```


# Execution: Turning Structure into Action

The **Execution** phase transforms the Abstract Syntax Tree (AST) generated during parsing into actual command execution. This involves traversing the AST, resolving commands and arguments, managing redirections, and orchestrating processes for pipelines and commands. The shell employs a structured and efficient execution strategy grounded in theoretical principles.

## Execution Strategy

The execution strategy integrates key theoretical approaches:

1. **Post-Order AST Traversal**:
   - Commands and their dependencies (e.g., redirections and pipelines) are resolved in a post-order sequence, ensuring that child nodes are processed before their parent.

2. **Process-Oriented Execution**:
   - **Built-In Commands**: Handled directly in the parent process to avoid unnecessary forking.
   - **External Commands**: Executed in isolated child processes using `fork()`.

3. **Pipeline Execution**:
   - Commands in pipelines (e.g., `cmd1 | cmd2`) are executed in parallel, with Unix pipes (`pipe()`) facilitating data flow between processes.

4. **Dynamic Interpretation**:
   - Each AST node is interpreted dynamically during traversal, with actions tailored to the node type (e.g., command execution, input/output redirection).

5. **Error Handling**:
   - Errors, such as ambiguous redirections or invalid commands, are detected early and handled gracefully.

## Key Components

### AST Utilities (`ast_analysis_utils.c`, `ast_utils.c`)
These utilities extract and manipulate data from the AST for execution:

- **Command and Argument Resolution**:
  - `collect_cmd()`: Retrieves the main command from a node.
  - `collect_options()`: Gathers arguments/options associated with the command.

- **Node Identification**:
  - `is_executable_node()`: Determines if a node represents an executable command or redirection.
  - `has_outfile()` and `get_outfile_node()`: Identify and retrieve output redirection nodes.

- **Traversal Helpers**:
  - `count_cmds()`: Counts executable nodes to assist with process orchestration.
  - `siblings_to_array()`: Converts sibling nodes into an array for argument processing.


### Execution Logic (`executor.c`)
The execution logic governs AST traversal and command execution:

1. **AST Traversal**:
    - `execute_ast()` uses a stack-based approach for **post-order traversal**, ensuring that child nodes (e.g., redirections) are processed before commands.

2. **Command Execution**:
   - **Built-In Commands**:
     - Handled directly in the parent process.
     - Example: `cd`, `exit`.
   - **External Commands**:
     - Resolved using the `PATH` environment variable and executed in child processes created by `fork()`.

3. **Redirection Management**:
   - Input redirections (`<`, `<<`) are set up in `pre_execute()`.
   - Output redirections (`>`, `>>`) are processed in `spawn_process()`.

4. **Pipeline Handling**:
   - Commands in a pipeline are connected via pipes, with the output of one process becoming the input for the next.


### Helper Utilities (`utils.c`)
These utilities support execution by managing arguments and commands:

- **Command-Argument Combination**:
  - `join_cmd_and_options()`: Combines a command and its arguments into an array for execution.
- **Array Management**:
  - `get_arr_without_last()`: Modifies arrays, often for pipeline-related adjustments.

## Example Workflow

For the input:
```bash
cat input.txt | grep "pattern" > output.txt
```

1. **AST Traversal:**
    - Nodes for `cat`, `grep`, `input.txt`, and `output.txt` are identified.
    - The pipeline and redirection are resolved.

2. **Redirection Setup:**
    - Input redirection connects `cat` to `input.txt`.
    - Output redirection connects `grep` to `output.txt`.

3. **Pipeline Execution:**
    - `cat` and `grep` are executed in parallel processes, with a pipe connecting their output and input.

4. **Process Management:**
    - The parent process waits for child processes to complete.

## Highlights of the Execution Design

1. **Post-Order Traversal:**
    - Dependencies like redirections and pipes are resolved before executing commands.

2. **Process Efficiency:**
    - Built-in commands are executed in the parent process to avoid unnecessary overhead.
    - External commands are isolated in child processes for robust handling.

3. **Pipeline Parallelization:**
    - Commands in pipelines execute concurrently, with seamless data flow through pipes.

4. **Error Resilience:**

    - Ambiguous redirections and invalid commands are handled gracefully, ensuring shell stability.

This execution framework provides a robust, modular, and efficient mechanism for running commands, adhering to Unix shell principles while supporting complex pipelines and redirections.

# Redirection and I/O Management: Controlling Data Flow

Redirection enables commands to manipulate input and output streams, including:
- **Input Redirection**: `<` and `<<` (heredoc).
- **Output Redirection**: `>` (overwrite) and `>>` (append).

This shell handles redirection by dynamically modifying file descriptors during execution, fully integrating it with the AST traversal.

## Key Features

1. **Dynamic File Descriptor Management**:
   - Input/output streams are redirected using `dup2()` to replace `STDIN_FILENO` or `STDOUT_FILENO`.
   - Original streams are restored after command execution.

2. **Heredoc Handling**:
   - Heredoc input (`<<`) is written to a temporary file in `/tmp` and used as input for commands.

3. **Error Resilience**:
   - Invalid files or ambiguous redirections are detected early, ensuring robust execution.


## Highlights

- **Flexible Redirection**: Handles complex scenarios with multiple or chained redirections.
- **Robust Heredoc Support**: Manages quoted delimiters and variable expansion.
- **Error Handling**: Detects and reports invalid files or ambiguous redirections.

This streamlined redirection system integrates seamlessly into the execution pipeline, enabling flexible and reliable data flow for commands.

# Built-in Commands: Core Functionality of the Shell

Built-in commands are directly implemented within the shell, allowing for immediate execution without spawning a separate process. They provide essential functionality and seamless interaction with the shell's environment.


## List of Built-in Commands

1. **`cd`**: Changes the current working directory.
2. **`echo`**: Prints text to the standard output with support for flags like `-n` to suppress newline.
3. **`env`**: Displays the current environment variables.
4. **`exit`**: Exits the shell with an optional exit code.
5. **`export`**: Adds or updates environment variables.
6. **`pwd`**: Displays the current working directory.
7. **`unset`**: Removes environment variables.


## Key Features of Built-in Commands

1. **Direct Execution**:
   - Built-ins are executed in the parent process to directly interact with the shell state, avoiding unnecessary forking.

2. **Environment Integration**:
   - Commands like `env`, `export`, and `unset` interact directly with the shell's environment variables.

3. **Error Handling**:
   - Comprehensive error messages for invalid input, unsupported flags, or environment-related issues.

4. **Efficient Argument Parsing**:
   - Arguments are validated and processed dynamically, ensuring flexible usage.

## Highlights of Each Built-in Command

### 1. `cd` (Change Directory)
- **Functionality**:
  - Changes the current directory based on the provided path or environment variables (`HOME`, `OLDPWD`).
- **Key Behaviors**:
  - Updates `PWD` and `OLDPWD` environment variables.
  - Handles errors like "too many arguments" or invalid paths.

### 2. `echo` (Print Text)
- **Functionality**:
  - Prints text to the standard output.
- **Key Behaviors**:
  - Supports the `-n` flag to suppress the trailing newline.
  - Handles quoted strings and escape sequences (e.g., `\n`, `\t`).

### 3. `env` (Environment Variables)
- **Functionality**:
  - Lists all current environment variables.
- **Key Behaviors**:
  - Prints variables in a key-value format.

### 4. `exit` (Exit Shell)
- **Functionality**:
  - Exits the shell with an optional exit code.
- **Key Behaviors**:
  - Validates the exit code argument for numeric input.
  - Handles errors like "too many arguments" or invalid exit codes.

### 5. `export` (Add/Update Environment Variables)
- **Functionality**:
  - Adds new environment variables or updates existing ones.
- **Key Behaviors**:
  - Validates variable names and formats.
  - Adds variables to the shell's environment dynamically.

### 6. `pwd` (Print Working Directory)
- **Functionality**:
  - Displays the current working directory.
- **Key Behaviors**:
  - Resolves and prints the absolute path.

### 7. `unset` (Remove Environment Variables)
- **Functionality**:
  - Removes environment variables.
- **Key Behaviors**:
  - Validates variable names before removal.
  - Ensures no invalid identifiers are processed.

## Error Handling

- Invalid input or unsupported arguments are met with descriptive error messages.
- Specific edge cases like missing environment variables (`HOME`, `OLDPWD`) for `cd` or invalid identifiers for `unset/export` are managed gracefully.

Built-in commands form the core functionality of this shell, providing efficient and integrated features for interacting with the shell environment and managing basic tasks.

## Interesting outputs: 
- cat << EOF | wc > out
- cat << EOF << EOF << EOF <Makefile | wc > out
- ls > a > b > c

ls -lah | wc > out.txt
<out.txt cat | wc -l

## Conclusions

Working on Minishell has been a fun and challenging experience for us. It’s been a chance to really dig into how command-line tools work and turn that knowledge into something functional. Of course, there’s still plenty of room for improvement and a ton of features we could add, but that’s part of the excitement—there’s always something more to build on. Whether it becomes the base for something bigger or just stays as a solid learning project, it’s been incredibly rewarding to collaborate and bring it to life.


