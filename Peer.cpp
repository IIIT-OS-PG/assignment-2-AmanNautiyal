#include "custom_headers.h"
using namespace std;
void *setup_server(void *arguments)
{
	int port=*((int *)arguments);
	int sockid=socket(PF_INET,SOCK_STREAM,0); //Domain, type ,protocol
	if(sockid < 0)
	{
		cout<<"Error in starting server\n";
		exit(1);
	}
	struct sockaddr_in s_address,serv_stor;
	socklen_t addr_size;
	s_address.sin_family=AF_INET;
	s_address.sin_port=htons(port);
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
		struct sockaddr_in serv_stor;
		socklen_t addr_size=sizeof(serv_stor);
		int newsocket=accept(sockid,(struct sockaddr *)&serv_stor,&addr_size);
		pthread_t thread;
		if( pthread_create(&thread,NULL,serveReq,&newsocket)!=0)
           cout<<"Thread creation failed\n";
       	pthread_detach(thread);
	}
}
void *serveReq(void *arguments)
{
	int rs;
	int new_sockid=*((int *)arguments);
	char message[1024];
	memset(message,'\0',sizeof(message));
	rs=recv(new_sockid,message,sizeof(message),0);
	cout<<"Received:"<<message<<"\n";
	// //cout<<rs<<"\n";
	if(strcmp(message,"download")==0)
		send_file(new_sockid);
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
void send_file(int sockid)
{
	char name[1024];
	char *message;
	memset(name,'\0',sizeof(name));
	int rs=recv(sockid,name,sizeof(name),0);
	cout<<"Received:"<<name<<"\n";
	FILE *fp=fopen(name,"rb");
	if(fp==NULL)
	{
		message=(char*)"Not present";
		send(sockid,message,1024,0);
		cout<<"Sent:"<<message<<"\n";
		close(sockid);
		return;
	}
	message=(char*)"Present";
	send(sockid,message,1024,0);
	cout<<"Sent:"<<message<<"\n";
	fseek(fp,0,SEEK_END);
	long long size=ftell(fp);
	send(sockid,&size,sizeof(size),0);
	cout<<"Sent:"<<size<<"\n";
	fseek(fp,0,SEEK_SET);
	char buffer_read[1024];
	memset(buffer_read,'\0',1024);
	while(!feof(fp) && (rs=fread(buffer_read,sizeof(char),sizeof(buffer_read),fp))>0)
	{
		send(sockid,buffer_read,rs,0);
		memset(buffer_read,'\0',1024);
	}
	close(sockid);
	fclose(fp);
}
void download_file()
{
	string str;
	int ind;
	char *message;
	char buffer[1024];
	cout<<"Enter filename:";
	cin>>str;
	char *name=const_cast<char*> (str.data());
	int sockid=get_connection((char *)"127.0.0.1",8000);//connection to tracker
	message=(char *)"getfile";
	cout<<"Sent:"<<message<<"\n";
	ind=send(sockid,message,1024,0);// request to tracker sent
	cout<<"Sent:"<<name<<"\n";
	ind=send(sockid,name,1024,0);// file name to download sent to tracker
	ind=recv(sockid,buffer,sizeof(buffer),0);//message whether file record present at tracker
	cout<<"Received:"<<buffer<<"\n";
	if(strcmp(buffer,"Not present")==0)
	{
		cout<<"File not Available\n";
		return;
	}
	char ip[1024],port[1024],hash[1024];
	ind=recv(sockid,ip,sizeof(ip),0);//ip from tracker
	cout<<"Received:"<<ip<<"\n";
	ind=recv(sockid,port,sizeof(port),0);//port from tracker
	cout<<"Received:"<<port<<"\n";
	ind=recv(sockid,hash,sizeof(hash),0);//hash from tracker
	cout<<"Received:"<<hash<<"\n";
	close(sockid);
	sockid=get_connection(ip,atoi(port));
	message=(char *)"download";
	cout<<"Sent:"<<message<<"\n";
	ind=send(sockid,message,1024,0);// request to download sent
	cout<<"Sent:"<<name<<"\n";
	ind=send(sockid,name,1024,0);// file name to download sent
	ind=recv(sockid,buffer,sizeof(buffer),0);
	cout<<"Received:"<<buffer<<"\n";
	if(strcmp(buffer,"Not present")==0)
	{
		cout<<"File not Available\n";
		return;
	}
	long long size;
	ind=recv(sockid,&size,sizeof(size),0);
	cout<<"Received:"<<size<<"\n";
	FILE* fp=fopen(name,"wb");
	char buffer_write[1024]={0};
	while((ind=recv(sockid,buffer_write,sizeof(buffer_write),0))>0)
	{
		fwrite(buffer_write,sizeof(char),ind,fp);
		memset(buffer_write,'\0',sizeof(buffer_write));
	}
	close(sockid);
	fclose(fp);
	char *hash2=filehash(name);
	if(strcmp(hash,hash2)==0)
		cout<<"Hash integrity verificaton successful\n";
	else
		cout<<"Hash integrity verificaton failed\n";
}
int get_connection(char *ip,int port)
{
	int sockid=socket(PF_INET,SOCK_STREAM,0); //Domain, type ,protocol
	if(sockid < 0)
	{
		cout<<"Error in starting client\n";
		exit(1);
	}
	struct sockaddr_in c_address;
	c_address.sin_family=AF_INET;
	c_address.sin_port=htons(port);
	c_address.sin_addr.s_addr=inet_addr(ip);
	int addr_len=sizeof(c_address);
	int c;
	if((c=connect(sockid,(struct sockaddr *)&c_address,addr_len))<0)
	{
		cout<<"Connection could not be established"<<endl;
		exit(1);
	}
	return sockid;
}
void share(char *port)
{
	int ind;
	char *message;
	string str;
	char buffer[1024];
	cout<<"Enter filename:";
	cin>>str;
	char *name=const_cast<char*> (str.data());
	FILE *fp=fopen(name,"rb");
	if(fp==NULL)
	{
		cout<<"File not present in directory\n";
		return;
	}
	int sockid=get_connection((char *)"127.0.0.1",8000);
	message=(char *)"share";
	cout<<"Sent:"<<message<<"\n";
	ind=send(sockid,message,1024,0);// request to share sent
	cout<<"Sent:"<<name<<"\n";
	ind=send(sockid,name,1024,0);// filename to share sent
	cout<<"Sent:"<<port<<"\n";
	ind=send(sockid,port,1024,0);// port sent
	char *hash=filehash(name);
	cout<<"Sent:"<<hash<<"\n";
	ind=send(sockid,hash,1024,0);// port sent
}
char* filehash(char *name)
{
	unsigned char buffer[BUFSIZ];
	cout<<BUFSIZ<<endl;
	FILE *fp=fopen(name,"rb");
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	size_t len=0;
	while(!feof(fp) && (len=fread(buffer,sizeof(char),sizeof(buffer),fp))>0)
	{
		SHA1_Update(&ctx,buffer,len);
	}
	SHA1_Final(buffer,&ctx);
	char *res=(char*)malloc (sizeof (char) * 21);;
	for(int i=0;i<20;i++)
		sprintf(&res[i],"%c ",buffer[i]);
	fclose(fp);
	buffer[20]='\0';
	return res;
}
int main(int argc,char **argv)
{
	if(argc!=2)
	{
		cout<<"Invalid number of arguments\n";
		exit(1);
	}
	pthread_t server;
	int port=atoi(argv[1]);
	if( pthread_create(&server,NULL,setup_server,&port)!=0)
           cout<<"Server setup failed\n";
       int choice=-1;
      while(choice!=3)
      {
      	cout<<"1.Download file\n";
      	cout<<"2.Share file\n";	
      	cout<<"3.Exit\n";
      	cout<<"Enter choice-";
      	cin>>choice;
      	if(choice==1)
      		download_file();
      	else if(choice==2)
      		share(argv[1]);
      }

}