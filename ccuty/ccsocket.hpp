//
//  cpsocket: C++ Classes for TCP/IP and UDP Datagram INET Sockets.
//  (c) Eric Lecolinet 2016/17 - https://www.telecom-paristech.fr/~elc
//

/** @file
 *  Classes for TCP/IP and UDP Datagram INET Sockets.
 *  - Socket: TCP/IP or UDP/Datagram socket (for AF_INET connections following IPv4).
 *  - ServerSocket: TCP/IP Socket Server.
 *  - SocketBuffer: preserves record boundaries when exchanging data between TCP/IP sockets.
 *
 * @author Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
 */

#ifndef ccuty_ccsocket
#define ccuty_ccsocket
/// @file.

#include <string>
#include <sys/types.h>
#include <sys/socket.h>

/// C++ Utilities.
namespace ccuty {
  
// ignore SIGPIPES when possible
#if defined(MSG_NOSIGNAL)
#  define _NO_SIGPIPE(flags) (flags | MSG_NOSIGNAL)
#else
#  define _NO_SIGPIPE(flags) (flags)
#endif
  
  /** @brief TCP/IP or UDP/Datagram socket.
   * This class encapsulates a TCP/IP or UDP/Datagram socket.
   * AF_INET connections following the IPv4 Internet protocol are supported.
   * @note
   * - The ServerSocket class should be used on the server side.
   * - SIGPIPE signals are ignored when using Linux, BSD or MACOSX.
   * - TCP/IP sockets do not preserve record boundaries but class SocketBuffer 
   *   solves this problem.
   */
  class Socket {
  public:
    /** @brief Socket errors.
     * - Socket::Failed (-1): connection error (could not connect, could not bind, etc.)
     * - Socket::InvalidSocket (-2): invalid socket or wrong socket type
     * - Socket::UnknownHost (-3): could not reach host
     */
    enum Errors {Failed = -1, InvalidSocket = -2, UnknownHost = -3};
    
    /** @brief Creates a new Socket.
     * Creates a AF_INET socket using the IPv4 Internet protocol.
     * Type can be:
     * - SOCK_STREAM (the default) for TCP/IP connected stream sockets
     * - SOCK_DGRAM for UDP/datagram sockets
     */
    Socket(int type = SOCK_STREAM);
    
    /// Creates a Socket object from an existing socket file descriptor.
    Socket(int type, int sockfd);
    
    /// Destructor (closes the socket).
    virtual ~Socket();
    
    /** @brief Assign the socket to a local address.
     * Typically used for UDP/Datagram sockets, see Unix 'bind' system call for details.
     * @return 0 on success or a negative value on error, which is one of Socket::Errors
     */
    virtual int bind(int port);
    
    /** @brief Assign the socket to an address.
     * Typically used for UDP/Datagram sockets, see Unix 'bind' system call for details.
     * @return 0 on success or a negative value on error, which is one of Socket::Errors
     */
    virtual int bind(const std::string& host, int port);
    
    /** @brief Connect the socket to an address.
     * Typically used for TCP/IP sockets on the client side, see Unix 'connect' system call
     * for details.
     * @return 0 on success or a negative value on error which is one of Socket::Errors
     */
    virtual int connect(const std::string& host, int port);
    
    /** @brief Closes the socket.
     * @return 0 on success and -1 on error.
     */
    virtual int close();
    
    /// Return true if the socket has been closed.
    bool isClosed() const {return _sockfd < 0;}
    
    /// Return the Unix descriptor of the socket.
    int descriptor() {return _sockfd;}
    
    /** @brief Send data to a connected socket.
     * Sends _len_ bytes to a TCP/IP socket using the Unix 'send' function.
     * @return the number of bytes that were sent or:
     * - _len_ is 0 or shutdownInput() was called on the other side,
     * - Socket::Failed (-1): a connection error occured.
     *
     * @note TCP/IP sockets do not preserve record boundaries but SocketBuffer solves this problem.
     */
    ssize_t send(const void* buf, size_t len, int flags = 0) {
      return ::send(_sockfd, buf, len, _NO_SIGPIPE(flags));
    }
    
    /** @brief Receive data from a connected socket.
     * Reads at most _len_ bytes from a TCP/IP socket using the Unix 'recv' function.
     * By default, this function blocks the caller until data is present.
     * @return the number of bytes that were received or:
     * - 0: _len_ is 0 or shutdownOutput() was called on the other side,
     * - Socket::Failed (-1): a connection error occured.
     *
     * @note TCP/IP sockets do not preserve record boundaries but SocketBuffer solves this problem.
     */
    ssize_t receive(void* buf, size_t len, int flags = 0) {
      return ::recv(_sockfd, buf, len, flags);
    }
    
    /** @brief Send data to a datagram socket.
     * Sends _len_ bytes to a datagram socket using the Unix 'sendto' function.
     * @return the number of bytes that were sent or Socket::Failed (-1) if an error occurred.
     */
    ssize_t sendTo(const void* buf, size_t len, int flags,
                   const struct sockaddr* dest_addr, socklen_t addrlen) {
      return ::sendto(_sockfd, buf, len, _NO_SIGPIPE(flags), dest_addr, addrlen);
    }
    
    /** @brief Receive data from datagram socket.
     * Reads at most _len_ bytes from a datagram socket using the Unix 'recvfrom' function.
     * By default, this function blocks the caller until data is present.
     * @return the number of bytes which was received or Socket::Failed (-1) if an error occurred.
     */
    ssize_t receiveFrom(void* buf, size_t len, int flags,
                        struct sockaddr* src_addr, socklen_t* addrlen) {
      return ::recvfrom(_sockfd, buf, len, flags, src_addr, addrlen);
    }
    
    /// Disable further receive operations.
    virtual void shutdownInput();
    
    /// Disable further send operations.
    virtual void shutdownOutput();
    
    /// Set the size of the TCP/IP input buffer.
    int setReceiveBufferSize(int size);
    
    /// Enable/disable the SO_REUSEADDR socket option.
    int setReuseAddress(bool);
    
    /// Set the size of the TCP/IP output buffer.
    int setSendBufferSize(int size);
    
    /// Enable/disable SO_LINGER with the specified linger time in seconds.
    int setSoLinger(bool, int linger);
    
    /// Enable/disable SO_TIMEOUT with the specified timeout (in milliseconds).
    int setSoTimeout(int timeout);
    
    /// Enable/disable TCP_NODELAY (turns on/off TCP coalescence).
    int setTcpNoDelay(bool);
    
    /// Return the size of the TCP/IP input buffer.
    int getReceiveBufferSize() const;
    
    /// Return SO_REUSEADDR state.
    bool getReuseAddress() const;
    
    /// Return the size of the TCP/IP output buffer.
    int getSendBufferSize() const;
    
    /// Return SO_LINGER state and the specified linger time in seconds.
    bool getSoLinger(int& linger) const;
    
    /// Return SO_TIMEOUT value.
    int getSoTimeout() const;
    
    /// Return TCP_NODELAY state.
    bool getTcpNoDelay() const;
    
    /// Initialize a local INET4 address, returns 0 on success, -1 otherwise.
    virtual int setLocalAddress(struct sockaddr_in& addr, int port);
    
    /// Initialize a remote INET4 address, returns 0 on success, -1 otherwise.
    virtual int setAddress(struct sockaddr_in& addr, const std::string& host, int port);
    
  private:
    friend class ServerSocket;
    int _sockfd;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /** @brief TCP/IP server socket.
   * This class implements a TCP/IP socket that waits for requests to come in over the network.
   * AF_INET connections following the IPv4 Internet protocol are supported.
   * @note TCP/IP sockets do not preserve record boundaries but SocketBuffer solves this problem.
   */
  class ServerSocket {
  public:
    /** @brief Creates a new server socket.
     * Creates a listening socket that waits for connection requests by TCP/IP clients.
     */
    ServerSocket();
    
    virtual ~ServerSocket();
    
    /** @brief Accepts a new connection request and returns the corresponding socket.
     * By default, this function blocks the caller until a connection is present.
     * @return the new Socket or nullptr on error.
     */
    virtual Socket* accept();
    
    /** @brief Assigns the socket to the local address.
     * The socket must be bound before using it.
     * @return 0 on success or a negative value on error which is one of Socket::Errors
     */
    virtual int bind(int port, int backlog = 50);
    
    /// Closes the socket.
    virtual int close();
    
    /// Returns true if the socket has been closed.
    bool isClosed() const {return _sockfd < 0;}
    
    /// Returns the Unix descriptor of the socket.
    int descriptor() {return _sockfd;}
    
    /// Sets the SO_RCVBUF option to the specified value.
    int setReceiveBufferSize(int size);
    
    /// Enables/disables the SO_REUSEADDR socket option.
    int setReuseAddress(bool);
    
    /// Enables/disables SO_TIMEOUT with the specified timeout (in milliseconds).
    int	setSoTimeout(int timeout);
    
    /// Turns on/off TCP coalescence (useful in some cases to avoid delays).
    int setTcpNoDelay(bool);
    
  protected:
    virtual Socket* createSocket(int sockfd);
    
  private:
    int _sockfd;  // listening socket.
    ServerSocket(const ServerSocket&) = delete;
    ServerSocket& operator=(const ServerSocket&) = delete;
    ServerSocket& operator=(ServerSocket&&) = delete;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /** @brief Preserves record boundaries when exchanging messages between connected TCP/IP sockets.
   *
   * This class ensures that one call to readLine() corresponds to one and exactly one call
   * to writeLine() on the other side.
   * This differs from the behavior of Socket::send() and Socket::receive() because TCP/IP
   * connected sockets do not preserve record boundaries. writeLine() and readLine() solve
   * this problem by automatically adding/searching for a *message separator* between 
   * successive messages.
   *
   * By default, writeLine() automatically adds a \\n separator at the end of each
   * message and readLine() searches for \\n, \\r or \\n\\r for separating messages.
   * Message separators can be changed by calling setReadSeparator() and setWriteSeparator()
   * accordingly on both sides. Obviously, messages should not contain the *message separator*. 
   * If this is the case, readLine() will be called as many times as the number of occurences
   * of the message separator in the message given to writeLine().
   *
   * Exemple:
   @code
   int main() {
     Socket sock;
     SocketBuffer sockbuf(sock);

     int status = sock.connect("localhost", 3331);
     if (status < 0) {
       cerr << "Could not connect" << endl;
       return 1;
     }

     while (cin) {
       string request, response;
   
       cout << "Request: ";
       getline(cin, request);
   
       if (sockbuf.writeLine(request) < 0) {
         cerr << "Could not send message" << endl;
         return 2;
       }
   
       if (sockbuf.readLine(response) < 0) {
         cerr << "Couldn't receive message" << endl;
         return 3;
       }
     }
     return 0;
   }
   @endcode
   */
  class SocketBuffer {
  public:
    /** @brief Constructor.
     * _socket_ must be a connected TCP/IP Socket (i.e. of SOCK_STREAM type).
     * This socket must **not** be deleted while the SocketBuffer is used.
     *
     * _inputBufferSize_ and _ouputBufferSize_ are the sizes of the buffers that 
     * are used internally for exchanging data.
     */
    SocketBuffer(Socket* socket, size_t inputBufferSize = 8192, size_t ouputBufferSize = 8192);
    SocketBuffer(Socket& socket, size_t inputBufferSize = 8192, size_t ouputBufferSize = 8192);
    
    virtual ~SocketBuffer();
    
    /** @brief Read a message from a connected socket.
     * readLine() receives one (and only one) message sent by writeLine() on the other side.
     * The message is stored in _message_. This method blocks until the message 
     * is fully received.
     *
     * A call to readLine() corresponds to one and exactly one call to writeLine() 
     * on the other side. For this purpose, readLine() searches for a *message separator*
     * (see setReadSeparator() and setWriteSeparator()). By default, readLine() 
     * searches for \\n, \\r or \\n\\r.
     *
     * @return The number of bytes that were received or one of the following values:
     * - 0: shutdownOutput() was called on the other side
     * - Socket::Failed (-1): a connection error occured
     * - Socket::InvalidSocket (-2): the socket is invalid.
     *
     * Note that the message separator is counted in the value returned by readLine().
     */
    virtual ssize_t readLine(std::string& message);
    
    /** @brief Send a message to a connected socket.
     * writeLine() sends a message that will be received by a single call to readLine()
     * on the other side.
     *
     * A call to writeLine() corresponds to one and exactly one call to readLine()
     * on the other side. For this purpose, writeLine() automatically adds a *message separator*
     * (see setReadSeparator() and setWriteSeparator()). By default, writeLine()
     * adds the \\n character.
     *
     * @return The number of bytes that were received or one of the following values:
     * - 0: shutdownInput() was called on the other side
     * - Socket::Failed (-1): a connection error occured
     * - Socket::InvalidSocket (-2): the socket is invalid.
     *
     * Note that the message separator is counted in the value returned by writeLine().
     *
     * @note if _message_ constains one or several occurences of the message separator,
     * readLine() will be called as many times on the other side.
     */
    virtual ssize_t writeLine(const std::string& message);
    
    /* Receives a given number of characters from a connected socket.
     * Reads *exactly* _len_ bytes from the socket, blocks otherwise.
     *
     * @return the number of bytes that were received or:
     * - 0: shutdownOutput() was called on the other side
     * - Socket::Failed (-1): a connection error occured
     * - Socket::InvalidSocket (-2): the socket is invalid.
     */
    virtual ssize_t read(char* buffer, size_t len);
    
    /* @brief Sends a given number of characters to a connected socket.
     * Writes _len_ bytes to the socket.
     *
     * @return the number of bytes that were sent or:
     * - 0: shutdownInput() was called on the other side
     * - Socket::Failed (-1): a connection error occured
     * - Socket::InvalidSocket (-2): the socket is invalid.
     */
    virtual ssize_t write(const char* str, size_t len);
    
    /// Return the associated socket.
    Socket* socket() {return _sock;}
    
    /// Return the message separator used by readLine().
    int readSeparator() const {return _inSep;}
    
    /// Return the message separator used by writeLine().
    int writeSeparator() const {return _outSep;}
    
    /** @brief Change the message separator used by readLine().
     * This function changes the character(s) used by readLine() to separate successive messages:
     * - if _separ_ >= 0, readLine() searches for this character to separate messages,
     * - if _separ_ < 0 (the default) readLine() searches for \\n, \\r or \\n\\r.
     *
     * Obviously, writeLine() must use the same message separator on the
     * other side of the socket (see setWriteSeparator())
     */
    virtual void setReadSeparator(int separ);
    
    /** @brief Change the message separator used by writeLine().
     * This function changes the character(s) used by writeLine() to separate successive messages:
     * - if _separ_ >= 0, writeLine() inserts _separ_ between successive lines,
     * - if _separ_ < 0 (the default) writeLine() inserts \\n\\r between successive lines.
     *
     * Obviously, readLine() must use the same message separator on the
     * other side of the socket (see setReadSeparator())
     */
    virtual void setWriteSeparator(int separ);
    
  private:
    SocketBuffer(const SocketBuffer&) = delete;
    SocketBuffer& operator=(const SocketBuffer&) = delete;
    SocketBuffer& operator=(SocketBuffer&&) = delete;
    
  protected:
    virtual bool retrieveLine(std::string& str, ssize_t received);
    size_t _inSize, _outSize;
    int _inSep, _outSep;
    Socket* _sock;
    struct InputBuffer* _in;
  };
  
}

#endif
