1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

> In my implementation, afrer closing all the pipe ends, I call a loop that iterates over each command in my command list and calls the waitpid() method for the process id of the child process of that command. In a situation where I forgot to call waitpid on all the child processes, the parent process could resume running before all the children processes have completed execution which could lead to a child process returns being ignored or zombied.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

> Closing pipes after calling dup2() helps the program/process identify the EOF, and in the case where that EOF indicator is missing, a reading process could hang waiting indefinitely for input. Also, for each open file descriptor, system resources are being used and may eventually lead to the program running out of file descriptors to assign to new processes.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

> Cd involves changing the working directory, and implementing it as an external command would lead to the directory being changed in a child process which will not propagate to the parent process so the working directory would stay as is even after the command.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

> In a situation where I want to accept an arbitrary number of piped commands, I would remove the max command and allocate my clist structure to store num_commands( = number of pipes  + 1) and then use a dynamic allocation method to allocate the space for the commands. The tradeoffs would be a larger opportunity for memory issues to come up while allocating memory for an arbitrary number of commands and potentially forgetting to free memory which could lead to insuffiennt memory if improperly implemented. 
