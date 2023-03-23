#include <iostream>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_LENGTH    250
#define FALSE              0

int main(int, char**) {
{
   std::string data{"Fish mouth is opening."};

   int    sd=-1, rv, bytesReceived;
   char   buffer[1024];
   struct sockaddr_un serveraddr;

   do
   {
      sd = socket(AF_UNIX, SOCK_STREAM, 0);
      if (sd < 0)
      {
         perror("socket() failed");
         break;
      }

      memset(&serveraddr, 0, sizeof(serveraddr));
      serveraddr.sun_family = AF_UNIX;

      auto server_path = "./stockmarket.socket";

      strcpy(serveraddr.sun_path, server_path);

      rv = connect(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
      if (rv < 0)
      {
         perror("connect() failed");
         break;
      }

      rv = send(sd, data.c_str(), data.length(), 0);
      if (rv < 0)
      {
         perror("send() failed");
         break;
      }

      std::cout << "waiting for a reply\n";
      bytesReceived = 0;
      while (true)
      {
         rv = recv(sd, & buffer[bytesReceived],
                   BUFFER_LENGTH - bytesReceived, 0);
         if (rv < 0)
         {
            perror("recv() failed");
            break;
         }

         if (rv == 0 && bytesReceived > 0)
         {
            std::cout << "get some data\n";
            break;
         }
         bytesReceived += rv;
      }
      buffer[bytesReceived+1] = '\0';
      std::cout << buffer << "\n";

   } while (FALSE);

   if (sd != -1)
      close(sd);
}

}
