#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
#include <time.h>
using namespace std;

void post(int socket, string fileName);//post method
void get(int socket, string fileName);//fet method
void execute(string command);
vector<string> split(const string &s, char delim);//vector in which we split
string clean_string(string& str,char delimiter=' ');
vector<string> &split(const string &s, char delim, vector<string> &elems);
void read_file_commands();

int main (int argc, char** argv)
{
    time_t startT,endT;
    time (&startT);
    int listenFd, portNo;
    bool loop = false;
    struct sockaddr_in svrAdd;
    struct hostent *server;

    if(argc < 2)
    {
        cout<<"Syntax : ./client <server name> <port>"<<endl;
        return 0;
    }

    if(argc == 3)
    {
        portNo = atoi(argv[2]);
    }
    else
    {
        portNo=80;
    }

//    string client_id="";
//    if(argc == 4)
//    {
//        string str(argv[3]);
//        client_id = str;
//    }
//    cout<<"client_id = "<<client_id<<endl;

    //create client skt
    listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(listenFd < 0)
    {
        cout << "Cannot open socket" << endl;
        return 0;
    }

    server = gethostbyname(argv[1]);

    if(server == NULL)
    {
        cout << "Server does not exist" << endl;
        return 0;
    }

    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;

    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

    svrAdd.sin_port = htons(portNo);

    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));

    if (checker < 0)
    {
        cout << "Cannot connect!" << endl;
        return 0;
    }
    else
    {
        cout << "Connected to server" << endl;
    }

    read_file_commands();

    close(listenFd);
    time (&endT);
    double duration= endT-startT;
    cout<<"duration = "<<duration<<endl;
    return 0;
}

void get(int socket, string fileName)
{

    int responseLength;
    char response[1024];
    if ((responseLength = recv(socket, response, sizeof(response), 0)) < 0)
        cout<<"receving response failed"<<endl;
    cout << "response is: " << response << endl;

    int found_ok=0;
    for(int i=0; i<sizeof(response); i++)
    {
        if((response[i]=='O') && (response[i+1]=='K') && i+1<sizeof(response))
        {
            found_ok=1;
            break;
        }
    }


    if(found_ok)
    {
        FILE* outFile = fopen(fileName.c_str(), "wb");
        char buffer[1024];
        int bufferLength;
        if ((bufferLength = recv(socket, buffer, sizeof(buffer), 0)) < 0)
            cout<<"receving data failed"<<endl;
        cout<<"Receving now... "<<endl;

        while (bufferLength > 0)
        {
            fwrite(buffer, 1, bufferLength, outFile);
            if ((bufferLength = recv(socket, buffer, sizeof(buffer), 0)) < 0)
            {
                cout<<"receving data failed"<<endl;
            }
            else
            {
//            cout<<"buffer = "<<buffer<<endl;
            }
        }
        fclose(outFile);
        cout<<"Receving finished"<<endl;
    }

}
void post(int socket, string fileName)
{
    char buffer2[1024];
    int bufferLength;
    if ((bufferLength = recv(socket, buffer2, sizeof(buffer2), 0)) < 0)
        cout<<"receving ok failed"<<endl;
    cout<<"Response from server is  "<<buffer2<<endl;

    int found_ok=0;
    for(int i=0; i<sizeof(buffer2); i++)
    {
        if((buffer2[i]=='o'||buffer2[i]=='O') && (buffer2[i+1]=='k'||buffer2[i+1]=='K') && i+1<sizeof(buffer2))
        {
            found_ok=1;
            break;
        }
    }


    int readLength;
    char buffer[1024];

    if(found_ok)
    {
        FILE *inFile = fopen(fileName.c_str(), "rb");
        if (inFile == NULL)  //file not found
        {
            cout<<"File not found to be sent."<<endl;
        }
        cout<<"Posting now... "<<endl;
        while (!feof(inFile))
        {
            readLength = fread(buffer, 1, 1024, inFile);
            if (send(socket, buffer, readLength, 0) != readLength)
                cout<<"sending data failed"<<endl;
        }
        fclose(inFile);
        cout<<"Posting finished. "<<endl;
    }
}

void read_file_commands()
{
    ifstream file_commands ("input_commands.txt");
    if (file_commands.is_open())
    {
        while (!file_commands.eof())
        {
            string line;
            getline(file_commands,line);
            if(line.empty()) return;
            execute(line);
        }
        file_commands.close();
    }
    else
    {
        cout << "Unable to open file" << endl;
    }
}



void execute(string command)
{
    string type, file_name,host_name,port_number;
    vector<string> splited = split(command, ' ');
    if(splited.size() > 0) type = splited[0];
    if(splited.size() > 1) file_name = splited[1];
    if(splited.size() > 2) host_name = splited[2];
    if(splited.size() > 3) port_number = splited[3];
    cout<<"--------------------------------------------------"<<endl;
    cout << "type: " << type << endl;
    cout << "file_name: " << file_name << endl;
    cout << "host_name: " << host_name << endl;
    cout << "port_number: " << port_number << endl;

    int listenFd, portNo;
    bool loop = false;
    struct sockaddr_in svrAdd;
    struct hostent *server;

    stringstream ss(port_number);
    ss >> portNo;

    //create client skt
    listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(listenFd < 0)
    {
        cout << "Cannot open socket" << endl;
        return;
    }

    server = gethostbyname(host_name.c_str());

    if(server == NULL)
    {
        cout << "Server does not exist" << endl;
        return;
    }

    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;

    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

    svrAdd.sin_port = htons(portNo);

    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));

    if (checker < 0)
    {
        cout << "Cannot connect!" << endl;
        return;
    }
    else
    {
        cout<<"Connected to the server."<<endl;

    }
    cout<<"sending command ...."<<endl;
    char* inputCommand=new char[command.length()+1];
    strcpy(inputCommand, command.c_str());
    unsigned int inputCommandLen;
    inputCommandLen = strlen(inputCommand);
    /* Send request to the server */
    if (send(listenFd, inputCommand, inputCommandLen, 0) != inputCommandLen)
    {
        cout << "Cannot send request to the server" << endl;
        return;
    }
    delete [] inputCommand;

    if (type.compare("GET")==0)
    {
        get(listenFd, file_name);
    }
    else if (type.compare("POST")==0)
    {
        post(listenFd, file_name);
    }
    close(listenFd);
}


vector<string> &split(const string &s, char delim, vector<string> &elems)
{
    stringstream ss;
    string item;
    string delim_string;
    ss << delim;
    ss >> delim_string;
    ss.clear();
    ss << s;
    string space=" ";
//    cout<<"delim_string = "<<delim_string<<endl;
    while (getline(ss, item, delim))
    {
        if(item.compare(delim_string)!=0 && item.compare(space)!=0)
        {
//            cout<<"delim_string = "<<delim_string<<endl;
//            cout<<"item = "<<item<<endl;
            elems.push_back(item);
        }
    }
    return elems;
}


vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}


string clean_string(string& str,char delimiter)
{
    string cleaned="";

    for(int i=0; i<str.length(); i++)
    {
        if(str[i]!=' ' && !(i+1<str.length()&&str[i]==delimiter &&str[i+1]==delimiter))
        {
            cleaned+=str[i];
        }
    }
    return cleaned;
}
