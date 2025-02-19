1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > We need fork because execvp would completely replace our shell program, but with fork we can create a copy of our shell that runs the command while keeping our original shell running.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > If the fork() system call fails, it returns -1 to the parent process. I handle this scenario like:
    ```c
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        continue;
    }
    ```
    > When fork() fails:
    > 1. I use perror() to print an error message that includes the string "fork"
    > 2. The continue statement restarts the loop so the user can input another command

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: execvp() searches for the command in the directories listed in the PATH environment variable. When I call execvp() in my code (like in `execvp(cmd.argv[0], cmd.argv)`), it looks through each directory in PATH until it finds an executable file with the name it wants.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: In my implementation I use `waitpid(pid, &status, 0)` in the parent process to wait for the child process to complete. This is important because without it, child processes would have finished executing but still take up system resources because their exit status hasn't been collected.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() extracts the 8-bit exit code (0-255) from the status value returned by wait/waitpid. This information is important for error handling and determining whether commands executed successfully because programs return 0 for success and other things for different types of failures.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: My build_cmd_buff() implementation handles quoted arguments using a boolean `in_quote` to track whether we're inside a quoted string or not. When it sees a quote, it:
    > 1. Removes the quote character using memmove()
    > 2. Changes `in_quote` to true
    > 3. Doesn't split the spaces while inside the quotes

    > This is necessary because shell commands might need to handle stuff that contains spaces, like filenames with spaces or text messages.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: The main changes I made to my parsing logic were:
    > - Changed from pipe-separated command parsing to single command argv-style parsing
    >- Added support for quoted arguments using `in_quote`
    > - Implemented a new command buffer structure (cmd_buff_t)
    > - Changed from separate exe/args strings to an argv array format

    > The unexpected challenges were:
    > - Handling edge cases with quotes and spaces
    > - Converting from the previous assignment's arg separation to the new argv format

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a way for the OS to notify processes about important events (like Ctrl+C) or tell them to do something. Unlike other IPC methods, signals are asynchronous and don't transfer data between processes.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGINT interrupts a program, SIGKILL forcefully terminates a process and can't be ignored, and SIGTERM asks a program to terminate gracefully

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  SIGSTOP pauses a process and cannot be caught or ignored because it's a special signal that the kernel enforces directly.
