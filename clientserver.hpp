////////////////////////////////////////////////////////////////
//clientserver 0.7
////////////////////////////////////////////////////////////////

#ifndef CLIENTSERVER_HPP
#define CLIENTSERVER_HPP

//-----
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
//-----
//-----
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//-----
//-----
#include <pthread.h>
//#include <boost/thread/thread.hpp>
//#include <boost/thread/mutex.hpp>
//-----

//---Final products:

//---Low  level connections.

// class TCPClient(string IPaddress, string type)
	  // start()
	  // stop()
	  // send(string message)
	  // string receive()
	  // string breceive()

// class TCPServer(string type)
	  // start()
	  // stop()
	  // send(string message)
	  // string receive()
	  // string breceive()

//---High level connections.

// class Client(string IPaddress, string type)
	  // start()
	  // stop()
	  // send(string message)
	  // string receive()
	  // string breceive()

// class Server(string type), (int maxClients), (string type, int maxClients)
	  // start()
	  // stop()
	  // wait_to_end()
	  
//////////////////////////////////////////////////////
// in v2.0 will be added new features:
// on Server class:
	  // kick(string clientID)
	  // closeGroup(string groupType)
	  // getListClientID();
	  // shutDown()
	  // receiveClient(bool receiverState)
//////////////////////////////////////////////////////

//Globals
//boost::mutex cMutex;
static pthread_mutex_t cMutex;
/////////

//Other components

class TCPBase{
public:
	~TCPBase();

	virtual bool start() = 0;
	void stop();
	bool isConnected();
	bool send(std::string message);
	std::string breceive();
	std::string receive();
protected:
	int port;
	int socFD;
	int connSocFD;
	int messg_len;
	char messg[1024];
	std::string type;
	bool inRun;
	timeval timeleft;
	sockaddr_in addr;
	
/////// Test /////////	
	void test(){
		std::cout<<"test"<<std::endl;
	}
	
	template<class T>
	void test(T value){
		std::cout<<value<<std::endl;
	}
//////////////////////

	TCPBase();
	virtual void setup();
	virtual bool create() = 0;
	virtual void setAddr() = 0;
	virtual void setPort();
	void clearBuff();
};

class TCPClient : public TCPBase {
public:
	TCPClient(std::string IPaddress, std::string clientType);
	bool start();
protected:
	const char* IP;
	
	bool create();
	void setAddr();
};

class TCPServer : public TCPBase {
public:
	TCPServer(std::string serverType);
	bool start();
protected:
	bool set;
	
	bool create();
	void setAddr();
	void takeClient();
};

class Client : public TCPClient {
public:
	Client(std::string IPaddress, std::string clientType);
	bool start();
protected:
	int connectionPort;
};

void* run_ClientListener_thread(void* object);

class ClientListener : protected TCPServer{
public:
	friend class ClientGroup;
	friend void* run_ClientListener_thread(void* object);
protected:
	int id;
	int wasConnected;
//	volatile int connSocFD;
//	boost::mutex cMutex;
	std::vector<ClientListener>* friends;
	
	ClientListener(int clientID, std::vector<ClientListener>* friendList);
	ClientListener(int clientID);
	void setFriendList(std::vector<ClientListener>* friendList);	
	bool start();
	bool disconnected();
//	bool send(std::string message);
//	void takeClient();
	void setPort();
	int getPort();
	int getID();
	void receive();
};

void* zombieKiller_thread(void* object);

class ClientGroup{
public:
	friend class Server;
	friend void* zombieKiller_thread(void* object);
protected:
	std::string type;
	std::vector<ClientListener> clients;
//	boost::mutex cMutex;
//	std::vector<volatile ClientListener> clients;
	
/////// Test /////////	
	void test(){
		std::cout<<"test"<<std::endl;
	}
	
	template<class T>
	void test(T value){
		std::cout<<value<<std::endl;
	}
//////////////////////
	
	ClientGroup(std::string groupType);
	int getCount();
	void zombieThreadCollector();
	std::string getType();
	std::string creatSlot();
};

void* Server_takeClient_thread(void* object);

class Server : protected TCPServer {
public:
	friend void* Server_takeClient_thread(void* object);

	Server(std::string serverType, int maxClients);
	Server(std::string serverType);
	Server(int maxClients);
	bool start();
	void wait_to_end();
	~Server();
protected:
	int slots;
	std::vector<ClientGroup> groups;
	//v0.6 boost::thread threadForClientReceive;
	pthread_t threadForClientReceiving;
	
	void takeClient();
};
#endif
