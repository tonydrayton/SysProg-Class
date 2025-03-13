1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client determines when a command's output is fully received from the server by checking for the EOF character at the end of each chunk.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A network shell protocol should define and detect the beginning and end of a command sent over a TCP connection by checking EOF characters and null bytes. If this is not handled correctly then messages could be fragmented or the receiver could wait indefinitely for messages when it has ended already.

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols maintain information about the session across requests. Stateless protocols treat each request independently with no memory.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

We would use UDP because it has lower latency than TCP.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The socket interface/abstraction is provided by the operating system to enable applications to use network communications.
