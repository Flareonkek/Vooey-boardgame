# Build the server. Note: The server will only run on unix-like systems,
# and that's fine because no sane person uses windows for servers anyway.
gcc source/server_main.c -o server
