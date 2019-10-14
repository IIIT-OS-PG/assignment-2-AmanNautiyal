#include "custom_headers.h"
using namespace std;
unordered_map<string, vector<string>> table;
pthread_mutex_t table_lock;
struct argument
{
	struct sockaddr_in *address;
	int sockid;
};
void *tracker(void *arguments)
{
	int sockid=socket(PF_INET,SOCK_STREAM,0); //Domain, type ,protocol
	if(sockid < 0)
	{
		cout<<"Error in starting server\n";
		exit(1);
	}
	struct sockaddr_in s_address;
	socklen_t addr_size;
	s_address.sin_family=AF_INET;
	s_address.sin_port=htons(8000);
	s_address.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addr_len=sizeof(s_address);
	int b;
	if((b=bind(sockid,(struct sockaddr *)&s_address,sizeof(s_address)))<0)
	{
		cout<<"Bind failed\n";
		exit(1);
	}
	if(listen(sockid,50)==0);
		//cout<<"Server initialized\n";
	else
		cout<<"Listen failed\n";
	
	while(true)
	{
		struct sockaddr_in client_addr;
		socklen_t addr_size=sizeof(client_addr);
		int newsocket=accept(sockid,(struct sockaddr *)&client_addr,&addr_size);
		struct argument arg;
		arg.address=&client_addr;
		arg.sockid=newsocket;
		pthread_t thread;
		if( pthread_create(&thread,NULL,serveTrackreq,&arg)!=0)
           cout<<"Thread creation failed\n";
       	pthread_detach(thread);
	}
}
void *serveTrackreq(void *arguments)
{
	int rs;
	char message[1024];
	struct argument *args=((struct argument*)arguments);
	int new_sockid=args->sockid;
	memset(message,'\0',sizeof(message));
	rs=recv(new_sockid,message,sizeof(message),0);
	cout<<"Received:"<<message<<"\n";
	// //cout<<rs<<"\n";
	char ip[20];
	inet_ntop(AF_INET, &(((struct sockaddr_in *)args->address)->sin_addr),
                    ip, 20);
	//cout<<ip<<"\n";
	if(strcmp(message,"share")==0)
		share_file(new_sockid,ip);
	else if(strcmp(message,"getfile")==0)
		get_file(new_sockid);
	// FILE* fp=fopen(name,"wb");
	// char buffer_write[1024]={0};
	// long long recieved=0;
	// //fwrite(buffer_write,sizeof(char),rs,fp);
	// while((rs=recv(new_sockid,buffer_write,sizeof(buffer_write),0))>0)
	// {
	// 	cout<<name<<endl;
	// 	fwrite(buffer_write,sizeof(char),rs,fp);
	// 	memset(buffer_write,'\0',sizeof(buffer_write));
	// 	recieved+=rs;
	// }
	// cout<<name<<":Bits recieved="<<recieved<<endl;
	close(new_sockid);
	// fclose(fp);
	pthread_exit(NULL);
}
void get_file(int sockid)
{
	char name[1024];
	char *message;
	int ind;
	memset(name,'\0',sizeof(name));
	ind=recv(sockid,name,sizeof(name),0);
	cout<<"Received:"<<name<<"\n";
	string filename(name,strlen(name));
	if (table.find(filename) == table.end()) 
	{
        cout << "File Not Present\n"; 
        message=(char *)"Not present";
		cout<<"Sent:"<<message<<"\n";
		ind=send(sockid,message,1024,0);// request to tracker sent
		return;
	}
    else
    {
       cout << "File Present\n"; 
        message=(char *)"Present";
		cout<<"Sent:"<<message<<"\n";
		ind=send(sockid,message,1024,0);// request to tracker sent
    }
    char *ip=const_cast<char*>(table.at(filename)[0].data());
    char *port=const_cast<char*>(table.at(filename)[1].data());
    char *hash=const_cast<char*>(table.at(filename)[2].data());
    cout<<"Sent:"<<ip<<"\n";
	ind=send(sockid,ip,1024,0);// ip sent
	cout<<"Sent:"<<port<<"\n";
	ind=send(sockid,port,1024,0);// port sent
	cout<<"Sent:"<<hash<<"\n";
	ind=send(sockid,hash,1024,0);// hash sent
}
void share_file(int sockid,char *ip)
{
	char name[1024],port[1024],hash[1024];
	int rs;
	//cout<<ip<<"\n";
	memset(name,'\0',sizeof(name));
	rs=recv(sockid,name,sizeof(name),0);
	memset(port,'\0',sizeof(port));
	rs=recv(sockid,port,sizeof(port),0);
	memset(hash,'\0',sizeof(hash));
	rs=recv(sockid,hash,sizeof(hash),0);
	//cout<<"Name:"<<name<<"\n";
	//cout<<"Port:"<<port<<"\n";
	vector<string> stor;
	stor.push_back(string(ip,strlen(ip)));
	stor.push_back(string(port,strlen(port)));
	stor.push_back(string(hash,strlen(hash)));
	pthread_mutex_lock(&table_lock);
	table[string(name,strlen(name))]=stor;
    for (unordered_map<string, vector<string>>:: iterator itr=table.begin();itr!=table.end();itr++) 
    { 
        cout<<itr->first<<"\t"<<itr->second[0]<<"\t"<<itr->second[1]<<"\t"<<itr->second[2]<<endl; 
     } 
	pthread_mutex_unlock(&table_lock);
}
int main()
{
	pthread_t thread;
	if (pthread_mutex_init(&table_lock, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        exit(1); 
    } 
	if( pthread_create(&thread,NULL,tracker,NULL)!=0)
           cout<<"Thread creation failed\n";
    int choice=0;
    cout<<"Enter any non zero number to exit:\n";
    while(choice==0)
    {
    	cin>>choice;
    }
    pthread_mutex_lock(&table_lock);
    for (unordered_map<string, vector<string>>:: iterator itr=table.begin();itr!=table.end();itr++) 
    { 
        cout<<itr->first<<"\t"<<itr->second[0]<<"\t"<<itr->second[1]<<"\t"<<itr->second[2]<<endl; 
     } 
     pthread_mutex_unlock(&table_lock);
     pthread_mutex_destroy(&table_lock);
	return 0;
}