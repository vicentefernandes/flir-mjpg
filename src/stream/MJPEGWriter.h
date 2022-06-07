#ifndef MJPEGWRITER_H
#define MJPEGWRITER_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <unistd.h>
#include <iostream>
#define PORT        unsigned short
#define SOCKET    int
#define HOSTENT  struct hostent
#define SOCKADDR    struct sockaddr
#define SOCKADDR_IN  struct sockaddr_in
#define ADDRPOINTER  unsigned int*
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define TIMEOUT_M       200000
#define NUM_CONNECTIONS 10

#include <pthread.h>

namespace Streamer
{

    using std::cout;
    using std::cerr;
    using std::endl;

    class MJPEGWriter{

        struct clientFrame {
            uchar* outbuf;
            int outlen;
            int client;
        };

        struct clientPayload {
            void* context;
            clientFrame cf;
        };

        std::atomic_bool IsViewer {};
        SOCKET sock;
        fd_set master;
        int timeout;
        int quality; // jpeg compression [1..100]
        std::vector<int> clients;
        pthread_t thread_listen, thread_write;
        mutable pthread_mutex_t mutex_client = PTHREAD_MUTEX_INITIALIZER;
        mutable pthread_mutex_t mutex_cout = PTHREAD_MUTEX_INITIALIZER;
        mutable pthread_mutex_t mutex_writer = PTHREAD_MUTEX_INITIALIZER;
        cv::Mat lastFrame;
        int port;
        int fps;

        int _write(int sock, char *s, int len)
        {
            if (len < 1) { len = strlen(s); }
            {
                try
                {
                    int retval = ::send(sock, s, len, 0x4000);
                    return retval;
                }
                catch (int e)
                {
                    cerr << "An exception occurred. Exception Nr. " << e << endl;;
                }
            }
            return -1;
        }

        int _read(int socket, char* buffer)
        {
            int result;
            result = recv(socket, buffer, 4096, MSG_PEEK);
            if (result < 0 )
            {
                cerr <<"An exception occurred. Exception Nr. " << result << endl;
                return result;
            }
            std::string s = buffer;
            buffer = (char*) s.substr(0, (int) result).c_str();
            return result;
        }

        static void* listen_Helper(void* context)
        {
            ((MJPEGWriter *)context)->Listener();
            return NULL;
        }

        static void* writer_Helper(void* context)
        {
            ((MJPEGWriter *)context)->Writer();
            return NULL;
        }

        static void* clientWrite_Helper(void* payload)
        {
            void* ctx = ((clientPayload *)payload)->context;
            struct clientFrame cf = ((clientPayload *)payload)->cf;
            ((MJPEGWriter *)ctx)->ClientWrite(cf);
            return NULL;
        }

    public:

        MJPEGWriter(int port = 0, int fps = 25)
        : sock(INVALID_SOCKET)
        , timeout(TIMEOUT_M)
        , quality(55) //TODO set quality here
        , port(port)
        , fps(fps)
        {
            signal(SIGPIPE, SIG_IGN);
            FD_ZERO(&master);
            // if (port)
            //     open(port);
        }

        //Swap idiom
        auto swap(MJPEGWriter& rhs) noexcept(true)
        {
            using std::swap;
            swap(this->sock, rhs.sock);
            swap(this->master, rhs.master);
            swap(this->timeout, rhs.timeout);
            swap(this->quality, rhs.quality);
            swap(this->port, rhs.port);
            swap(this->lastFrame, rhs.lastFrame);
        }

        friend auto swap(MJPEGWriter& lhs, MJPEGWriter& rhs) noexcept(true)
        {
            lhs.swap(rhs);
        }

        //Copy semantic
        MJPEGWriter(const MJPEGWriter& RHS)
        {
            this->IsViewer.store(RHS.IsViewer.load());
            this->sock = RHS.sock;
            this->master = RHS.master;
            this->timeout = RHS.timeout;
            this->quality = RHS.quality;
            this->port = RHS.port;
            this->lastFrame = RHS.lastFrame;
        }

        MJPEGWriter &operator=(MJPEGWriter RHS)
                {
            this->IsViewer.store(RHS.IsViewer.load());
            RHS.swap(*this);
            return *this;
                }

                //Move semantic
                MJPEGWriter (MJPEGWriter&& Other) noexcept(true)
                :MJPEGWriter()
                {
            this->IsViewer.store(Other.IsViewer.load());
            this->swap(Other);
                };

        ~MJPEGWriter()
        {
            release();
        }

        bool release()
        {
            if (sock != INVALID_SOCKET)
                shutdown(sock, 2);
            sock = (INVALID_SOCKET);
            return false;
        }

        bool open()
        {
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            SOCKADDR_IN address;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_family = AF_INET;
            address.sin_port = htons(port);
            if (::bind(sock, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
            {
                cerr << "error : couldn't bind sock " << sock << " to port " << port << "!" << endl;
                return release();
            }
            if (listen(sock, NUM_CONNECTIONS) == SOCKET_ERROR)
            {
                cerr << "error : couldn't listen on sock " << sock << " on port " << port << " !" << endl;
                return release();
            }
            FD_SET(sock, &master);
            return true;
        }

        bool isOpened()
        {
            return sock != INVALID_SOCKET;
        }

        void start(){
            pthread_mutex_lock(&mutex_writer);
            pthread_create(&thread_listen, NULL, this->listen_Helper, this);
            pthread_create(&thread_write, NULL, this->writer_Helper, this);
        }

        void stop(){
            this->release();
            pthread_join(thread_listen, NULL);
            pthread_join(thread_write, NULL);
        }

        void write(cv::Mat const &frame);

        void setPort(int value);

        void setFPS(int value);

        auto ViewerExists() const ->bool;

    private:
        void Listener();
        void Writer();
        void ClientWrite(clientFrame &cf);
    };

} //namespace streamer

#endif //MJPEGWRITER_H
