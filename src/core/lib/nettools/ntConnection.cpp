
#include "ntConnection.h"
#include "ntEventHandler.h"
#include <mstcpip.h>  // for tcp_keepalive structure
#include <cstdio>     // for printf

namespace nettools {

	static NT_ERROR wsa_to_nt_error(int wsa_error) {
		switch (wsa_error) {

		case WSAEWOULDBLOCK:
			return NTERR_WOULDBLOCK;
		case WSAEADDRINUSE:
			return NTERR_ADDRINUSE;
		case WSAECONNREFUSED:
			return NTERR_CONNREFUSED;
		}

		return NTERR_OTHER;
	}

	static NT_ERROR wsa_error_or_success(int result) {
		if (result == SOCKET_ERROR)
			return wsa_to_nt_error(::WSAGetLastError());
		return NTERR_SUCCESS;
	}



	NT_ERROR ntConnection::_socket(int *socket) {
		int s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		*socket = s;
		return NTERR_SUCCESS;
	}

	NT_ERROR ntConnection::_bind(int socket, sockaddr *address, u32 len) {
		int result = ::bind(socket, address, len);
		return wsa_error_or_success(result);
	}

	NT_ERROR ntConnection::_listen(int socket) {
		int result = ::listen(socket, 10);
		return wsa_error_or_success(result);
	}

	NT_ERROR ntConnection::_accept(int socket, ntConnection **newClient) {
		sockaddr_in clientAddr;
		int addrLen = sizeof(sockaddr_in);
		int newSocket = ::accept(m_socket, (sockaddr*)&clientAddr, &addrLen);
		
		NT_ERROR err = wsa_error_or_success(newSocket);
		if (err == NTERR_SUCCESS) {
			*newClient = new ntConnection(newSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));
			// Set socket options for new connection
			(*newClient)->setOption();
		}
		else {
			*newClient = NULL;
		}
		return err;
	}

	NT_ERROR ntConnection::_connect(int socket, sockaddr *address, u32 len) {
		int res = ::connect(socket, address, len);
		return wsa_error_or_success(res);
	}

	NT_ERROR ntConnection::_send(int socket, const u8 *data, u32 len) {
		int result = ::send(socket, (const char*)data, len, 0);
		return wsa_error_or_success(result);
	}

	NT_ERROR ntConnection::_recv(int socket, u8 *data, u32 maxlen, u32 *recvLen) {
		int result = ::recv(socket, (char*)data, maxlen, 0);
		NT_ERROR err = wsa_error_or_success(result);
		if (err == NTERR_SUCCESS) {
			*recvLen = result;
		}
		return err;
	}

	NT_ERROR ntConnection::_close(int socket) {
		::closesocket(socket);
		return NTERR_SUCCESS;
	}

	ntConnection::ntConnection() {
		WSADATA wsaData;
		int result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		_socket(&m_socket);
		setBlockingMode(false);
		m_boundData = NULL;
	}

	ntConnection::ntConnection(int sock, sockaddr *address, u32 len) {
		m_socket = sock;
		m_address = *(sockaddr_in*)address;
	}

	ntConnection::~ntConnection() {
		close();
		int result = ::WSACleanup();
	}

	NT_ERROR ntConnection::setBlockingMode(bool blocking) {
		u_long mode = blocking ? 0 : 1;
		int result = ::ioctlsocket(m_socket, FIONBIO, &mode);
		return wsa_error_or_success(result);
	}

	NT_ERROR ntConnection::getOption() {
		// Reserved for getting socket options
		return NTERR_SUCCESS;
	}

	NT_ERROR ntConnection::setOption() {
		// 1. Disable Nagle algorithm to reduce latency (important for game servers)
		int nodelay = 1;
		if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, 
		               (char*)&nodelay, sizeof(nodelay)) == SOCKET_ERROR) {
			printf("[WARN] Failed to set TCP_NODELAY: %d\n", WSAGetLastError());
		}
		
		// 2. Enable TCP Keep-Alive to detect zombie connections
		int keepalive = 1;
		if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, 
		               (char*)&keepalive, sizeof(keepalive)) == SOCKET_ERROR) {
			printf("[WARN] Failed to set SO_KEEPALIVE: %d\n", WSAGetLastError());
		}
		
		// 3. Set Keep-Alive parameters (Windows specific)
		tcp_keepalive alive;
		alive.onoff = 1;                  // Enable
		alive.keepalivetime = 30000;      // Start probing after 30s of no data
		alive.keepaliveinterval = 5000;   // Probe interval 5s
		
		DWORD bytesReturned;
		if (WSAIoctl(m_socket, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
		             NULL, 0, &bytesReturned, NULL, NULL) == SOCKET_ERROR) {
			printf("[WARN] Failed to set Keep-Alive parameters: %d\n", WSAGetLastError());
		}
		
		// 4. Set send buffer size (optional, adjust as needed)
		int sendbuf = 64 * 1024; // 64KB
		if (setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,
		               (char*)&sendbuf, sizeof(sendbuf)) == SOCKET_ERROR) {
			printf("[WARN] Failed to set send buffer size: %d\n", WSAGetLastError());
		}
		
		// 5. Set receive buffer size (optional, adjust as needed)
		int recvbuf = 64 * 1024; // 64KB
		if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF,
		               (char*)&recvbuf, sizeof(recvbuf)) == SOCKET_ERROR) {
			printf("[WARN] Failed to set receive buffer size: %d\n", WSAGetLastError());
		}
		
		return NTERR_SUCCESS;
	}

	NT_ERROR ntConnection::send(int socket, const u8 *data, u32 len) {
		return _send(socket, data, len);
	}

	NT_ERROR ntConnection::recv(int socket, u8 *data, u32 maxlen, u32 *recvLen) {
		return _recv(socket, data, maxlen, recvLen);
	}

	NT_ERROR ntConnection::poll() {
		return NTERR_SUCCESS;
	}

	NT_ERROR ntConnection::close() {
		::shutdown(m_socket, SD_BOTH);
		return _close(m_socket);
	}

}
