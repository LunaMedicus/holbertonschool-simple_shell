# Simple Shell

`simple_shell` is a minimal UNIX command interpreter developed as a Holberton
School project. It reads commands from standard input, executes builtins or
programs found in the current `PATH`, and repeats until the user exits.

## Features

- Interactive prompt mode
- Non-interactive mode (command input via pipe/file)
- Executes commands using `fork` + `execve`
- Searches executable files in `PATH`
- Handles command separators by line (`\n`)
- Builtins:
  - `exit`
  - `env`

## Compilation

Compile with `gcc` using the required flags:

```sh
gcc -Wall -Werror -Wextra -pedantic *.c -o hsh
```

## Usage

### Interactive mode

```sh
./hsh
($) /bin/ls
($) exit
```

### Non-interactive mode

```sh
echo "/bin/ls" | ./hsh
```

## Manual page

After adding the manual file, display it with:

```sh
man ./man_1_simple_shell
```

## Authors

See the [`AUTHORS`](./AUTHORS) file at the repository root.
