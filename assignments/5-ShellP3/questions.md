1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My shell implementation uses a loop in `execute_pipeline` that does `waitpid(pids[i], &status, 0)` for each child process. If I forgot to call `waitpid()` on all child processes, the shell would prompt for new input before the commands before finished.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

It's necessary to close unused pipe ends after calling `dup2()` because leaving them open would cause a file descriptor leak and maybe prevent the pipeline from stopping when a process tries to read from a pipe that still has open write ends.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

`cd` is implemented as a built-in rather than an external command because it needs to change the current directory of the shell process. If `cd` was implemented as a external process it would change the directory in its own forked process and leave the parent shell's directory unchanged.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would modify my implementation to allow an arbitrary number ofpiped commands while still handling memory allocation efficiently by replacing command_list_t with a dynamic array. I would need to keep reallocating the commands and account for possible failures due to errors.
