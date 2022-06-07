#include "MJPEGWriter.h"
#include "auto.h"


using namespace Streamer;

bool MJPEGWriter::ViewerExists() const
{
    return IsViewer;
}

void MJPEGWriter::write(const cv::Mat &frame){
    pthread_mutex_lock(&mutex_writer);
    Auto(pthread_mutex_unlock(&mutex_writer));
    lastFrame = frame.clone();
}

void MJPEGWriter::setPort(int value)
{
    port = value;
}

void MJPEGWriter::setFPS(int value)
{
    port = value;
}

void
MJPEGWriter::Listener()
{

    // send http header
    std::string header;
    header += "HTTP/1.0 200 OK\r\n";
    header += "Cache-Control: no-cache\r\n";
    header += "Pragma: no-cache\r\n";
    header += "Connection: close\r\n";
    header += "Content-Type: multipart/x-mixed-replace; boundary=mjpegstream\r\n\r\n";
    const int header_size = header.size();
    char* header_data = (char*)header.data();
    fd_set rread;
    SOCKET maxfd;
    this->open();
    pthread_mutex_unlock(&mutex_writer);
    while (true)
    {
        rread = master;
        struct timeval to = { 0, timeout };
        maxfd = sock + 1;
        if (sock == INVALID_SOCKET){
            return;
        }
        int sel = select(maxfd, &rread, NULL, NULL, &to);
        if (sel > 0) {
            for (int s = 0; s < maxfd; s++)
            {
                if (FD_ISSET(s, &rread) && s == sock)
                {
                    int         addrlen = sizeof(SOCKADDR);
                    SOCKADDR_IN address { };
                    SOCKET      client = accept(sock, (SOCKADDR*)&address, (socklen_t*)&addrlen);
                    if (client == SOCKET_ERROR)
                    {
                        cerr << "error : couldn't accept connection on sock " << sock << " !" << endl;
                        return;
                    }
                    maxfd = (maxfd>client ? maxfd : client);
                    pthread_mutex_lock(&mutex_cout);
                    cout << "new client " << client << endl;
                    char headers[4096] = "\0";
                    /*int readBytes = */_read(client, headers);
                    //                    cout << headers;
                    pthread_mutex_unlock(&mutex_cout);
                    pthread_mutex_lock(&mutex_client);
                    _write(client, header_data, header_size);
                    clients.push_back(client);
                    pthread_mutex_unlock(&mutex_client);
                }
            }
        }
        usleep(1000);
    }
}

void
MJPEGWriter::Writer()
{
    pthread_mutex_lock(&mutex_writer);
    pthread_mutex_unlock(&mutex_writer);
    const int _fr = 14;
    const int milis2wait = static_cast<int> (1000000 / _fr);

    std::vector<uchar> outbuf;
    while (this->isOpened())
    {
        pthread_mutex_lock(&mutex_client);
        int num_connected_clients = clients.size();
        pthread_mutex_unlock(&mutex_client);
        this->IsViewer = num_connected_clients;
        if (!num_connected_clients) {
            usleep(milis2wait);
            continue;
        }
        pthread_t threads[NUM_CONNECTIONS];
        int count = 0;


        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(quality);
        pthread_mutex_lock(&mutex_writer);
        auto const Frame = lastFrame.clone();
        pthread_mutex_unlock(&mutex_writer);
        try
        {
            imencode(".jpg", Frame, outbuf, params);
        }
        catch(cv::Exception const &)
        {
            //Do nothing and use the previous outbuf
        }
        int outlen = outbuf.size();

        pthread_mutex_lock(&mutex_client);
        std::vector<int>::iterator begin = clients.begin();
        std::vector<int>::iterator end = clients.end();
        pthread_mutex_unlock(&mutex_client);
        std::vector<clientPayload*> payloads;
        for (std::vector<int>::iterator it = begin; it != end; ++it, ++count)
        {
            if (count > NUM_CONNECTIONS)
                break;
            struct clientPayload *cp = new clientPayload({ (MJPEGWriter*)this, { outbuf.data(), outlen, *it } });
            payloads.push_back(cp);
            pthread_create(&threads[count], NULL, &MJPEGWriter::clientWrite_Helper, cp);
        }
        for (; count > 0; count--)
        {
            pthread_join(threads[count-1], NULL);
            delete payloads.at(count-1);
        }
        usleep(milis2wait);
    }
}

void
MJPEGWriter::ClientWrite(clientFrame & cf)
{
    std::stringstream head;
    head << "--mjpegstream\r\nContent-Type: image/jpeg\r\nContent-Length: " << cf.outlen << "\r\n\r\n";
    auto const string_head = head.str();
    pthread_mutex_lock(&mutex_client);
    _write(cf.client, (char*) string_head.c_str(), string_head.size());
    int n = _write(cf.client, (char*)(cf.outbuf), cf.outlen);
    if (n < cf.outlen)
    {
        auto it = find (clients.begin(), clients.end(), cf.client);
        if (it != clients.end())
        {
            cerr << "kill client " << cf.client << endl;
            clients.erase(std::remove(clients.begin(), clients.end(), cf.client));
            ::shutdown(cf.client, SHUT_RDWR);
        }
    }
    pthread_mutex_unlock(&mutex_client);
    pthread_exit(NULL);
}
