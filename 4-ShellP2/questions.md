1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: Because the fork function is what duplicates the parent process, but execvp is what creates a new memory context for the child, that is seperate from that of the parent, and runs the needed program in this new child context.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If the fork() syscall fails, that means that the child process wasn't successfully created. In my implementation, a failed fork() results in printing an error message and returning an error code.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  When calling execvp(), the first argument passed to the function is the command to be run and when it receives that, it checks all the directories in the $PATH variable till an executable file with the same name as the first argument is found.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  We have the wait() function in the parent process because the parent process would close before the child process finishes execution and so the child process could potentially get zombied and the parent process terminates without knowing the child process exit code, whether error or not.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() returns the exit code returned by the child process when the child exits normally. This code can signal success or error depending on what the code is, and this is important for determining how the parent process will pick up execution after the wait after an error or a success.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: My implementation parses the input command line character by character and is constantly checking for a quotation mark - to indicate the pointer is in a quoted argumemy- and for the spaces which seperate the arguments in the command. This is necessary because parsing the command just by white space alone, the way I did it in the last assignment, will lead to quoted arguments being broken up in to multiple arguments rather than being considered a single argument. This could cause errors executing the commands because the executable file for the arguments cannot be found by the program. This could also lead to a valid command being rejected by the program for having too many arguments because one argument was tokenozed and cause the argumemt limit to be hit. Generally, quoted arguments need to be handled as a single entity and this is very important for implementation and execution by the program.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  My parsing logic had to change to one-by-one character inspection, contrary to the logic implemented in my last assignment where I was able to use string methods like strtok to break up the input by white space. Using string methods would result in the quoted arguments being broken up into multiple arguments most of the time. Also, this time I had to include logic to avoid skipping duplicate spaces when they were in quoted arguments unlike the last assignment where for each whitespace terminated token, I could clean up any leading white spaces in all case. The major challenge was moving from dependence on string methods to manually checking input strings one-by-one which required more time and effort to implement. 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a way for a Linux system to notify a process that an event or action has occured. A primary difference between a signal and other IPC, such as pipes, is that signals perform notification functionality rather than data exchange for processing like other IPCs.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGCHLD - SIGCHLD is a signal sent when a child process changes state, whether it terminates, crashes, pauses, resumes, to the parent process. It's typically used by forked child processes which allows the parent process to keep working on what it needs to without being blocked by a wait call.
    >              SIGQUIT - SIGQUIT is a signal sent when a user requests that a program quit and do a core dump. It's typically used by a user to force quit a process by hitting the Ctrl and '\' keys.
    >              SIGILL - SIGILL is a signal sent when a program attemts to execute a command that's illegal, malformed, or not permitted. It's typically used when a program has a bug which leads to compiled code that has some wrong commands.


- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: When a process rceives SIGSTOP, it cannot catch or ignore the signal and so must terminate without any cleanup, which is unlike SIGINT which can be intercepted by a signal handler. This is important for execution because it ensures the operating system still can still control the system even during process execution as need be, in case of anything that could happen.