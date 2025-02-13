1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  'fgets()' works for this application because it captures the user's input up to a specific string length which we have decided as the maximum length an acceptable command to our application should be. It also handles newline characters and EOF pretty well so the application executes the command sequentially as entered and expected by the user rather than writing extra logic into our code to check for indicators in the user's input for the end and start of comamads. It also has an awesome capability which is it appends a null-termination character to the string read in which would help later while programming to prevent reading past the memory allocated or looking through all the allocated memory in a situation where the user's input does not occupy the whole memory, all to make processing more efficient. Generally, these features make the command capture process very effective by handling extra steps that would have needed to be taken to ease the process of processing of the user's input.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: Allocating a fixed-size array would lead to memory for cmd_buff being allocated on the stack. Allocating memory on the stack could lead to stack overflows, say if SH_CMD_MAX is changed to a much larger value, or it could also allow for the command read from the user's input to be overwritten when the block of code is done executing, so returning a pointer to cmd_buff to execute the command stored there may be returning a pointer to something unplanned if that memory in the stack has been overwritten. On the other hand, malloc allocates space for cmd_buff on the heap and allows the contents to persist after execution of the block to be used later as necessary.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: Without trimming spaces, the spaces are valid characters and would be counted towards the length of the command; if it's a leading space, it could count towards the executable length, and if it's a trailing whitespace, it could count towards the arguments length. This could potentially lead to violation of the space for the maximum executable/argument.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  1. > - cmd > file
    > Using '>' for redirection of stdout from displaying on the terminal to being written, and possibly overwriting current contents, to a file. 
    > 2.>> - cmd >> file
    > Using '>>' for redirection of stdout from displaying on the terminal to being appended to a file. 
    >The challenges that may come up primarily deal with giving write privoleges that 
    > 3.2> - cmd 2> file
    > Using '2>' for redirection of stderr from displaying on the terminal to being being written, and possibly overwriting current contents, to a file.
    > When it comes to implementing the redirection functionality as above, challenges that could arise would relate to managing the file descriptor for processes, and ensuring that after the redirection of output or errors are done, we restore the stdout and stderr to what it initially was. Another challenge would be handling forking so that the redirection execution does not happen on the parent/main process, but rather on a child process and this is achieved through forking to prevent permanent redirection in the situation that a command that kills the parent process is run or that we forget to redirect the subsequent processes to the original output. 

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: Redirection involves rerouting the input or output(whether valid or an error) of a command to a file, while piping involves rerouting the output of a command to become the input of another command.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  Because it helps differentiate what stream would hold valid output from which stream would hold error messages, which can aid diagnosis of any issues in the program. 
- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  If a command fails, the shell should output an error code describing what the issue was, and prompt the user for another input. If  a command ooutputs both STDOUT and STDERR, I don't think they should be merged to avoid confusion. We want to make sure all error diagnosis can be conducted with ease, and keeping them seperate helps avoid any confusion.