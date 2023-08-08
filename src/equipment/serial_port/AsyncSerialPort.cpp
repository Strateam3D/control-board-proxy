#include "AsyncSerialPortPort.hpp"

#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>


using namespace std;
using namespace boost;

namespace strateam::equipment{

class BasicAsyncSerialPortImpl: private boost::noncopyable
{
public:
    BasicAsyncSerialPortImpl(): io(), port(io), backgroundThread(), open(false),
            error(false) {}

    boost::asio::io_service io; ///< Io service object
    boost::asio::serial_port port; ///< Serial port object
    std::thread backgroundThread; ///< Thread that runs read/write operations
    bool open; ///< True if port open
    bool error; ///< Error flag
    mutable std::mutex errorMutex; ///< Mutex for access to error

    /// Data are queued here before they go in writeBuffer
    std::vector<char> writeQueue;
    boost::shared_array<char> writeBuffer; ///< Data being written
    size_t writeBufferSize; ///< Size of writeBuffer
    std::mutex writeQueueMutex; ///< Mutex for access to writeQueue
    char readBuffer[BasicBasicAsyncSerialPortPort::readBufferSize]; ///< data being read

    /// Read complete callback
    std::function<void (const char*, size_t)> callback;
};

BasicBasicAsyncSerialPortPort::BasicBasicAsyncSerialPortPort(): pimpl(new BasicAsyncSerialPortImpl)
{}

BasicAsyncSerialPort::BasicAsyncSerialPort(const std::string& devname, unsigned int baud_rate,
        asio::serial_port_base::parity opt_parity,
        asio::serial_port_base::character_size opt_csize,
        asio::serial_port_base::flow_control opt_flow,
        asio::serial_port_base::stop_bits opt_stop)
        : pimpl(new BasicAsyncSerialPortImpl)
{
    open(devname,baud_rate,opt_parity,opt_csize,opt_flow,opt_stop);
}

void BasicAsyncSerialPort::open(const std::string& devname, unsigned int baud_rate,
        asio::serial_port_base::parity opt_parity,
        asio::serial_port_base::character_size opt_csize,
        asio::serial_port_base::flow_control opt_flow,
        asio::serial_port_base::stop_bits opt_stop)
{
    if(isOpen()) close();

    setErrorStatus(true);//If an exception is thrown, error_ remains true
    pimpl->port.open(devname);
    pimpl->port.set_option(asio::serial_port_base::baud_rate(baud_rate));
    pimpl->port.set_option(opt_parity);
    pimpl->port.set_option(opt_csize);
    pimpl->port.set_option(opt_flow);
    pimpl->port.set_option(opt_stop);

    //This gives some work to the io_service before it is started
    pimpl->io.post(boost::bind(&BasicAsyncSerialPort::doRead, this));

    thread t(boost::bind(&asio::io_service::run, &pimpl->io));
    pimpl->backgroundThread.swap(t);
    setErrorStatus(false);//If we get here, no error
    pimpl->open=true; //Port is now open
    std::cout << "BasicAsyncSerialPort::open " <<pimpl->open << std::endl;
}

bool BasicAsyncSerialPort::isOpen() const
{
    return pimpl->open;
}

bool BasicAsyncSerialPort::errorStatus() const
{
    lock_guard<mutex> l(pimpl->errorMutex);
    return pimpl->error;
}

void BasicAsyncSerialPort::close()
{
    if(!isOpen()) return;

    pimpl->open=false;
    pimpl->io.post(boost::bind(&BasicAsyncSerialPort::doClose, this));
    pimpl->backgroundThread.join();
    pimpl->io.reset();
    if(errorStatus())
    {
        throw(boost::system::system_error(boost::system::error_code(),
                "Error while closing the device"));
    }
}

void BasicAsyncSerialPort::write(const char *data, size_t size)
{
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        pimpl->writeQueue.insert(pimpl->writeQueue.end(),data,data+size);
    }
    pimpl->io.post(boost::bind(&BasicAsyncSerialPort::doWrite, this));
}

void BasicAsyncSerialPort::write(const std::vector<char>& data)
{
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        pimpl->writeQueue.insert(pimpl->writeQueue.end(),data.begin(),
                data.end());
    }
    pimpl->io.post(boost::bind(&BasicAsyncSerialPort::doWrite, this));
}

void BasicAsyncSerialPort::writeString(const std::string& s)
{
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        pimpl->writeQueue.insert(pimpl->writeQueue.end(),s.begin(),s.end());
    }
    pimpl->io.post(boost::bind(&BasicAsyncSerialPort::doWrite, this));
}

BasicAsyncSerialPort::~BasicAsyncSerialPort()
{
    if(isOpen())
    {
        try {
            close();
        } catch(...)
        {
            //Don't throw from a destructor
        }
    }
}

void BasicAsyncSerialPort::doRead()
{
    pimpl->port.async_read_some(asio::buffer(pimpl->readBuffer,readBufferSize),
            boost::bind(&BasicAsyncSerialPort::readEnd,
            this,
            asio::placeholders::error,
            asio::placeholders::bytes_transferred));
}

void BasicAsyncSerialPort::readEnd(const boost::system::error_code& error,
        size_t bytes_transferred)
{
    if(error)
    {
        #ifdef __APPLE__
        if(error.value()==45)
        {
            //Bug on OS X, it might be necessary to repeat the setup
            //http://osdir.com/ml/lib.boost.asio.user/2008-08/msg00004.html
            doRead();
            return;
        }
        #endif //__APPLE__
        //error can be true even because the serial port was closed.
        //In this case it is not a real error, so ignore
        if(isOpen())
        {
            doClose();
            setErrorStatus(true);
        }
    } else {
//    	std::cout << "readEnd got " << bytes_transferred << " bytes\n";
        if(pimpl->callback) pimpl->callback(pimpl->readBuffer,
                bytes_transferred);
        doRead();
    }
}

void BasicAsyncSerialPort::doWrite()
{
    //If a write operation is already in progress, do nothing
    if(pimpl->writeBuffer==0)
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        pimpl->writeBufferSize=pimpl->writeQueue.size();
        pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
        copy(pimpl->writeQueue.begin(),pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
        pimpl->writeQueue.clear();
        async_write(pimpl->port,asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&BasicAsyncSerialPort::writeEnd, this, asio::placeholders::error));
    }
}

void BasicAsyncSerialPort::writeEnd(const boost::system::error_code& error)
{
    if(!error)
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        if(pimpl->writeQueue.empty())
        {
            pimpl->writeBuffer.reset();
            pimpl->writeBufferSize=0;
            
            return;
        }
        pimpl->writeBufferSize=pimpl->writeQueue.size();
        pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
        copy(pimpl->writeQueue.begin(),pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
        pimpl->writeQueue.clear();
        async_write(pimpl->port,asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&BasicAsyncSerialPort::writeEnd, this, asio::placeholders::error));
    } else {
        setErrorStatus(true);
        doClose();
    }
}

void BasicAsyncSerialPort::doClose()
{
    boost::system::error_code ec;
    pimpl->port.cancel(ec);
    if(ec) setErrorStatus(true);
    pimpl->port.close(ec);
    if(ec) setErrorStatus(true);
}
void BasicAsyncSerialPort::doWrite()
{
    //If a write operation is already in progress, do nothing
    if(pimpl->writeBuffer==0)
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        pimpl->writeBufferSize=pimpl->writeQueue.size();
        pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
        copy(pimpl->writeQueue.begin(),pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
        pimpl->writeQueue.clear();
        async_write(pimpl->port,asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&BasicAsyncSerialPort::writeEnd, this, asio::placeholders::error));
    }
}

void BasicAsyncSerialPort::writeEnd(const boost::system::error_code& error)
{
    if(!error)
    {
        lock_guard<mutex> l(pimpl->writeQueueMutex);
        if(pimpl->writeQueue.empty())
        {
            pimpl->writeBuffer.reset();
            pimpl->writeBufferSize=0;
            
            return;
        }
        pimpl->writeBufferSize=pimpl->writeQueue.size();
        pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
        copy(pimpl->writeQueue.begin(),pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
        pimpl->writeQueue.clear();
        async_write(pimpl->port,asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&BasicAsyncSerialPort::writeEnd, this, asio::placeholders::error));
    } else {
        setErrorStatus(true);
        doClose();
    }
}

void BasicAsyncSerialPort::doClose()
{
    boost::system::error_code ec;
    pimpl->port.cancel(ec);
    if(ec) setErrorStatus(true);
    pimpl->port.close(ec);
    if(ec) setErrorStatus(true);
}

void BasicAsyncSerialPort::setErrorStatus(bool e)
{
    lock_guard<mutex> l(pimpl->errorMutex);
    pimpl->error=e;
}

void BasicAsyncSerialPort::setReadCallback(const std::function<void (const char*, size_t)>& callback)
{
    pimpl->callback=callback;
}

void BasicAsyncSerialPort::clearReadCallback()
{
    std::function<void (const char*, size_t)> empty;
    pimpl->callback.swap(empty);
}

AsyncSerialPort::AsyncSerialPort(): AsyncSerial()
{

}

AsyncSerialPort::AsyncSerialPort(const std::string& devname,
        unsigned int baud_rate,
        asio::serial_port_base::parity opt_parity,
        asio::serial_port_base::character_size opt_csize,
        asio::serial_port_base::flow_control opt_flow,
        asio::serial_port_base::stop_bits opt_stop)
        :AsyncSerial(devname,baud_rate,opt_parity,opt_csize,opt_flow,opt_stop)
{

}

void AsyncSerialPort::setCallback(const std::function<void (const char*, size_t)>& callback)
{
    setReadCallback(callback);
}

void AsyncSerialPort::clearCallback()
{
    clearReadCallback();
}

AsyncSerialPort::~AsyncSerialPort()
{
    clearReadCallback();
}



}