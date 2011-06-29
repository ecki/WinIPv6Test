// based on getaddrinfo() sample code from MSDN
// Modifications: Bernd Eckenfels <bernd-2011@eckenfels.net>

#undef UNICODE

#include "targetver.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

int __cdecl main(int argc, char **argv)
{
    WSADATA wsaData;
    int iResult;
    INT iRetval;

    DWORD dwRetval;

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    struct sockaddr_in  *sockaddr_ipv4;
//    struct sockaddr_in6 *sockaddr_ipv6;
    LPSOCKADDR sockaddr_ip;

    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;


    // Validate the parameters
    if (argc < 2) {
        printf("usage: %s <hostname>\n", argv[0]);
        printf("getaddrinfo provides protocol-independent translation\n");
        printf("   from an ANSI host name to an IP address\n");
        printf("%s example usage\n", argv[0]);
        printf("   %s www.heise.de\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    //--------------------------------
    // Setup the hints address info structure
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_CANONNAME; /* AI_ADDRCONFIG | AI_ALL; */

    printf("Calling getaddrinfo with following parameters:\n");
    printf("\tnodename = %s\n", argv[1]);
	printf("\tflags    = 0x%04x\n\n", hints.ai_flags);
    
	// Call getaddrinfo().
    dwRetval = getaddrinfo(argv[1], NULL, &hints, &result);
    if ( dwRetval != 0 ) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
		goto error_out;
    }

    printf("getaddrinfo returned success\n");
    
    // Retrieve each address and print out the hex bytes
	int expected_length = 0;
    int i = 1;
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tAddress: ");
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                printf("Unspecified (addrlene=%d)\n", ptr->ai_addrlen);
                break;
            case AF_INET:
				expected_length = 16;
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                printf("AF_INET (IPv4) %s\n",
					inet_ntoa(sockaddr_ipv4->sin_addr)); // ntohs(sockaddr_ipv4->sin_port);
				if (ptr->ai_addrlen != 16)
					printf("Warning: unexpected addrlen=%d\n", ptr->ai_addrlen);
                break;
            case AF_INET6:
                // the InetNtop function is available on Windows Vista and later
                // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                // printf("\tIPv6 address %s\n",
                //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );
                
                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
                    ipstringbuffer, &ipbufferlength );
                if (iRetval)
                    printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
                else    
                    printf("AF_INET6 (IPv6) %s\n", ipstringbuffer);
				if (ptr->ai_addrlen != 28)
					printf("Warning: unexpected addrlen=%d\n", ptr->ai_addrlen);
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS) (addrlen=%d)\n", ptr->ai_addrlen);
                break;
            default:
                printf("Other %ld (addrlen=%d)\n", ptr->ai_family, ptr->ai_addrlen);
                break;
        }

		if (ptr->ai_canonname)
			printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }

    freeaddrinfo(result);
    WSACleanup();

    return 0;

error_out:
    WSACleanup();
    return 1;
}
