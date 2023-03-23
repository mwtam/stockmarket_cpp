#include <iostream>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <fmt/format.h>

void print_error(std::string_view s, int rv, int error_num)
{
    std::cerr << s << " failed\n";
    std::cerr << "rv: " << rv << "\n";
    std::cerr << "errno: " << error_num << " " << strerror(error_num) << "\n";
}

auto setup_socket(const std::string& server_path)
{
    auto rv = -1;

    auto sd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sd < 0)
    {
        print_error("socket()", sd, errno);
        return -1;
    }

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, server_path.c_str());

    rv = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    if (rv < 0)
    {
        print_error("bind()", rv, errno);
        return -1;
    }

    rv = listen(sd, 10);
    if (rv < 0)
    {
        print_error("listen()", rv, errno);
        return -1;
    }

    return sd;
}

auto play_with_socket()
{
    auto server_path = "./stockmarket.socket";
    auto rv = -1;
    auto sd = setup_socket(server_path);

    std::cout << "Ready for client connect().\n";

    while(true)
    {
        fd_set our_sockets;
        FD_ZERO(&our_sockets);
        FD_SET(sd, &our_sockets);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        rv = select(sd+1, &our_sockets, NULL, NULL, &timeout);
        if (rv == 0) {
            fmt::print("Waiting\n");
        }
        else if (rv < 0)
        {
            print_error("select()", rv, errno);
        }
        else
        {
            break;
        }
    }

    auto sd2 = accept(sd, NULL, NULL);
    fmt::print("after accept\n");
    if (sd2 < 0)
    {
        print_error("accept()", sd2, errno);
        return -1;
    }

    char buffer[1024];

    do
    {
        rv = recv(sd2, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (rv < 0)
        {
            if (errno == EAGAIN)
            {
                fmt::print("finished reading");
                break;
            }
            else
            {
                print_error("recv()", rv, errno);
                return -1;
            }
        }
        buffer[rv + 1] = '\0';
        std::cout << rv << ": " << buffer << "\n";
    } while (rv != 0);

    rv = send(sd2, "fish", 4, 0);
    if (rv < 0)
    {
        print_error("send()", rv, errno);
        return -1;
    }

    if (sd != -1)
        close(sd);

    if (sd2 != -1)
        close(sd2);

    unlink(server_path);

    return 0;
}

int main(int, char**) {
    play_with_socket();
}
