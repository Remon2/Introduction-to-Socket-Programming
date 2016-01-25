#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <vector>
#include <sstream>

using namespace std;
void get(int clientSocket, string fileName);
void post(int clientSocket, string fileName);
vector<string> split(const string &s, char delim);
string clean_string(string& str,char delimiter=' ');
vector<string> &split(const string &s, char delim, vector<string> &elems);
void getHtmlForRealClient(string file_name);
time_t startT,endT;
struct PARAMS
{
    int clientSocket;
};

void *task(void *);

int main(int argc, char* argv[])
{
    int pId, portNo, listenFd;
    socklen_t len; //for host addr length
    bool loop = false;
    int MAX_NO_OF_CONNECTIONS=300;
    struct sockaddr_in svrAdd, clntAdd;

    if (argc < 2)
    {
        cerr << "Syntam : ./server <port>" << endl;
        return 0;
    }

    portNo = atoi(argv[1]);


    //create socket
    listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }

    bzero((char*) &svrAdd, sizeof(svrAdd));

    svrAdd.sin_family = AF_INET;
    svrAdd.sin_addr.s_addr = INADDR_ANY;
    svrAdd.sin_port = htons(portNo);

    //bind socket
    if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0)
    {
        cerr << "Cannot bind" << endl;
        return 0;
    }

time (&startT);
    while (true)
    {

        pthread_t thread;
        cout << "Listening" << endl;
        if(listen(listenFd, MAX_NO_OF_CONNECTIONS)<0)
        {
            cerr << "Cannot Listen" << endl;
            return 0;
        }

        len = sizeof(clntAdd);

        //this is where client connects. svr will hang in this mode until client conn
        PARAMS params;
        params.clientSocket = accept(listenFd, (struct sockaddr *)&clntAdd, &len);

        if (params.clientSocket < 0)
        {
            cerr << "Cannot accept connection" << endl;
            return 0;
        }
        else
        {
            cout << "Successful Connection with client "<< endl;//<<clntAdd.sin_addr<< endl;
        }

        pthread_create(&thread, NULL, task, &params);

    }
}

void *task (void *dummyPt)
{
    cout<<"--------------------------------------------------------------------------------"<<endl;

    PARAMS* params = (PARAMS*)dummyPt;
    cout << "Thread No: " << pthread_self() << endl;
    char buffer[1024];
    if ((recv(params->clientSocket, buffer, 1024, 0)) < 0)
        cerr<<"receiving failed in begin of task"<<endl;
    //cout<<"buffer = "<<buffer<<endl;
    string command(buffer);
    cout<<"command = "<<command<<endl;
    //split the message and find the method and the fileName

    if(command.size() < 2)
    {
        time (&endT);
    double duration= endT-startT;
    cout<<"duration = "<<duration<<endl;

    }

    string type, file_name,host_name,port_number;
    vector<string> splited = split(command, ' ');
    if(splited.size() > 0) type = splited[0];
    if(splited.size() > 1) file_name = splited[1];
    if(splited.size() > 2) host_name = splited[2];
    if(splited.size() > 3) port_number = splited[3];
    cout << "type: " << type << endl;
    cout << "file_name: " << file_name << endl;
    cout << "host_name: " << host_name << endl;
    cout << "port_number: " << port_number << endl;
    if(file_name.size()>0&&file_name.at(0)=='/')
    {
        cout<<"in0"<<endl;
        getHtmlForRealClient(file_name.substr(1));
    }
    else
    {
        cout<<"no"<<endl;
        if (type.compare("GET")==0)
            get(params->clientSocket, file_name);
        else if (type.compare("POST")==0)
        {
            post(params->clientSocket, file_name);
        }
    }

    close(params->clientSocket);
    pthread_exit(NULL);
}
void getHtmlForRealClient(string file_name)
{
    cout<<"in1"<<endl;
    vector<string> splited = split(file_name, '.');
    string extension = splited[1];
    string name = splited[0];
    string htmlFile=name+".html";
    if(extension.compare("html")==0)
    {
        string path = "firefox /home/remon/Desktop/NetWork/Server/"+file_name;
        char* path2=new char[path.length()];
        strcpy(path2, path.c_str());
        system(path2);
        delete path2;
    }
    else if(extension.compare("png")==0)
    {
        ofstream file;
        file.open(htmlFile.c_str());

        file<<"<img display:\"inline-block\" position:\"relative\" src=\""<<"/home/remon/Desktop/NetWork/Server/"<<file_name<<"\" width: \"100%\" height=\"100%\" style=\"position:absolute;right:35%;\">"<<endl;
        file.close();
        string path = "firefox /home/remon/Desktop/NetWork/Server/"+htmlFile;
        char* path2=new char[path.length()];
        strcpy(path2, path.c_str());
        system(path2);
        delete path2;
    }
    else if(extension.compare("txt")==0)
    {
        ofstream file;
        file.open(htmlFile.c_str());
        ifstream file2;
        file2.open(file_name.c_str());
        string line;
        if (file2.is_open())
        {
            while (!file2.eof())
            {
                getline(file2,line);
                file << line;
            }
        }
        file.close();
        file2.close();
        string path = "firefox /home/remon/Desktop/NetWork/Server/"+htmlFile;
        char* path2=new char[path.length()];
        strcpy(path2, path.c_str());
        system(path2);
        delete path2;
    }

}

void get(int clientSocket, string fileName)
{

    FILE *file = fopen(fileName.c_str(), "rb");
    //file not found
    if (file == NULL)
    {

        string error = "HTTP/1.0 404 Not Found\r\n";
        char buffer[error.size() + 1];
        strncpy(buffer, error.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        if(send(clientSocket, buffer, sizeof(buffer), 0)<0)
            cerr<<"send not found msg failed"<<endl;


        return;
    }

    //file found
    string data = "HTTP/1.0 200 OK\r\n";
    char buffer[1024];
    strncpy(buffer, data.c_str(), sizeof(buffer));
    buffer[sizeof(buffer)-1] = 0;
    if (send(clientSocket, buffer, sizeof(buffer), 0) != sizeof(buffer))
        cerr<<"send found msg failed"<<endl;
    int read_size;
    char data_buffer[1024];
    while (!feof(file))
    {
        read_size = fread(data_buffer, 1, 1024 , file);
        //cout<<read_size<<endl;
        if (send(clientSocket, data_buffer, read_size, 0) != read_size)
        {
            cerr<<"send data failed"<<endl;
        }
    }
    fclose(file);
}

void post(int clientSocket, string fileName)
{
    //create file with fileName
    //cout<<"i am in Post1"<<endl;
    string ok = "OK";
    char buffer2[ok.size() + 1];
    strncpy(buffer2, ok.c_str(), sizeof(ok));
    buffer2[sizeof(buffer2) - 1] = 0;
    if(send(clientSocket, buffer2, sizeof(buffer2), 0)<0)
        cerr<<"send not ok msg failed"<<endl;
    FILE* file=fopen(fileName.c_str(), "wb");
    //cout<<"i am in Post2"<<endl;.substr(1,file_name.size-1)
    int recevingDataLength;
    char buffer[1024];
    //cout<<"i am in Post3"<<endl;1
    if ((recevingDataLength = recv(clientSocket, buffer, sizeof(buffer), 0)) < 0)
        cerr<<"receving file form host failed 1."<<endl;
    //cout<<"i am in Post4 len "<<recevingDataLength<<endl;
    while (recevingDataLength > 0)
    {
        //cout<<"i am in Post5"<<endl;
        fwrite(buffer, 1, recevingDataLength, file);
        //cout<<"i am in Post6+"<<endl;
        if ((recevingDataLength = recv(clientSocket, buffer, sizeof(buffer), 0)) < 0)
            cerr<<"receving file form host failed 2."<<endl;
    }
    fclose(file);
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
//        cout<<"delim_string = "<<delim_string<<endl;
//        cout<<"item = "<<item<<endl;
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
