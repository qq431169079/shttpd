## sHTTPD 1.0.3 Snapshot
###Description
shttpd is simple http non-blocking web server which uses tcp sockets and select(). Software is written for educational purposes and is distributed in the hope that it will be useful for anyone interested in this field.

###Usage
```
Usage: shttpd [-a <addr>] [-p <port>] [-t <number>] [-v] [-h]
options are:
 -a <addr>       # address of server
 -p <port>       # listening port of server
 -t <number>     # threads proces of server
 -h              # prints version and usage
```
Each parameters without argument will be parsed from config file

###TODO
- add threads
- add child process
- add sgi script execution function

###Get more info from my blog
- Offensice-Security Georgia: [http://off-sec.com/](http://off-sec.com)
