This is telnet proxy between a FICS client and FICS.

The proxy is necessary for myself and many others because FICS admins
have been very liberal in banning players who have broken the rules.

Thanks to CMU course 15-213 for providing some starter code for the proxy.

To start the proxy, simply type "./tiny 5000" from CLI. That should start
the proxy listening on port 5000.

To have the client connect to the proxy, have the client connect to the
IP address that your proxy is on, and to port 5000 (if that is the port
you chose to have the proxy listen to).

If you are hosting the proxy on an Amazon EC2 Instance, be sure to have
port 5000 open for TCP connections in the relevant Security Group.

Thanks for reading. If any troubles, don't hesitate to email me at
aguo@andrew.cmu.edu.

Thanks again. Have fun playing chess!
