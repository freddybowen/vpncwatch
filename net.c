/*
 * net.c
 * Network functions for vpncwatch
 * Modified by: Freddy Bowen <frederick.bowen@gmail.com>
 * Original Author: David Cantrell <dcantrell@redhat.com>
 *
 * Adapted from vpnc-watch.py by Gary Benson <gbenson@redhat.com>
 * (Python is TOO BIG for a 16M OpenWRT router.)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "vpncwatch.h"

int is_network_up(char *host, char *port) 
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;

    int s, sfd;

    /* don't do a network check if the user didn't specify a host */
    if (host == NULL || port == NULL) {
        return 1;
    }

 /* Obtain address(es) matching host/port */

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */

   s = getaddrinfo(host, port, &hints, &result);
   if (s != 0) {
       syslog(LOG_ERR, "getaddrinfo() error: %s", gai_strerror(s));
       return 0;
   }

	/* getaddrinfo() returns a list of address structures.
    	  Try each address until we successfully connect(2).
	      If socket(2) (or connect(2)) fails, we (close the socket
	      and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
	   sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	   if (sfd == -1)
	       continue;
	
	   if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
	       break;                  /* Success */
		else 
			syslog(LOG_ERR, "connect() failed: %s", strerror(errno));
	
	   close(sfd);
	}
	
	if (rp == NULL)                /* No address succeeded */
	   return 0;

	freeaddrinfo(result);           /* No longer needed */
	close(sfd);
	return 1;
}
