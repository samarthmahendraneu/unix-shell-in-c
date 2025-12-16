# Unix Shell Implementation (C, POSIX)

A **fully functional Unix-style shell** implemented in C, supporting **process creation**, **I/O redirection**, **pipes**, **conditional execution**, **background jobs**, and **built-in commands**.


> Built as part of an advanced OS curriculum, but engineered to production-quality standards.

## Supported Features

**🚀 Command Execution**
- Fork–exec model (`fork`, `execve`)
- Argument vector construction (`argv`)
- Proper exit status handling

**🔁 Process Control**
- Foreground and background execution (`&`)
- Parent–child synchronization (`waitpid`)
- Correct propagation of exit codes

**🔀 Pipes**
- Arbitrary pipelines using `|`
- Proper file descriptor setup
- Concurrent execution of pipeline stages

**📂 I/O Redirection**
- Input (`<`)
- Output (`>`)
- Error (`2>`)
- Redirection combined with pipes and subshells

**🧠 Conditional Execution**
- Sequential commands (`;`)
- Logical AND / OR (`&&`, `||`)
- Exit-status–based control flow

**🧩 Subshells**
- Grouped execution using parentheses `( )`
- Scoped redirection and execution context

**🏗️ Built-in Commands**
- `cd` (directory changes within shell process)
- `pwd`
- `exit`

(Built-ins are implemented correctly without `exec`, since they must modify shell state.)

## Architecture Overview

```
    User Input
        |
        v
+--------------------+
|  Command Parser    |
|  - Tokenization    |
|  - AST / List      |
+---------+----------+
          |
          v
+--------------------+
| Execution Engine   |
|  - fork / exec     |
|  - pipes           |
|  - redirection     |
|  - conditionals    |
+---------+----------+
          |
          v
+--------------------+
|   Unix Kernel      |
+--------------------+
```

The shell separates **parsing** from **execution**, mirroring real shell design.

## Example Commands

```sh
ls -l | grep txt > files.txt
(cat files.txt && echo "done") || echo "failed"
echo hello &
(cd /tmp ; pwd)
```

All behave consistently with standard Unix shells.

## Technology Stack

- C (systems programming)
- POSIX system calls
- `fork`, `execve`, `waitpid`
- `pipe`, `dup2`
- `open`, `close`
- `chdir`, `getcwd`
- Manual memory management
