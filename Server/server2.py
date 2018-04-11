import socket
import os, struct
import subprocess
import hashlib
command = "/usr/bin/axis-remote tmpfile.ngc"

PTA_PACKET_NGC = 2
PTA_PACKET_SS = 4

class PTAPacket:
	def __init__(self):
		self.identifier = ''
		self.size = 0
		self.packetType = 0
		self.md5 = ''
		self.dataSize = 0
		self.data = ''

def recv_ptapacket(sock):
	packet = PTAPacket
	packet.identifier = sock.recv(4)

	sizeBuff = sock.recv(4)
	packet.size = struct.unpack('!I', sizeBuff)[0]

	typeBuff = sock.recv(4)
	packet.packetType = struct.unpack('!I', typeBuff)[0]

	packet.md5 = sock.recv(32)

	sizeBuff = sock.recv(4)
	packet.dataSize = struct.unpack('!I', sizeBuff)[0]

	packet.data = recvall(sock, packet.dataSize)

	return packet


def recv_packet(sock):
	hmsg = sock.recv(4)
	print(list(hmsg))
	print 'Received headermsg'
	if hmsg[0] != 'P' or hmsg[1] != 'T' or hmsg[2] != 'A': return None

	lenbuff = sock.recv(4)
	print 'Received lenbuff: ' + str(len(lenbuff))
	length = struct.unpack('!I', lenbuff)[0]
	print 'received packet header size: ' + str(length)
	return recvall(sock, length)

def recvall(sock, count):
	buf = ''
	while count:
		newbuf = sock.recv(count)
		if not newbuf: break
		buf += newbuf
		count -= len(newbuf)
		print 'Received ' + str(len(newbuf)) + ' bytes'

	return buf

HOST = ''
PORT = 9992
ADDR = (HOST, PORT)
BUFFSIZE = 4096

serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

serv.bind(ADDR)
serv.listen(5)

print 'listening ...'
okMessage = "ok"
failMessage = "fail"

while True:
	conn, addr = serv.accept()
	print 'client connected ...', addr

	packet = recv_ptapacket(conn)

	dataMd5 = hashlib.md5(packet.data).hexdigest()

	print packet.data

	print dataMd5
	print packet.md5

	conn.close()

	if os.path.isfile('tmpfile.ngc'):
		os.remove('tmpfile.ngc')
	outputFile = open('tmpfile.ngc', 'wb')

	outputFile.write(packet.data)
	outputFile.close()


	process = subprocess.Popen(command.split())

