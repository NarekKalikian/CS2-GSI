import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", 4444))

print("Python Listener Active on Port 4444...")

while True:
    data, addr = sock.recvfrom(1024)
    message = data.decode('utf-8')
    print(f'Received from C++:{message}')
