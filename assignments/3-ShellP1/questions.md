1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  It's a good choice cause it reads line by line until there is a newline or EOF character.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  I needed to do that because it's good for resizing and also in the case that memory overflows on the stack.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  It's necessary because we don't want different cases for the same command but one has a trailing or leading whitespace so it would not exectute.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: One example is output redirection (`command > output`). The challenge for this is need to have the correct permissions to open and write to files. Another example is input redirection (`command < input`). The challenge for this is to have permissions to read the file and that it exists. Another example is error redirection (`command 2> file`). The challenge for this is to handle any type of file error and write it to the error log.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: Redirection connects command input/output to files and pipes connect the output of a command to the input of another command.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It's important to keep these separate so regular output can be redirection, but errors won't accidentally be redirected along with that.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**: Our custom shell should provide a way to merge by using something like `2>&1` which combines the error and output.
