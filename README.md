# Socket-Programming

## Problem
The server and client are two different programs that are run in different directories.

The client will create a connection to the server and send requests to download files to the client directory. It will write the files into its own directory. Progress is printed while downloading the files.

## Run
- In the directory where server.c is present, compile and then run as follows:
`gcc server.c -o server`
- `./server`

- In the directory where client.c is present, compile and then run as follows:
`gcc client.c -o client`.
- `./client`

- On the client side, prompt `client>` is seen:
	- `client>​ get <fileName>`
	The file if exists at the server side is downloaded in the client directory else appropriate error is displayed.

	- `client> ​ get <file1> <file2> <file3>`
	Multiple files requested are downloaded in the client directory.

	- `client> ​ exit`
	Connection with the server is closed.`
