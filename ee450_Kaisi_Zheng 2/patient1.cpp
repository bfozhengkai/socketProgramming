
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

#define PORT1 "50460"
#define serverPort "30460"
#define MAXSIZE 1024
#define ACK "OK!"
#define IP "127.0.1.1"


void patientProcess (int, int);
string toString1(int c);
string toString2(char c[]);
unsigned short toShort(string report);
int main(int argc, char **argv)
{
    int patientnum = 1;
    int p;

    while (patientnum<5) {

        srand ((unsigned)time(0)+p);
        if ((p=fork())==0) {
            int UDPsock, TCPsock, TCPclientsock;
            char hostname[128];
            struct addrinfo udpclient,*udpclientPointer,tcpclient,*tcpclientPointer,server_addr,*server_addrPointer;
            struct sockaddr_in local_addr, client_addr;
            socklen_t local_len = sizeof (local_addr);
            

            int tcplen;

            memset (&udpclient, 0, sizeof udpclient);
            memset (&tcpclient, 0, sizeof tcpclient);
            memset (&server_addr,0,sizeof server_addr);
            gethostname(hostname, sizeof hostname);

            udpclient.ai_family = AF_UNSPEC;
            udpclient.ai_socktype = SOCK_DGRAM;
            udpclient.ai_flags = AI_PASSIVE;


            getaddrinfo(hostname, toString1(atoi(PORT1+patientnum*1000)).c_str(), &udpclient, &udpclientPointer);
            getaddrinfo(hostname, serverPort, &server_addr, &server_addrPointer);

            if((UDPsock=socket(udpclientPointer->ai_family, udpclientPointer->ai_socktype, udpclientPointer->ai_protocol))<0){
                cout<<"patient establishes UDP socket failed."<<endl;
                exit(1);
            }

            if(bind(UDPsock,udpclientPointer->ai_addr,udpclientPointer->ai_addrlen)<0){
                cout<<"patient UDP socket binds failed."<<endl;
                exit(1);
            }

            tcpclient.ai_family = AF_UNSPEC;
            tcpclient.ai_socktype = SOCK_STREAM;
            tcpclient.ai_flags = AI_PASSIVE;

            getaddrinfo(hostname, "0", &tcpclient, &tcpclientPointer);
            
            if((TCPsock=socket(AF_INET, SOCK_STREAM, 0))<0){
                cout<<"patient establishes TCP socket failed."<<endl;
                exit(1);
            }

            if(bind(TCPsock,tcpclientPointer->ai_addr,tcpclientPointer->ai_addrlen)<0){
                cout<<"patient TCP socket binds failed"<<endl;
                exit(1);
            }



            string doctornum = toString1(rand()%2+1);
            getsockname (TCPsock, (sockaddr *)&local_addr, &local_len);
            inet_pton (AF_UNSPEC, hostname, &(local_addr.sin_addr));
            string message="patient";
            message+=toString1(patientnum);
            message+=" ";
            message+=toString2(inet_ntoa(local_addr.sin_addr));
            message+=" ";
            message+=toString1(ntohs(local_addr.sin_port));
            message+=" ";
            message+=("doctor");
            message+= doctornum;



            const char *sendMessage=message.c_str();

            if(sendto(UDPsock,sendMessage,1024,0,server_addrPointer->ai_addr,server_addrPointer->ai_addrlen)<0){
                cout<<"patient sends message failed"<<endl;
                exit(1);
            }

            if (listen (TCPsock, 1)<0) {
                cout<<"Patient can not listen."<<endl;
                exit(1);

            }
            socklen_t client_addr_len = sizeof (local_addr);
            TCPclientsock = accept (TCPsock, (struct sockaddr*) &local_addr, &client_addr_len);
            if (TCPclientsock<0) {
                cout<<"Server can not accept the information. "<<endl;
                exit(1);
                }
            char buf[MAXSIZE];
            
            
            int bytes;
           //recv patient number.
            recv(TCPclientsock, buf, MAXSIZE, 0);

            cout<<"patient"<<patientnum<<": "<<buf<<endl;
            int patnum = atoi(buf);
            memset(buf,0,MAXSIZE);
            string ack = "OK!";
            //const char* ackbuf1 = ack.c_str();
            //if (send(TCPclientsock,ackbuf1,strlen(ackbuf1), 0)<0) {
            if (send(TCPclientsock,ACK,strlen(ACK), 0)<0) {
                cout <<"Can not receive the ACK."<<endl;
                exit(1);
            }

            //recv welcome.
            if (recv(TCPclientsock, buf, MAXSIZE, 0)<0){
                cout<<"Can not send the patient left"<<endl;
                exit(1);
            }

            cout<<"patient"<<patientnum<<": "<<buf<<endl;
            memset(buf,0,MAXSIZE);
            //const char* ackbuf2 = ack.c_str();
            //if (send(TCPclientsock,ackbuf2,strlen(ackbuf2), 0)<0) {
            if (send(TCPclientsock,ACK,strlen(ACK), 0)<0) {
                cout <<"Can not receive the ACK."<<endl;
                exit(1);
            }
            
            int patientnumleft = patnum-1;
            
            //recv patient left.
            if (patientnumleft>0){
                int i=0;
                vector<string> nextpatient;

                while (i<patientnumleft) {
                    memset(buf,0,MAXSIZE);
                    recv(TCPclientsock, buf, MAXSIZE, 0);
                    string temp = buf;
                    nextpatient.push_back(temp);
                    cout<<"patient"<<patientnum<<": "<<buf<<endl;
                    //const char* ackbuf3 = ack.c_str();
                    //if (send(TCPclientsock,ackbuf3,strlen(ackbuf3), 0)<0) {
                    if (send(TCPclientsock,ACK,strlen(ACK), 0)<0) {
                        cout <<"Can not receive the ACK."<<endl;
                        exit(1);
                   }
                    i++;
                }

                //recv schedule.
                vector<string> schedule;
                 
                memset(buf,0,MAXSIZE);
                recv(TCPclientsock, buf, MAXSIZE, 0);
                cout<<"patient"<<patientnum<<": "<<buf<<endl;
                string scheline = buf;

                istringstream ss(scheline);
                string inloopbuf;
                while (ss>>inloopbuf) {
                	//cout << "inloopbuf" << inloopbuf << endl;
                    schedule.push_back(inloopbuf);

                }
                //const char* ackbuf4 = ack.c_str();
               //if (send(TCPclientsock,ackbuf4, strlen(ackbuf4), 0)<0) {

             cout<<"Patient"<<patientnum<<" joined doctor"<<doctornum<<"!"<<endl;
               	if (send(TCPclientsock,ACK,strlen(ACK), 0)<0) {
                    cout <<"Can not receive the ACK."<<endl;
                    exit(1);
               }

                close(TCPclientsock);
                close (TCPsock);

                struct sockaddr_in next_sock, *next_sockP;
                 

                //Last part : Send the message to next patient.
                string tempport = nextpatient[0];
                //cout << "tempport" << tempport << endl;
                string portstr = tempport.substr(9, 5);
                //cout << "portstr" << portstr << endl;
                nextpatient.erase(nextpatient.begin());
               
                const char* port = portstr.c_str();

                memset(&next_sock,0,sizeof(next_sock));
                next_sock.sin_family = AF_INET;
                next_sock.sin_addr.s_addr = inet_addr(IP);
                next_sock.sin_port = htons(toShort(portstr));
               // getaddrinfo(hostname, port, &next_sock, &next_sockP);
                char recvBuf[MAXSIZE];

                 
                if ((TCPsock=socket(PF_INET, SOCK_STREAM, 0))<0) {
                    cout<<"Next patient socket failed to establishes the socket."<<endl;
                    exit(1);
                }

                if (connect(TCPsock,(struct sockaddr *)&next_sock, sizeof (struct sockaddr))<0) {
                    cout<<"Next patient TCP can not connect"<<endl;
                    exit (1);
                }

                int temp = nextpatient.size();
                temp++;

                //send the number 
                string sendnumber = toString1(temp);
                const char* numbuf = sendnumber.c_str();
                send(TCPsock, numbuf, strlen(numbuf), 0);
                if ((bytes = recv(TCPsock, recvBuf, MAXSIZE, 0))< 0) {
                    cout << "Can not receive the ACK." << endl;
                    exit(1);
                }
                recvBuf[bytes] = '\0';
                cout << "patient"<<patientnum<<": "<< recvBuf << endl;

                //send welcome information.
                string patInfo = "welcome to doctor";
                patInfo +=doctornum;
                patInfo+=" group";
                const char* welbuf1 = patInfo.c_str();
                send(TCPsock, welbuf1, strlen(welbuf1), 0);

                if ((bytes = recv(TCPsock, recvBuf, MAXSIZE, 0))<0) {
                    cout << "Can not receive the ACK." << endl;
                    exit(1);
                }
                recvBuf[bytes] = '\0';
                cout << "patient"<<patientnum<<": "<< recvBuf << endl;

                //send patient left.
                temp = temp-1;
                if (temp>0) {
                    int sizetemp = nextpatient.size();
                    for (int i=0;i<sizetemp;i++){
                        string sendMessage = "";
                        string strtemp = nextpatient[i];
                        sendMessage += strtemp.substr(0, 8);
                        sendMessage += " ";
                        sendMessage += strtemp.substr(9, 5);
                        const char *message = sendMessage.c_str();
                        if (send(TCPsock, message, strlen(message), 0) < 0) {
                            cout << "Can not send the patient information." << endl;
                            exit(1);
                        }
                        if ((bytes = recv(TCPsock, recvBuf, MAXSIZE, 0))< 0) {
                            cout << "Can not receive the ACK." << endl;
                            exit(1);
                        }
                        recvBuf[bytes] = '\0';
                        cout << "patient"<<patientnum<<": "<< recvBuf << endl;

                    }
                }
                //Send schedule.
                string scheMessage = schedule[2];
                int size = schedule.size();
                //cout <<"size" << size <<endl;
                for (int i=3;i<size;i++) {
                	//cout << "schedule i" << schedule[i] << endl;
                    scheMessage+=" ";
                    scheMessage+= schedule[i];
                }
                const char* scheMess = scheMessage.c_str();
                if (send(TCPsock, scheMess, strlen(scheMess), 0) < 0) {
                    cout << "Can not send the patient information." << endl;
                    exit(1);
                }
                if ((bytes = recv(TCPsock, recvBuf, MAXSIZE, 0))< 0) {
                        cout << "Can not receive the ACK." << endl;
                        exit(1);
                }
                recvBuf[bytes] = '\0';
                cout<<"patient"<<patientnum<<": "<<recvBuf<<endl;




           }

            else {
                
                vector<string> schedule;
                memset(buf,0,MAXSIZE);
                recv(TCPclientsock, buf, MAXSIZE, 0);
                cout<<"patient"<<patientnum<<": "<<buf<<endl;
                string scheline = buf;

                istringstream ss(scheline);
                string inloopbuf;
                while (ss>>inloopbuf) {
                	//cout << "else inloopbuf " << inloopbuf << endl;
                    schedule.push_back(inloopbuf);

                }
                //const char* ackbuf4 = ack.c_str();
               //if (send(TCPclientsock,ackbuf4, strlen(ackbuf4), 0)<0) {
               	if (send(TCPclientsock,ACK,strlen(ACK), 0)<0) {
                    cout <<"Can not receive the ACK."<<endl;
                    exit(1);
               }
               cout<<"Peer"<<patientnum<<" joined doctor"<<doctornum<<"!"<<endl;
            }

            close(TCPclientsock);
            close (TCPsock);
            break;
        }
        else {
            patientnum++;
        }
    }
    return 0;
}

//implement data transfer function.


string toString1(int c){
    stringstream stream;
    string result;
    stream << c;
    stream >> result;
    return result;
}
string toString2(char c[]){
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