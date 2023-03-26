# TecnicoFS - Simplified File System Library


This project was developed for the Operating Systems course during the 2nd semester of the academic year 2021/2022. The project is divided into two folders tecnicofs and tecnicofs_ex2, corresponding respectively to the first and second phases of the project.


# Part1

This project provides a base implementation of a simplified file system called "TecnicoFS". The system is implemented as a library that can be used by any client process that wants to have a private instance of a file system in which it can store its data.

TecnicoFS provides a programming interface (API) inspired by the POSIX file system API. However, to simplify the project, the TecnicoFS API offers only a subset of functions with a simplified interface. These functions include tfs_open, tfs_close, tfs_write, tfs_read, tfs_init, and tfs_destroy.

The system state of TecnicoFS is referenced in a main data structure called an "inode table," which represents a directory or a file in TecnicoFS. Each inode has a unique identifier called an "inode number," which corresponds to the index of the corresponding inode in the inode table. The inode contains a data structure that describes the attributes of the directory or file, as well as a reference to the content of the directory or file.

In addition to the inode table, there is a fixed-size block-organized data region that maintains the data of all files in the file system. This data is referenced from the inode of each file (in the inode table).

TecnicoFS maintains an open file table that keeps track of the files currently open by the client process of TecnicoFS and, for each open file, indicates the current position of the file pointer.

Simplifications of the project include having only one directory ("root") with files and no subdirectories, limiting the content of files and the root directory to a single block, assuming there is only one client process that can access the file system, and there are no permissions or access control.

# Part2

This second part aims to add two new features to the original file system:

Blocking Destruction Function: A new function int tfs_destroy_after_all_closed() is added, which is a variant of the function tfs_destroy(). This function waits until there are no open files, and only then it deactivates TécnicoFS. Any concurrent or subsequent calls to tfs_open() after tfs_destroy_after_all_closed() has deactivated TécnicoFS should return an error (-1).

Client-Server Architecture: TécnicoFS is transformed into an autonomous server process, launched using the command tfs_server name_of_pipe. Upon creation, the server creates a named pipe whose name (pathname) is the one indicated in the argument above. It is through this pipe that client processes can connect to the server and send operation requests.

API Documentation
The following operations allow the client to establish and terminate a session with the server:

* int tfs_mount(char const *client_pipe_path, char const *server_pipe_path)
Establishes a session using the named pipes indicated in the arguments. The client's named pipe must be created (by calling mkfifo()) with the name passed in the first argument. The server's named pipe must have been previously created by the server, with the name passed in the second argument. On success, the session_id associated with the new session will have been set to a value between 0 and S-1, where S is a constant defined in the server's code.

* int tfs_unmount()
Terminates the current session.

* int tfs_create(char *filename)
Creates a file with the given name.

* int tfs_delete(char *filename)
Deletes the file with the given name.

* int tfs_open(char *filename)
Opens the file with the given name.

* int tfs_close(int fd)
Closes the file with the given file descriptor.

* int tfs_write(int fd, void *buf, size_t len)
Writes len bytes from the buffer pointed to by buf to the file referred to by the file descriptor fd.

* int tfs_read(int fd, void *buf, size_t len)
Reads up to len bytes from the file referred to by the file descriptor fd into the buffer pointed to by buf.

* int tfs_seek(int fd, off_t offset, int whence)
Changes the file offset of the file referred to by the file descriptor fd. The new offset is calculated as described in the lseek() man page.

*  int tfs_rename(char *oldname, char *newname)
Renames the file with the given old name to the new name.



## Build Part2



Building and Running
To build TécnicoFS, run the following command:


`make all`


To run TécnicoFS, use the following command:

`./tfs_server name_of_pipe`

To run the provided tests, use the following command:

`./test.sh`

# Installation
To use the TecnicoFS library, download the source code and compile it using the provided Makefile. 

tfs_init: initializes the file system.
tfs_destroy: frees the resources used by the file system.
tfs_open: opens a file in the file system.
tfs_close: closes a file in the file system.
tfs_read: reads data from a file in the file system.
tfs_write: writes data to a file in the file system.
Please see the documentation in the tecnicoFS.h header file for detailed information about these functions.

