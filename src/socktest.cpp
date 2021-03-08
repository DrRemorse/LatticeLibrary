#ifdef TEST_SOCKETS
#include <library/log.hpp>
#include <library/network/tcpsocket.hpp>
#include <library/sleep.hpp>

#include <iostream>
#include <vector>

using namespace library;

#include <thread>
#include <mutex>

const int CLIENT_THREADS = 16;

typedef struct
{
	int  id;
	
} socket_clientdata_t;

std::vector <std::thread*> serverClients;
std::mutex  textMutex; // avoid console output mess

void serverClientError(Socket* socket, Socket::socket_stage_t stage, int error, std::string& errorString)
{
	textMutex.lock();
	std::cout << "SERVER CLIENT: " << socket->stageToString(stage) << std::endl;
	std::cout << "--> Error " << error << ": " << errorString << std::endl;
	textMutex.unlock();
}

void serverClientRead(TcpSocket* socket, std::string& text)
{
	textMutex.lock();
	logger << Log::INFO << "Server client: ";
	logger << "READ: " << text << " (bytes: " << text.length() << ")" << Log::ENDL;
	textMutex.unlock();
	// disconnect immediately
	//socket->disconnect();
}

void serverClientThread(TcpSocket* socket)
{
	socket->onError        = serverClientError;
	socket->onDataReceived = serverClientRead;
	
	while (socket->isConnected())
	{
		// poll for events
		socket->poll();
		// write something
		socket->writeLine("abcdefgh");
	}
	// cleanup
	delete socket;
}

void serverError(Socket* socket, Socket::socket_stage_t stage, int error, std::string& errorString)
{
	textMutex.lock();
	std::cout << "SERVER: " << socket->stageToString(stage) << std::endl;
	std::cout << "--> Error " << error << ": " << errorString << std::endl;
	textMutex.unlock();
}

void serverListening(TcpSocket* socket, std::string& addr, int port)
{
	textMutex.lock();
	std::cout << "SERVER: Listening for clients on " << addr << ":" << port << std::endl;
	textMutex.unlock();
}

void serverAccept(TcpSocket* socket, int port, TcpSocket* newClient)
{
	// new client accepted
	textMutex.lock();
	std::cout << "SERVER: New client from " << newClient->getPeerName() << " (" << newClient->getPeerAddr() << ":" << newClient->getPeerPort() << ")" << std::endl;
	textMutex.unlock();
	
	// start new server clientsocket thread
	std::thread* newClientThread = new std::thread(serverClientThread, newClient);
	serverClients.push_back(newClientThread);
}

bool serverRetirement = false;

void serverThread(std::string hostname, int port)
{
	TcpSocket server(nullptr);
	
	server.onError      = serverError;
	server.onListening  = serverListening;
	server.onAccept     = serverAccept;
	
	// try to bind to interface:port a few times
	int retries = 5;
	while (retries--)
	{
		if (server.bind(hostname, port))
		{
			server.listen();
			break;
		}
		relinquishCPU();
	}
	
	// if we failed to start a listening socket, exit
	if (server.isListening() == false)
	{
		textMutex.lock();
		std::cout << "Server failed to bind to interface " << hostname << ":" << port << std::endl;
		textMutex.unlock();
		return;
	}
	
	// success
	textMutex.lock();
	std::cout << "Server started..." << std::endl;
	textMutex.unlock();
	
	while (server.isListening())
	{
		// poll for events
		server.poll();
		// relinquish cpu
		relinquishCPU();
		
		if (serverRetirement) break;
	}
	server.disconnect();
	std::cout << "Server exiting..." << std::endl;
}

void clientConnect(TcpSocket* socket, std::string& hostname, int port)
{
	socket_clientdata_t* data = (socket_clientdata_t*) socket->getUserData();
	
	textMutex.lock();
	std::cout << "Client " << data->id << ": ";
	std::cout << "Connecting to " << hostname << ":" << port << "..." << std::endl;
	textMutex.unlock();
}

void clientConnected(TcpSocket* socket, std::string& hostname, int port)
{
	socket_clientdata_t* data = (socket_clientdata_t*) socket->getUserData();
	
	textMutex.lock();
	std::cout << "Client " << data->id << ": ";
	std::cout << "Connected to " << socket->getPeerName() << " (" << socket->getPeerAddr() << ":" << socket->getPeerPort() << ")" << std::endl;
	textMutex.unlock();
}

void clientError(Socket* socket, Socket::socket_stage_t stage, int error, std::string& errorString)
{
	socket_clientdata_t* data = (socket_clientdata_t*) socket->getUserData();
	
	textMutex.lock();
	std::cout << "Client " << data->id << ": " << socket->stageToString(stage) << std::endl;
	std::cout << "--> Error " << error << ": " << errorString << std::endl;
	textMutex.unlock();
}

void clientRead(TcpSocket* sock, std::string& text)
{
	socket_clientdata_t* data = (socket_clientdata_t*) sock->getUserData();
	
	textMutex.lock();
	logger << Log::INFO << "Client " << data->id << ": ";
	logger << "READ: " << text << " (bytes: " << text.length() << ", Rx=" << sock->getRx() << ")" << Log::ENDL;
	textMutex.unlock();
	
	// reply back text
	sock->writeLine(text);
	
	// disconnect immediately
	sock->disconnect();
}

void clientDisconnected(TcpSocket* socket)
{
	socket_clientdata_t* data = (socket_clientdata_t*) socket->getUserData();
	
	textMutex.lock();
	std::cout << "Client " << data->id << ": ";
	std::cout << "Socket was disconnected successfully" << std::endl;
	textMutex.unlock();
}

void clientThread(int id, std::string hostname, int port)
{
	socket_clientdata_t data;
	data.id = id;
	
	TcpSocket sock(&data);
	
	sock.onConnect    = clientConnect;
	sock.onConnected  = clientConnected;
	sock.onDataReceived = clientRead;
	sock.onError      = clientError;
	sock.onDisconnect = clientDisconnected;
	
	// connect N times
	const int CONNECTS = 3;
	
	for (int i = 0; i < CONNECTS; i++)
	{
		sock.connect(hostname, port, HostInfo::HOSTNAME);
		
		while (sock.isConnected())
		{
			// poll for events
			sock.poll();
			// send stuff
			//sock.writeLine("ABCDEFGH");
			
			relinquishCPU();
		}
		
		sock.disconnect();
	}
	
}

void test_sockets()
{
	std::cout << "Testing resolver..." << std::endl;
	std::string hname = "www.vg.no";
	
	std::cout << "Resolving " << hname << "..." << std::endl;
	// resolve hostname
	std::string ip   = Socket::resolve(hname, Socket::HOSTNAME);
	// reverse lookup
	std::string host = Socket::resolve(ip, Socket::ADDRESS);
	
	std::cout << hname << " resolved to " << ip << std::endl;
	std::cout << ip << " reverse is " << host << std::endl;
	
	std::cout << "Starting server..." << std::endl;
	// enable outputting to console
	logger.setOutputConsole(true);
	
	std::string serverHost = "localhost";
	int serverPort = 13102;
	
	// create server thread
	std::thread server(serverThread, "127.0.0.1", serverPort);
	// relinquish to hopefully make server start
	relinquishCPU();
	
	std::cout << "Starting threaded clients..." << std::endl;
	
	// create client threads
	std::thread clients[CLIENT_THREADS];
	
	for (int i = 0; i < CLIENT_THREADS; i++)
	{
		clients[i] = std::thread(clientThread, i + 1, serverHost, serverPort);
	}
	
	// wait for clients
	for (int i = 0; i < CLIENT_THREADS; i++) clients[i].join();
	// wait for server
	serverRetirement = true;
	server.join();
}
#endif
