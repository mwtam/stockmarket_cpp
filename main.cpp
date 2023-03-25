#include <iostream>
#include <vector>

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

class SocketWrapper
{
private:
    std::string server_path_;

public:
    int sd;

    void set_server_path(std::string_view server_path);
    void setup_socket();
    auto wait_socket();
    virtual ~SocketWrapper(); // TODO: Rule of five?
};

auto SocketWrapper::wait_socket()
{
    std::vector<int> v;

    int rv = -1;

    fd_set our_sockets;
    FD_ZERO(&our_sockets);
    FD_SET(this->sd, &our_sockets);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    rv = select(this->sd + 1, &our_sockets, NULL, NULL, &timeout);
    if (rv == 0)
    {
        fmt::print("Waiting\n");
    }

    if (rv < 0)
    {
        print_error("select()", rv, errno);
        return v;
    }

    for (auto fd=0; fd<this->sd+1; ++fd)
    {
        if (FD_ISSET(fd, &our_sockets))
        {
            v.emplace_back(fd);
        }
    }

    return v;
}

void SocketWrapper::set_server_path(std::string_view server_path)
{
    this->server_path_ = std::string(server_path);
}

void SocketWrapper::setup_socket()
{
    auto rv = -1;

    auto sd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sd < 0)
    {
        print_error("socket()", sd, errno);
        return;
    }

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, this->server_path_.c_str());

    rv = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    if (rv < 0 && errno == EADDRINUSE)
    {
        print_error("bind()", rv, errno);
        fmt::print("Unlink old server path and try once more.\n");
        unlink(this->server_path_.c_str());
        rv = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    }
    if (rv < 0)
    {
        print_error("bind()", rv, errno);
        return;
    }

    rv = listen(sd, 10);
    if (rv < 0)
    {
        print_error("listen()", rv, errno);
        return;
    }

    this->sd = sd;
}

SocketWrapper::~SocketWrapper()
{
    unlink(this->server_path_.c_str());
    if (this->sd != -1)
    {
        close(this->sd);
    }
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
    SocketWrapper sw;
    sw.set_server_path("./stockmarket.socket");
    sw.setup_socket();

    auto rv = -1;

    std::cout << "Ready for client connect().\n";

    auto sd2 = -1;

    auto v = std::vector<int>{};

    while((v = sw.wait_socket()).size() == 0);

    for (auto &fd : v)
    {
        fmt::print("fd: {}\n", fd);
        if (fd != sw.sd)
        {
            continue;
        }

        sd2 = accept(sw.sd, NULL, NULL);

        if (sd2 < 0)
        {
            print_error("accept()", sd2, errno);
            return -1;
        }

        fmt::print("Accept OK\n");

        break;
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

    if (sd2 != -1)
        close(sd2);

    return 0;
}

int main(int, char**) {
    play_with_socket();
}
