#!/bin/python3

import argparse
import hashlib
import os
import socket
import struct
import yaml

# This is quick hack to get relative imports of a higher file working 
if __package__ is None:
    import sys
    sys.path.append(
        os.path.dirname(os.path.dirname( os.path.abspath(__file__)))
    )

from shared import *

class Server(object):
    def __init__(self, ip: str, port: int):
        """
        Constructor for custom Server object

        ip(str): IP address for remote server
        port(int): Port number of remote server
        """
        self.ip = ip
        self.port = port

    def send_to_server(self, request: bytes) -> bytes:
        """
        Function to send a message to the defined server

        request(bytes): The message to send

        This function assumes the server is already listening at the address 
        specified by the constructor. The provided request is sent to the 
        server and a response is always expected, even in the case of errors.
        The response message data is always returned. In the case of multiple 
        blocks of response, all blocks are combined into a single return value.
        """
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as conn_socket:
            # Connect and send message. Note that a try/except statement like
            # This is very difficult to implement in C, so a different style is
            # need
            try:
                conn_socket.connect((self.ip, self.port))
            except ConnectionRefusedError:
                print("Connection was refused. Is a server running at "
                      f"{self.ip}:{self.port}?")
                exit()
            conn_socket.sendall(request)

            responding = True
            all_blocks = None

            # Note loop here for processing messages of several blocks. 
            # Students may wish to have two seperate implementations, one for 
            # blocked messages and one for message that can be entirely read 
            # within a single block. That is acceptable, but they should be 
            # encouraged to solve it propperly (e.g. one function for both)
            while responding:
                # Read just enough bytes to get the length of this response
                inital_read = conn_socket.recv(LEN_RESPONSE_LENGTH)
                response_length = struct.unpack('!I', inital_read)[0]

                # Now read the expected number of bytes
                response_bytes = conn_socket.recv(
                    response_length+LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT
                    +LEN_BLOCK_HASH+LEN_TOTAL_HASH)

                # Parse the individual message attributes
                status = struct.unpack('!I', response_bytes[
                    0
                    :LEN_STATUS
                ])[0]
                this_block = struct.unpack('!I', response_bytes[
                    LEN_STATUS
                    :LEN_STATUS+LEN_BLOCK_ID
                ])[0]
                total_blocks = struct.unpack('!I', response_bytes[
                    LEN_STATUS+LEN_BLOCK_ID
                    :LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT
                ])[0]
                block_checksum = response_bytes[
                    LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT
                    :LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT+LEN_BLOCK_HASH
                ]
                total_checksum = response_bytes[
                    LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT+LEN_BLOCK_HASH
                    :LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT+LEN_BLOCK_HASH
                        +LEN_TOTAL_HASH
                ]
                response = response_bytes[
                    LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT+LEN_BLOCK_HASH
                        +LEN_TOTAL_HASH
                    :LEN_STATUS+LEN_BLOCK_ID+LEN_BLOCKS_COUNT+LEN_BLOCK_HASH
                        +LEN_TOTAL_HASH+response_length
                ]

                # Setup container for all blocks to be assembled together
                if all_blocks == None:
                    all_blocks = [None] * total_blocks

                # Report any unexpected feedback
                if status != STATUS_OK:
                    msg = f"Got unexpected status code: {status}"
                    print(msg)
                    all_blocks[this_block] = msg
                    return None

                # Report hash inconsistencies
                if get_sha256(response) != block_checksum:
                    msg = f"Block {this_block}/{total_blocks} checksums for " \
                        "a block did not match"
                    print(msg)
                    all_blocks[this_block] = msg
                    responding = False
                    return None

                # Add this response to the all_blocks container
                all_blocks[this_block] = response

                # Report how many more blocks we are expecting
                count = len(all_blocks) - all_blocks.count(None)
                print(f"block: {this_block} ({count}/{len(all_blocks)})")

                # If we have all the blocks, then finish the loop
                if all_blocks.count(None) == 0:
                    responding = False
            
            # Check hash of all the data 
            all_data = b"".join(all_blocks)
            if get_sha256(all_data) != total_checksum:
                print("Total checksum for all data did not match")

            return all_data

def to_server(payload: bytes, configs: dict):
    """
    Function to send a message to a server

    payload(bytes): The message to send
    config(dict): Configuration options defining server location

    The payload is sent to the server, with any response either printed or
    written directly to file depending on the request. No value is returned.
    """
    # Connect to the remote server
    server = Server(configs['server_ip'], configs['server_port'])

    data = server.send_to_server(payload)

    # If we did not recieve any data then something has gone wrong. 
    if not data:
        print(f"Could not retrieve data from server")
        return

    # Where, if anywhere any output should be written
    local_path = payload[LEN_USERNAME+LEN_SIGNATURE+LEN_REQUEST_LENGTH:] \
        .decode('utf-8')

    # If we are not expecting any data files to be written, then print the
    # output of the request
    if not local_path or local_path[-1] == os.path.sep:
        print(f"Got response: {data.decode('utf-8')}")

    # Data files are written straight to file rather than printing
    else:
        with open(local_path, "wb") as binary_file:
            binary_file.write(data) 

        print(f'Retrieved data written to {local_path}')

    return


def assemble_payload(username: str, password: str, salt: str, request=""):
    """
    Funtion to create a message out of the provided arguments.
    
    username(str): The name of the user connecting
    password(str): The user-remembered password
    salt(str): A string to apply to the user-remembered password
    
    The password and salt are combined and then hashed according to sha256. A 
    bytearray of the necessary parts is assembled and returned. 
    """    
    salted = f"{password}{salt}"
    signature = hashlib.sha256()
    signature.update(str.encode(salted))

    payload = bytearray()
    payload.extend(bytes(username[:LEN_USERNAME].ljust(LEN_USERNAME, '\x00'), 
        'utf-8'))
    payload.extend(signature.digest())
    payload.extend(struct.pack('!I', len(request)))
    payload.extend(bytes(request, "utf-8"))

    return payload
  

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "config",
        help="Path to the client config file, e.g.: path/to/file")
    parser.add_argument(
        "mode",
        nargs='?',
        default="user",
        help="Flag for if this client runs with user interaction (default), " \
             "or as a linear script for automated testing")
    args = parser.parse_args()

    # Read configuration options from file
    with open(args.config, 'r') as yaml_file:
        configs = yaml.load(yaml_file, Loader=yaml.Loader)

    # Validate mode argument
    if args.mode not in ["user", "test"]:
        print("Invalid argument given for mode. Valid are 'user' or 'mode'")
        exit()

    # If we are in the standard user mode, then expect to get some user interaction
    if args.mode == "user":

        username = input("Enter username: ")
        password = input("Enter password: ")
        
        # Note that the Python client always uses a hard coded salt. The C 
        # implementation may initially do the same, but a task is to generate this 
        # at runtime and save appropriately.
        salt = "0123456789012345678901234567890123456789012345678901234567890123"

        # Register our new user
        to_server(
            assemble_payload(username, password, salt), 
            configs,
        )

        while 1:
            print("Type the name of a file to be retrieved, or 'quit' to quit:")    
            user_input = input()
            if user_input == 'quit':
                exit()
            else:
                to_server(
                    assemble_payload(username, password, salt, user_input),
                    configs
                )

    # If we are not in user mode, then just run as a linear script
    else:
        username = "test_user"
        password = "test_password"
        salt = "0123456789012345678901234567890123456789012345678901234567890123"

        # Register our new user
        to_server(
            assemble_payload(username, password, salt), 
            configs,
        )
        
        # Retrieve the smaller file
        to_server(
            assemble_payload(username, password, salt, 'tiny.txt'),
            configs
        )

        # Retrieve the larger file that will be broken into blocks
        to_server(
            assemble_payload(username, password, salt, 'hamlet.txt'),
            configs
        )