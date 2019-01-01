Programs are very large and bloated. They need a healthy dose of dieting to get back into shape.

`diet` will execute programs in a constrained environment by using rlimit to set the maximum memory size and maximum number of open files.

```
$ diet --verbose /bin/ls
Current address space limit:
  soft: 17179869184
  hard: 17179869184
address space limit set to 1073741824
Current data segment limit:
  soft: 18446744073709551615
  hard: 18446744073709551615
data segment limit set to 1073741824
Current open file limit:
  soft: 1024
  hard: 1048576
open file limit set to 512
diet  fix  Makefile  README.md	src
```

Some predefined diet plans are available with `--large`, `--medium`, `--small`, `--starving`. If your program can run in the medium diet then it is doing pretty well. If your program can run in the small diet then it is a lean mean machine.

```
$ diet --help

diet: run programs with reduced resources (via rlimit)
diet <option>... <program> <program-arg> ...
 --large: set limits to 1GB address space, 1GB data segment, 512 open files. This is the default if no option is specified.
 --medium: set limits to 512MB address space, 512MB data segment, 256 open files
 --small: set limits to 128MB address space, 128MB data segment, 64 open files
 --starving: set limits to 16MB address space, 16MB data segment, 16 open files
 --memory <size>: set the memory limit to <size>
 --data <size>: set the data segment limit to <size>
 --files <size>: set the number of open files limit to <size>
 --verbose: print what the limits are being set to
 --help: show this help
<size> is an integer optionally followed a unit, such as 2k or 3m. 2kb is also ok

Example:
$ diet --medium /bin/ls -l
```
