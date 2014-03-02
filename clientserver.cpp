#include "clientserver.hpp"

///////////////////////////////////////////////////////////////////////////
//
//          Functional Description Of Classes 
//
///////////////////////////////////////////////////////////////////////////

//v0.6 boost::mutex cMutex;

//
//Class TCPBase
//

TCPBase::TCPBase(){}

void TCPBase::setup(){
	setPort();
//	test(port);
	timeleft.tv_sec = 1;
	timeleft.tv_usec = 0;
	//inRun = true;
	setAddr();
}

TCPBase::~TCPBase(){
	stop();
}

void TCPBase::stop(){
	if(!inRun) return;
	close(socFD);
	inRun = false;
}

bool TCPBase::isConnected(){
	return inRun;
}

void TCPBase::setPort(){
	port = 4023;
}

bool TCPBase::send(std::string message){
	if(message == "")
		return false;
//	test(connSocFD);
	messg_len = write(connSocFD, message.c_str(), 1024);
	if(messg_len < 0) return false;
	return true;
}

std::string TCPBase::breceive(){
	if(!isConnected()){
		return "";
	}
//	test();
	clearBuff();
	messg_len = read(connSocFD, messg, 1024);
	if(messg_len <= 0){
		stop();
		return "";
	}
	return (std::string) messg;
}

std::string TCPBase::receive(){
	fd_set fd4read;
	FD_ZERO(&fd4read);
	FD_SET(connSocFD,&fd4read);
	if(select(connSocFD + 1, &fd4read, NULL, NULL, &timeleft) <= 0 || !isConnected()) return "";
	return breceive();
}

void TCPBase::clearBuff(){
	for(int i = 0; i < int(sizeof(messg)); i++)
		messg[i] = 0;
}

//
//class TCPClient
//

TCPClient::TCPClient(std::string IPaddress, std::string clientType) : TCPBase(){
	IP = IPaddress.c_str();
	type = clientType;
}

bool TCPClient::start(){
	setup();
	if(!create()) return false;
	return true;
}

bool TCPClient::create(){
	socFD = socket(AF_INET, SOCK_STREAM, 0);
//	test(port);
	if(socFD < 0) return false;
	while(connect(socFD, (sockaddr*) &addr, sizeof(addr)) != 0){
		sleep(1);
	}
	connSocFD = socFD;
	inRun = true;
	return true;
}

void TCPClient::setAddr(){
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(IP, &addr.sin_addr);
}

//
//class TCPServer
//

TCPServer::TCPServer(std::string serverType) : TCPBase(){
	type = serverType;
	set = false;
}

bool TCPServer::start(){
	setup();
	if(!create()) return false;
	takeClient();
	return true;
}

bool TCPServer::create(){
	socFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socFD < 0) return false;
	if(bind(socFD, (sockaddr*) &addr, sizeof(addr)) < 0) return false;
	if(listen(socFD, 5) < 0) return false;
	set = true;
	inRun = true;
	return true;
}

void TCPServer::setAddr(){
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
}

void TCPServer::takeClient(){
	socklen_t some = sizeof(addr);
	connSocFD = accept(socFD, (sockaddr*) &addr, &some);
//	test(connSocFD);
}

//
//class Client
//

Client::Client(std::string IPaddress, std::string clientType) : TCPClient(IPaddress, clientType){}

bool Client::start(){
	setup();
	if(!create()) return false;
	// connecting
	std::string setupMsg;
	// step 1 <<
	setupMsg = breceive();
	if(setupMsg != "OK"){
		std::cout<<"Server: "<<setupMsg<<std::endl;
		return false;
	}
	// step 2 >>
	send(type);
	// step 3 <<
	setupMsg = breceive();
//	test("| "+setupMsg+" |");
	int touchPort = atoi(setupMsg.c_str());
	if(touchPort == -1)
		return false;
//	test(touchPort);
	addr.sin_port = htons(touchPort);
	stop();
	inRun = true;
	if(!create()) return false;
	// connected
//	test("haha");
	return true;
}

///////////////////////////////////////////////////////////////////////////////////

//
//void* run_ClientListener_thread(void*)
//

void* run_ClientListener_thread(void* object){
	((ClientListener*) object)->start();
	return 0;
}

//
//class ClientListener
//

ClientListener::ClientListener(int clientID, std::vector<ClientListener>* friendList) : TCPServer(""){
	id = clientID;
	setFriendList(friendList);
}

ClientListener::ClientListener(int clientID) : TCPServer(""){
	id = clientID;
}

void ClientListener::setFriendList(std::vector<ClientListener>* friendList){	
	friends = friendList;
}

bool ClientListener::start(){
	if(!create()) return false;
	takeClient();
	wasConnected = true;
//	test(connSocFD);
//	send("hello");
	std::cout<<"\e[1;32m[]<-\e[0m One more client has been connected to "<<port<<" port."<<std::endl;
	while(isConnected()){
		receive();
	}
	return true;
}

bool ClientListener::disconnected(){
	if(wasConnected)
		return !isConnected();
	else
		return false;
		
}

void ClientListener::setPort(){
//	test("begin");
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int socFDs = socket(AF_INET, SOCK_STREAM, 0);
	if(socFDs >= 0)
		for(int listenPort = 42310; listenPort < 60000; listenPort++){
//			test(listenPort);
			saddr.sin_port = htons(listenPort);
			if(bind(socFDs, (sockaddr*) &saddr, sizeof(saddr)) < 0){
//				test();
				continue;
			}
			else{
				close(socFDs);
				port = listenPort;
//				test("end");
				return;
			}
		}
//	test(socFDs);
	port = -1;
//	test("end 1");
}

int ClientListener::getPort(){
	return port;
}

int ClientListener::getID(){
	//test(connSocFD);
	return id;
}

void ClientListener::receive(){
	std::string msg = breceive();
	if(msg != "/exit"){
		for(int i = 0; i < int(friends->size()); i++){	// problem on friends->size()
			if(getID() != (*friends)[i].getID()){
				//test((*friends)[i].getID());
				(*friends)[i].send(msg);
			}
		}
	}
}

//
//void* zombieKiller_thread(void*)
//

void* zombieKiller_thread(void* object){
	((ClientGroup*) object)->zombieThreadCollector();
	return 0;
}

//
//class ClientGroup
//

ClientGroup::ClientGroup(std::string groupType){
	type = groupType;
//	boost::thread garbCThread;
//	garbCThread = boost::thread(&ClientGroup::zombieThreadCollector, this);    // needs to reference of this
}

void ClientGroup::zombieThreadCollector(){
	while(1){
		sleep(1);
//		test(clients.size());
		for(int i = 0; i < int(clients.size()); i++){
//	 		test(clients[i].isConnected());
			if(clients[i].disconnected()){
				//v0.6 boost::mutex::scoped_lock lockClients(cMutex);
				pthread_mutex_lock(&cMutex);
				int disPort = clients[i].getPort();
				clients.erase(clients.begin() + i);
				i--;
				pthread_mutex_unlock(&cMutex);
				std::cout<<"\e[1;31m[]->\e[0m One client has been disconnected from "<<disPort<<" port."<<std::endl;
			}
		}

	}
}

int ClientGroup::getCount(){
	return clients.size();
}

std::string ClientGroup::getType(){
	return type;
}

std::string ClientGroup::creatSlot(){
	//v0.6 boost::thread newThread;
	pthread_t newThread;
	int newID;
	if(clients.size() != 0){
		newID = clients.back().getID() + 1;
	}
	else{
		newID = 0;
	}
	ClientListener newOne(newID, &clients);
	clients.push_back(newOne);
//	clients.back().setFriendList(&clients);
	clients.back().setup();
	//v0.6 newThread = boost::thread(boost::bind(&ClientListener::start, &clients.back()));
	pthread_create(&newThread, NULL, &run_ClientListener_thread, &clients.back());
	std::ostringstream newPort;
	newPort<<clients.back().getPort();
	return newPort.str();
}

///////////////////////////////////////////////////////////////////////////////////

//
//void* Server_takeClient_thread(void*)
//

void* Server_takeClient_thread(void* object){
	((Server*) object)->takeClient();
	return 0;
}

//
//class Server
//

Server::Server(std::string serverType, int maxClients) : TCPServer(serverType) {
	slots = maxClients;
}

Server::Server(std::string serverType) : TCPServer(serverType) {
	slots = 10;
}

Server::Server(int maxClients) : TCPServer(NULL) {
	slots = maxClients;
}

bool Server::start(){
	setup();
	if(!create()) return false;
	//v0.6 threadForClientReceive = boost::thread(boost::bind(&Server::takeClient, this));
	pthread_create(&threadForClientReceiving, NULL, &Server_takeClient_thread, this);
	return true;
}

void Server::wait_to_end(){
	pthread_join(threadForClientReceiving, NULL);
}

void Server::takeClient(){
	socklen_t some = sizeof(addr);
	bool notAssigned;
	std::string msg;
	std::string clientType;
	ClientGroup* clientGroup;
	while(1){
		connSocFD = accept(socFD, (sockaddr*) &addr, &some);
		// receiving
		int mn = 0;
		for(int i = 0; i < int(groups.size()); i++){
			if(groups[i].getCount() != 0)
				mn += groups[i].getCount();
			else{
				groups.erase(groups.begin() + i);
				i--;
			}
		}
		// step 1 >>
		if(mn >= slots){
			send("Sorry. Server is full, please try later.");
			//close(connSocFD);				// maybe need
			continue;
		}
		send("OK");
		// step 2 <<
		clientType = breceive();
		// step 3 >>
		if((type.c_str() != NULL)&&(clientType != type)){
			send("-1");
			//close(connSocFD);				// maybe need
			continue;
		}
		notAssigned = true;
		for(int i = 0; i < int(groups.size()); i++){
			if(clientType == groups[i].getType()){
				clientGroup = &groups[i];
				notAssigned = false;
				break;
			}
		}
		if(notAssigned){
			ClientGroup newOne(clientType);
			groups.push_back(newOne);
			clientGroup = &groups[groups.size() - 1];
			//v0.6 boost::thread garbCThread;
			pthread_t zombieCThread;
			//v0.6 garbCThread = boost::thread(boost::bind(&ClientGroup::zombieThreadCollector, clientGroup));
			pthread_create(&zombieCThread, NULL, &zombieKiller_thread, &clientGroup);
		}
		msg = clientGroup->creatSlot();
		//test("rM = " + msg);
		send(msg);
		//close(connSocFD);					// maybe need
		// received
	}
}
Server::~Server(){
	std::cout<<"Server has stopped."<<std::endl;
}
//...
/////////////////////////////////////// END OF FILE ////////////////////////////////////////
