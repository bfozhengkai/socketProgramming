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
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

#define PORT "30460"
#define MAXSIZE 1024
#define numOfPatient 4
#define numOfDoctor 2
#define ACK "OK!"
#define file "directory.txt"
#define IP "127.0.1.1"

struct patientInfo {
    string patientname;
    string portNum;
    string IPaddr;
    string docNum;
};

void removeSpace (string &str);
int verify (string input);
string toString(int a);
void writeData (string str) ;

int main(int argc, char ** argv)
{

//Phase 1:
    string udpPatientName;
    int udpHealthCenterPort;
    char hostName[128];
    int UDPsock, TCPsock;
    struct addrinfo server_addr, *server_addrP;

    struct sockaddr_storage patient_addr[numOfPatient];
    struct sockaddr_storage doctor_addr[numOfDoctor];
    char patBuffer[numOfPatient][MAXSIZE];
    char docBuffer[numOfPatient][MAXSIZE];

    struct patientInfo allPatientInfo[4];

    memset (&server_addr,0, sizeof server_addr);
    gethostname (hostName, sizeof hostName);

    server_addr.ai_family = AF_UNSPEC;
    server_addr.ai_socktype = SOCK_DGRAM;
    server_addr.ai_flags = AI_PASSIVE;

    getaddrinfo (hostName, PORT, &server_addr, &server_addrP);

    if((UDPsock=socket(server_addrP->ai_family, server_addrP->ai_socktype, server_addrP->ai_protocol))<0){
        cout<<"Health center establishes socket failed."<<endl;
        exit(1);
    }
    if(bind(UDPsock,server_addrP->ai_addr,server_addrP->ai_addrlen)<0){
        cout<<"Health center binds socket failed."<<endl;
        exit(1);
    }

    int indextOfPatient = 0;
    while (indextOfPatient<4) {

        socklen_t addrLen = sizeof patient_addr[indextOfPatient];
        if((recvfrom(UDPsock,patBuffer[indextOfPatient],MAXSIZE-1,0,(struct sockaddr *)&patient_addr[indextOfPatient],&addrLen))<0) {
            cout<<"Can not receive message from the client." << endl;
            exit(1);

        }
        else {
            cout<<"healthcenter : "<<patBuffer[indextOfPatient]<<endl;
            indextOfPatient++;
        }
    }
    ofstream fout("directory.txt");
    for(int i=0;i<4;i++){
        string str=patBuffer[i];
        fout<<str<<endl;
        cout<<str<<": Phase 1 : Registration is done successfully!"<<endl;
    }
    cout<<"Phase 2 Begin : Registration of patient completed! Run the doctors!"<<endl;

    //Phase2 :
    int indexOfDoc = 1;
    while (indexOfDoc<3) {
        socklen_t addr_len = sizeof (doctor_addr[indexOfDoc]);
        if (recvfrom(UDPsock,docBuffer[indexOfDoc],MAXSIZE-1,0,
                     (struct sockaddr *)&doctor_addr[indexOfDoc],&addr_len)<0)
        {
            cout<<"Can not receive the username and password from doctors."<<endl;
            exit(1);
        }
        else {
            cout<<"The password we are receiving is :"<<docBuffer[indexOfDoc]<<endl;
            int i = verify (docBuffer[indexOfDoc]);

            if (i!=0) {
                cout<<"doctor"<<i<<" :  Login successfully."<<endl;

                int t=sendto(UDPsock, file, 1024, 0,(struct sockaddr*)&doctor_addr[indexOfDoc], addr_len);
                if (t<0) {
                    cout<<"Healthcenter can not send the shedule."<<endl;
                    exit(1);
                }

            }
            indexOfDoc++;
        }
    }
    return 0;
}



//implements other function.



int verify(string input){
    ifstream in("healthcenter.txt");
    string s;
    removeSpace(input);//doctor1aaaaa
    //doctor2bbbbb
    while(!in.eof()){
        getline(in,s);
        removeSpace(s);
        string str=s.substr(0,s.size()-1);
        if(input.compare(str)==0 || input.compare(s)==0) {
            in.close();
            return input[6]-'0';
        }
    }
    in.close();
    return 0;
}

string to_string(int a){
    stringstream stream;
    string result;
    stream << a;
    stream >> result;
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