#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <libsocket/inetclientdgram.hpp>
#include <libsocket/exception.hpp>
#include <netinet/in.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

int main(int argc, char* argv[])
{
	if (argc == 1) return 1;
	// Structure that defines the 48 byte NTP packet protocol.
	string remoteIP;
	string remotePort;

	remoteIP.resize(16);
	remotePort.resize(16);

	typedef struct
	{

		unsigned li : 2;       // Only two bits. Leap indicator.
		unsigned vn : 3;       // Only three bits. Version number of the protocol.
		unsigned mode : 3;       // Only three bits. Mode. Client will pick mode 3 for client.

		uint8_t stratum;         // Eight bits. Stratum level of the local clock.
		uint8_t poll;            // Eight bits. Maximum interval between successive messages.
		uint8_t precision;       // Eight bits. Precision of the local clock.

		uint32_t rootDelay;      // 32 bits. Total round trip delay time.
		uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
		uint32_t refId;          // 32 bits. Reference clock identifier.

		uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
		uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

		uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
		uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

		uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
		uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

		uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
		uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

	} ntp_packet;                 // Total: 384 bits or 48 bytes.

								  // Create and zero out the packet. All 48 bytes worth.

	ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	memset(&packet, 0, sizeof(ntp_packet));

	// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

	*((char *)&packet) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

							   // Create a UDP socket, convert the host-name to an IP address, set the port number,
							   // connect to the server, send the packet, and then read in the return packet.

	libsocket::inet_dgram_client sock(LIBSOCKET_IPv4);

	sock.sndto(&packet, 48, argv[1], "1230");

	sock.rcvfrom(&packet, 48, remoteIP, remotePort);


	// These two fields contain the time-stamp seconds as the packet left the NTP server.
	// The number of seconds correspond to the seconds passed since 1900.
	// ntohl() converts the bit/byte order from the network's to host's "endianness".

	packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
	packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

										  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
										  // Subtract 70 years worth of seconds from the seconds since 1900.
										  // This leaves the seconds since the UNIX epoch of 1970.
										  // (1900)------------------(1970)**************************************(Time Packet Left the Server)

	time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

	// Print the time we got from the server, accounting for local timezone and conversion from UTC time.
	struct timeval now;
	now.tv_sec = txTm;
	now.tv_usec = txTm_f;

	int rc = settimeofday(&now, NULL);
	if (rc == 0) {
		printf("settimeofday() successful.\n");
	}
	else {
		printf("settimeofday() failed, "
			"errno = %d\n", errno);
		return -1;
	}

	return 0;
}
