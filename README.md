# glusterfs-lock-demo

This repo produces a binary to reproduce the glusterfs issue [#4371](https://github.com/gluster/glusterfs/issues/4371).


## How to use

### Prerequisite
We need a linux box, with gcc installed. <br>
We need the glusterfs client, either by apt-get, yum install, or building it from source. <br>
We need a gluster volume, the brick server can be on the same host as the client, there is no special requirement on the volume setup, any volume is enough to reproduce this issue, default volume option and mount option is good. However, to make the `glfs_timer` block faster, it is recommended to set the volume option `network.frame-timeout` to a shorter value (like 120).

## Build
Clone this repo, in the repo root, run
```
make
```
The build artifact is in the "bin" folder, which is created by the build.

## Run
go to the "bin" folder, run:
```
./glusterfs-lock-demo <mountpoint>/[any_folder]>/<anyfile> 3
```
and type "Enter" several times following the prompt, until the terminal freezes. The frozen of the terminal is the indication that the issue is reproduced.

This testing program takes two mandatory positional arguments:
arg1: The folder and file basename for the file lock test
args: The number of thread pairs that will compete the file lock and trigger interruption

For example, if the volume is mounted at `/mnt`, and there is no reconfigured volume options regarding the client io threads, this command will trigger the issue:
```
./glusterfs-lock-demo /mnt/a 3
```

It will create 3 lock holder threads and lock waiter threads, each pair does the same thing. Take the locker-1 and waiter-1 for example, the follwing events happen in sequence: <p>
  * locker-1 locks the file /mnt/a-1 (granted) <br>
  * waiter-1 locks the file /mnt/a-1 (blocked) <br>
  * waiter-1 signaled by SIGINT <br>

(The file /mnt/a-1 is created by the program, if it doesn't exist.)

Two pairs of the competition and interruption will block the two epoll threads in the client daemon, after the `network.frame-timeout` timer expires, the thrid pair will block the timer thread.
