# AFLDFF
AFLDFF (American Fuzzy Lop Distributed Fuzzing Framework) is a tool designed to make to process of fuzzing with AFL across multiple machines easier. It features a retro curses interface for manageing machines and keeping track of test cases and crashes.

![afldff](https://cloud.githubusercontent.com/assets/1786880/12220160/854cd6c8-b731-11e5-9331-89716de0cfb0.png)

##Usage
```
afldff [ options ]
  -i ip       - IP address to listen on defaults to 0.0.0.0 if left blank
  -p port     - Port to listen on
  -m tar      - Patch afl to be network compatable
  -h          - Print help screen
```

##installation
``` 
$ make
# make install
```

##Prepairing AFL
Although AFL is opensource, the code is owned by google. I am only hosting the patches to make AFL network compatable. You can download the source for AFL, written by lcamtuf, at http://lcamtuf.coredump.cx/afl/ . The AFLDFF includes a useful "-m" flag to make the process of patching afl easier. All you have to do is point it at the afl tar file.

```
$ afldff -m [path-to-afl-tar]
```
