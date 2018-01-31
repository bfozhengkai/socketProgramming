/*
patient.cpp
University of Southern California
EE-450 Project_Assignment_2
Kaisi Zheng
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

using namespace std;

#define PORT1 "40460"
#define serverPort "30460"
#define MAXSIZE 1024
#define doc1 "doctor1 aaaaa"
#define doc2 "doctor2 bbbbb"
#define ACK "OK!"
#define IP "127.0.1.1"

// struct patientInfo {
//     string patientname;
//     string portNum;
//     string IPaddr;
//     string docNum;
// };

void doctorReadData (int);
void docPhase2();
void docPhase3 (vector<string>tcpdata , int doctornum, int p);
string toString1(int c);
void removeSpace (string &str);
string toString2(char[]);
unsigned short toShort(string report);
vector<string> docReadTXT(int);


int main (int argc, char ** argv) {
	docPhase2();
	return 0;
}

void docPhase2 () {
	int  doctornum =1;
	int p;
	
	while (doctornum <3) {
		//Phase 2:

		srand ((unsigned)time(0)+p);
		if ((p=fork())==0) {
			int udpsock,tcpsock;
			char hostname[128];
			char healthcenterBuf[MAXSIZE];
			struct addrinfo udpdocclient, *udpdocclientP, server_addr,*server_addrPointer;
			struct sockaddr_in local_addr;
			struct  sockaddr_in server_addrInfo;
			socklen_t local_len = sizeof (local_addr);

			memset (&udpdocclient, 0,sizeof udpdocclient);
			memset (&server_addr,0, sizeof server_addr);
			gethostname (hostname, sizeof hostname);

			udpdocclient.ai_family = AF_UNSPEC;
			udpdocclient.ai_socktype = SOCK_DGRAM;
			udpdocclient.ai_flags = AI_PASSIVE;

			getaddrinfo(hostname, toString1(atoi(PORT1+doctornum*1000)).c_str(), &udpdocclient, &udpdocclientP);
			getaddrinfo(hostname, serverPort, &server_addr, &server_addrPointer);

			 if((udpsock=socket(udpdocclientP->ai_family, udpdocclientP->ai_socktype, udpdocclientP->ai_protocol))<0){
			        cout<<"Doctor can not create socket."<<endl;
			        exit(1);
			       }

			 if(bind(udpsock,udpdocclientP->ai_addr,udpdocclientP->ai_addrlen)<0){
			        cout<<"Doctor can not bind the port."<<endl;
			        exit(1);
			       }
			 //Send the password.
			const char* sendPassword;
			if (doctornum==1) {
			 	 sendPassword = doc1;
			}
			else {
			 	 sendPassword = doc2;
			}
			 if (sendto(udpsock, sendPassword, 1024, 0, server_addrPointer->ai_addr, server_addrPointer->ai_addrlen)<0) {
			 	cout<<"Doctor can not send the password to healthcenter."<<endl;
			 	exit(1);
			 }
			 socklen_t healthcenter_len = sizeof (server_addr);
			 if (recvfrom(udpsock, healthcenterBuf, MAXSIZE-1,0, (struct sockaddr*)&server_addr, 
			 	&healthcenter_len)<0)
			 {
			 	cout<<"Doctor can not receive the schedule."<<endl;
			 	exit(1);
			 }
			 else {	
			 	cout<<"doctor"<<doctornum<<" : "<<healthcenterBuf<<endl;
			 	string file = toString2(healthcenterBuf);
			 	ifstream  infile;
			 	infile.open(file.data());
			 	string str;
			 	vector <string> doctcp;
			 	while (getline(infile, str)) {		 	       
			 		if ((str[str.size()-1]-'0')==doctornum) {
			 			doctcp.push_back(str);
			 		}
			 	}
			 	infile.close();
			 	// for (vector<string>:: iterator i=doctcp.begin();i!=doctcp.end();i++) {
			 	// 	cout<<*i<<endl;
			 	// }
			 	//cout<<"Phase3 will begin here. "<<endl;

			 	//Phase 3.
			 	docPhase3(doctcp,doctornum, p);
			 }
			break;
		}
		else  {
			doctornum++;
		}
	}
	return;
}


void docPhase3 (vector<string> tcpdata , int doctornum, int p) {
	srand ((unsigned)time(0)+p);
 	int client_sockfd;
	struct sockaddr_in server_addr;
	int temp = tcpdata.size();
    string patInfo= "";
    char buf[MAXSIZE];
    char recvBuf[MAXSIZE];
	if (!tcpdata.empty()) {
		string remotePort = tcpdata[0];
		tcpdata.erase(tcpdata.begin());
		remotePort = remotePort.substr(19, 5);
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(IP);
		server_addr.sin_port = htons(toShort(remotePort));

		if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			cout << "Can not create a TCP socket." << endl;
			exit(1);
		}

		if (connect(client_sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
			cout << "Can not connect the TCP client." << endl;
			exit(1);
		}
		//Send the number of patient.

		if (temp==1) {
			patInfo+="Doctor";
			patInfo+=toString1(doctornum);
			patInfo+=" ";
			patInfo+="has ";
			patInfo+="only one patient subscriber.";
		}

		if (temp>1) {
			patInfo+="Doctor";
			patInfo+=toString1(doctornum);
			patInfo+=" ";
			patInfo+="has ";
			patInfo+=toString1(temp);
			patInfo+=" patients";
		}
		cout<<patInfo<<endl;

		string sendnumber = toString1(temp);
		const char* buf = sendnumber.c_str();
		send(client_sockfd, buf, strlen(buf), 0);
		memset (recvBuf, 0,MAXSIZE );
		if (recv(client_sockfd, recvBuf, MAXSIZE, 0)< 0) {
			cout << "Can not receive the ACK." << endl;
			exit(1);
		}
		cout << "doctor"<<doctornum<<": "<< recvBuf << endl;


		//send welcome information.
		patInfo = "welcome to doctor";
		patInfo +=toString1(doctornum);
		patInfo+=" group";
		const char* welbuf = patInfo.c_str();
		send(client_sockfd, welbuf, strlen(welbuf), 0);
		memset(recvBuf,0,MAXSIZE);
		if (recv(client_sockfd, recvBuf, MAXSIZE, 0)<0) {
			cout << "Can not receive the ACK." << endl;
			exit(1);
		}
		cout << "doctor"<<doctornum<<": "<< recvBuf << endl;

		//send the patient left after select a patient.
		if (!tcpdata.empty()) {
			for (int i=0;i<tcpdata.size();i++){
				string sendMessage = "";
				sendMessage = "";
				string temp = tcpdata[i];
				sendMessage += temp.substr(0,8);
				sendMessage += " ";
				sendMessage += temp.substr(19, 5);
				const char *message = sendMessage.c_str();
				if (send(client_sockfd, message, strlen(message), 0) < 0) {
					cout << "Can not send the patient information." << endl;
					exit(1);
				}
				memset (recvBuf, 0, MAXSIZE);
				if (recv(client_sockfd, recvBuf, MAXSIZE, 0)< 0) {
					cout << "Can not receive the ACK." << endl;
					exit(1);
				}
				cout << "doctor"<<doctornum<<": "<< recvBuf << endl;

			}
		}
		//Read file for doctor1.txt and doctor2.txt.
		vector<string> schedulevector = docReadTXT(doctornum);
		string schedule = "";
		int tempsize = schedulevector.size();
		for (int i=0;i<tempsize;i++) {
			schedule+= schedulevector[i].c_str();
			schedule=schedule.substr(0,schedule.size()-1);
		}
		//cout<<schedule<<endl;
		//Send schedule.
		schedule+="m";
		const char* scheMessage = schedule.c_str();
		if (send(client_sockfd, scheMessage, strlen(scheMessage), 0) < 0) {
			cout << "Can not send the patient information." << endl;
			exit(1);
		}
		memset (recvBuf, 0, MAXSIZE);
		if (recv(client_sockfd, recvBuf, MAXSIZE, 0)< 0) {
				cout << "Can not receive the ACK." << endl;
				exit(1);
			}
			cout<<"doctor"<<doctornum<<": "<<recvBuf<<endl;
	}

	else
	{	
		cout<<"Doctor"<<doctornum<<" has no peer subscribers!"<<endl;
		close(client_sockfd);
		return;
	}
	return;
}

vector<string> docReadTXT (int doctornum) 
{	
	vector<string> res;
	
	if (doctornum==1) {
			ifstream  file;
			string str;
			file.open("doctor1.txt");
			char line1[1024] = {0};
		 	while (file.getline(line1, sizeof(line1))) {	
		 		res.push_back(line1);
		}
		file.close();
	}
	if (doctornum==2) {
			ifstream  file;
			string str;
			file.open("doctor2.txt"); 
			char line2[1024]={0};	
			while (file.getline(line2,sizeof(line2))){
				res.push_back(line2);
			}
		
		file.close();
	}
	return res;
}

string toString1(int c){
  stringstream stream;
  string result;
  stream << c; 
  stream >> result; 
  return result;
}

unsigned short toShort(string report) {
	stringstream stream;
	unsigned short result;
	stream<<report;
	stream>>result;
	return result;
}

void removeSpace(string &s)
{
    if( !s.empty() )
    {
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }

}

string toString2(char c[]){
  stringstream stream;
  string result;
  stream << c; 
  stream >> result; 
  return result;
}