1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used 
to handle partial reads or ensure complete message transmission?

> The client looks for the EOF character, typically 0x04, to indicate the end of the stream which indicates that the server is done sending over information. In the situation of a partial reads,checking and waiting for an EOF character will ensure that what's been read is a full read and not partial. Another technique could be predeterming a received buffer length and checking if the full buffer has been filled and if it is, possibly the data has been truncated and so ignoring the last received data and asking for another data packet to be sent. If the user prefers not to set a predetermined buffer length, the server could first send the length of the data packet and then the actual message so the client knows what to expect.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

> To define the start and end of the stream, the shell protocol has to determine what will be the delimiter that breaks up a stream into messages. The shell may also decide to capture commands/messages of a certain predetermined length and keep receiving streams but in the chunks. 

3. Describe the general differences between stateful and stateless protocols.

> While a stateful protocol keeps track of the state of a connection between a client and connection and so treats each connection as part of a broader sequence, a stateless protocol does not track close information of the state of the client-server connection and so treats communications as independent and unrelated. In the case of no response from a server, a stateful protocol will resend the request whereas a stateless protocol doesn't necessarily resend unanswered requests.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

> UDP is still used because it has much lower overhead than TCP and prioritizes speed and efficiency over assured delivery so is acceptable for operations where data loss may not be detrimental. UDP prioritizes efficiency over reliability and so when messages are sent, they are shipped off and then forgotten about immediately.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

> The operating system provides the socket API to facilitate sending and receiving of data over networks.