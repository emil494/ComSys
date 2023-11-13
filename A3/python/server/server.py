#!/bin/python3

import argparse
import os
import socketserver
import yaml
import struct
import math
import random

# This is quick hack to get relative imports of a higher file working 
if __package__ is None:
    import sys
    sys.path.append(
        os.path.dirname(os.path.dirname( os.path.abspath(__file__)))
    )

from shared import *

class CourseWorkServer(socketserver.ThreadingTCPServer):
    def __init__(self, configs: dict, 
            request_handler_class: socketserver.StreamRequestHandler):
        """
        Constructor for CourseWorkServer.

        configs(dict): Configuration dictionary of for the server. Must include
            entires for 'server_ip', and 'server_port'.
        request_handler_class(socketserver.StreamRequestHandler): A handler to
            reespond to any messages sent to this server.

        This server inherits from the socketserver.ThreadingTCPServer. Its only
        difference is adding a users dictionary.
        """
        super().__init__((configs['server_ip'], configs['server_port']), 
            request_handler_class)

        self.users = {}


class RequestHandler(socketserver.StreamRequestHandler):
    """
    Custom handler to handle any inbound messages. Any input message will 
    ALWAYS receive an appropriate response.
    """

    def handle(self):
        """
        Function to initially handle any requests received. 
        
        This will read a request, perform some curseory validation before 
        calling more specific handling functions. Nothing is returned.
        """
        try:
            # Read request
            bytes_message = self.request.recv(MSG_MAX)

            # Extract request attributes
            username = bytes_message[
                0
                :LEN_USERNAME
            ]
            signature = bytes_message[
                LEN_USERNAME
                :LEN_USERNAME+LEN_SIGNATURE
            ]
            request_length = struct.unpack('!I', bytes_message[
                LEN_USERNAME+LEN_SIGNATURE
                :LEN_USERNAME+LEN_SIGNATURE+LEN_REQUEST_LENGTH
            ])[0]
            request = bytes_message[
                LEN_USERNAME+LEN_SIGNATURE+LEN_REQUEST_LENGTH
                :LEN_USERNAME+LEN_SIGNATURE+LEN_REQUEST_LENGTH 
                + request_length
            ]

            username = username.decode('utf-8').strip('\x00')

            # Validate message length
            if len(request) != request_length:
                self.handle_error(
                    STATUS_MALFORMED,
                    "Malformed request. Length of request does not match "
                    "given length"
                )

            # Determine how to process request
            if request_length == 0:
                self._register_new_user(username, signature)
            else:
                self._handle_request(username, signature, request)
 
        # Always generate a response, this is the fallback for if all other
        # validation and handling fails. This is acceptable as a last resort,
        # but where possible more helpful feedback and responses should be 
        # generated.
        except Exception as e:
            self.handle_error(STATUS_OTHER, f"Something went wrong. {e}")

    def _handle_request(self, username:str, signature:bytes, request:bytes) \
            -> None:
        """
        Function to handle a 'get file' type request.

        username(str): The user conducting the request
        signature(str): The users client-generated signature
        request(bytes): The request message body

        The user credentials are checked against pre-registered users, and if
        accepted then a data file is read and sent back to the requestee. A 
        response is always generated, either the data file or an error message 
        explaining what went wrong.
        """

        # Check user credentials. Users should be preregistered before this
        # type of request
        if username not in self.server.users:
            self.handle_error(
                STATUS_USER_MISSING,
                f"Cannot serve non-registered user '{username}'")
            return
        if self.server.users[username][0] != get_sha256(
                str.encode(f"{signature}{self.server.users[username][1]}")):
            self.handle_error(
                STATUS_INVALID_LOGIN,
                f"Signature hashes do not match for {username}.")
            return

        get_path = request.decode("utf-8")

        # Process domain from top header line. If it starts with a path
        # separator, remove it. Without doing this the path will go to the root
        # directory of the server host, rather than the servers base folder.
        if get_path[0] == os.path.sep:
            get_path = get_path[1:]

        # Report a request for missing data
        if not os.path.exists(get_path):
            self.handle_error(
                STATUS_BAD_REQUEST,
                f"Requested content {get_path} does not exist")
            return

        data = ""

        # Report request for nonsense data
        if not os.path.isfile(get_path):
            self.handle_error(
                STATUS_BAD_REQUEST,
                f"Request URI {get_path} is not a file")
            return

        # Get file data as bytes
        with open(get_path) as requested_file:
            data = requested_file.read()
        if type(data) != bytes:
            data = bytes(data, "utf-8")

        # Send a response
        print(f'Sending requested data from {get_path}')
        self._build_and_send_responses(STATUS_OK, data)
        return

    def _register_new_user(self, username:str, signature: bytes) -> None:
        """
        Function to handle a 'register new user' type request.

        username(str): The username requesting to register
        signature(bytes): The users client-generated signature, which is 
            treated as the users password by the server
        
        The user is registered, unless a user by that name already exists. A 
        response is always generated, either the data file or an error message 
        explaining what went wrong.
        """
        # Validate new user
        if not username:
            self.handle_error(
                STATUS_BAD_REQUEST,
                "Cannot register empty username"
            )
            return

        if username in self.server.users:
            self.handle_error(
                STATUS_USER_EXISTS,
                f"Cannot register user under name '{username}', already "
                "exists"
            )
            return

        # Salt the signature before saving it, alongside the salt used.
        salt = get_random_salt()
        hash = get_sha256(str.encode(f"{signature}{salt}"))
        self.server.users[username] = (hash, salt)

        # Send a response
        msg = f"New user {username} registered."
        self._build_and_send_responses(STATUS_OK, bytes(msg, "utf-8"))
        print(msg)
            
        return

    def handle_error(self, status:int, msg_str: str) -> None:
        """
        Function to handle any errors that are encountered during request 
        handling and response processing. 
        
        status(int): A status code describing the error encountered.
        msg_str(str): A more descriptive response detailing exactly what went 
            wrong

        Will print a message to the server command line, and return an 
        appropriate response to the requesting client.        
        """
        print(msg_str)
 
        self._build_and_send_responses(status, bytes(msg_str, "utf-8"))

        return

    def _build_and_send_responses(self, status:int, to_send: bytes) \
            -> bytearray:
        """
        Function to build a response and send it.

        status(int): The response status code. Should reflect the content of
            the message itself
        to-send(bytes): The response message body.

        The provided attributes are assembled into a bytes message according to
        the protocol described in the handout. Various attributes such as the
        payload length are dynamically calculated. If the message is longer 
        than the set message limit, then the payload is broken into blocks and
        sent seperately until all blocks have been sent.
        """
        # Get chechsum of the total message data to send
        total_checksum = get_sha256(to_send)
        # Calculate how long the payload can be, as we have a set limit of how
        # many bytes can be sent, and a header that must be attatched to each
        # message.
        sendable_length = MSG_MAX-LEN_RESPONSE_LENGTH-LEN_STATUS-LEN_BLOCK_ID \
            -LEN_BLOCKS_COUNT-LEN_BLOCK_HASH-LEN_TOTAL_HASH

        blocks_count = math.ceil(len(to_send) / sendable_length)
        this_block = 0
        blocks = []

        # loop to determine all the blocks in the payload
        while len(to_send) > 0:

            this_payload = to_send[:sendable_length]
            blocks.append((this_block, this_payload))

            # Determine if more blocks to send
            to_send = to_send[sendable_length:]
            this_block = this_block + 1

        # Shuffle the blocks to reply in a random order
        random.shuffle(blocks)

        # loop to send one or more blocks of payload
        for block_num, block_payload in blocks:

            # Assemble an individual payload block
            payload = bytearray()
            payload.extend(struct.pack('!I', len(block_payload)))
            payload.extend(struct.pack('!I', status))
            payload.extend(struct.pack('!I', block_num))
            payload.extend(struct.pack('!I', blocks_count))
            payload.extend(get_sha256(block_payload))
            payload.extend(total_checksum)    
            payload.extend(block_payload)

            print(f"Sending reply {block_num+1}/{blocks_count} with payload "
                f"length of {len(block_payload)} bytes")

            # Send the block
            self.request.sendall(payload)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "config",
        help="Path to the server config file, e.g.: path/to/file")
    args = parser.parse_args()

    with open(args.config, 'r') as yaml_file:
        configs = yaml.load(yaml_file, Loader=yaml.Loader)
    
    # This check is not a formal requirement of the system but should let you 
    # know if you've started your server in the wrong place. Do feel free to 
    # change or remove this if you are wanting to use different data files
    for f in ["tiny.txt", "hamlet.txt"]:
        if not os.path.isfile(f): 
            print(f"Server can't find expected data file {f}. Make sure you " + 
                  "are starting the server with the files next to it in the " + 
                  "directory")
            exit(1)

    print(f"Starting server at: {configs['server_ip']}:"
        f"{configs['server_port']}")
    with CourseWorkServer(configs, RequestHandler) as coursework_server:
        try:
            coursework_server.serve_forever()
        finally:
            coursework_server.server_close()
