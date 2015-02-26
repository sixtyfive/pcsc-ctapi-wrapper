/*
 * pcsc-ctapi-wrapper.c - v0.2
 * (c) 2005 Patrick Schlangen <info@b1g.de>
 * (c) 2008 Michael Braun <michael-dev@fami-braun.de>
 *
 * Allows you to use your PCSC-only-cardreader in a CTAPI-only-application
 * Tested with Moneyplex + PCSC-lite + SCM SCR243 PCMCIA + T1-HBCI-card
 *             StarMoney on Linux + PCSC-lite + Chipdrive micro 130
 * May not have a complete and correct CT-BCS implementation
 *
 * -----------------------------------------------------------------------------
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <wintypes.h>		// provided by pcsc-lite
#include <winscard.h>		// provided by pcsc-lite
#include <string.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL 
#define NULL 0
#endif 
#ifndef bool 
#define bool char 
#endif 

#define CTAPI_OK			0
#define CTAPI_ERR_INVALID	-1
#define CTAPI_ERR_CT		-8
#define CTAPI_ERR_TRANS		-10
#define CTAPI_ERR_MEMORY	-11
#define CTAPI_ERR_HOST		-127
#define CTAPI_ERR_HTSI		-128

typedef unsigned char IU8;
typedef char IS8;
typedef unsigned short IU16;

int myCtn = -1;							// my CTN
SCARDCONTEXT hContext;					// PCSC context
SCARDHANDLE hCard;						// PCSC card handle
bool bConnected = FALSE;				// connected?
char *szReaders;						// reader list

bool ConnectCard()
{
	DWORD dwActiveProtocol = -1;
	long rv;
	#ifdef DEBUG
	printf("ConnectCard()");
	#endif 
	rv = SCardConnect(hContext, szReaders, SCARD_SHARE_EXCLUSIVE,
			SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);
	if(rv == SCARD_S_SUCCESS)
	{
		#ifdef DEBUG
		printf(": true\n");
		#endif 
		bConnected = TRUE;
		return(TRUE);
	}
	else
	{
		#ifdef DEBUG
		printf(": false (%s)\n",pcsc_stringify_error(rv));
		#endif 
		bConnected = FALSE;
		return(FALSE);
	}
}

IS8 CT_init(IU16 ctn, IU16 pn)
{
	char *ptr;
	long rv;
	DWORD dwReaders;
	int nbReaders;
	
	#ifdef DEBUG
	printf("CT_init: Called (ctn %d, pn %d)\n",
				ctn, pn);
	#endif 
	
	// initialize PCSC handle
	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	if(rv != SCARD_S_SUCCESS)
	{
		#ifdef DEBUG
		printf("CT_init: SCardEstablishContext failed (0x%08X) (%s)\n",
					rv, pcsc_stringify_error(rv));
		#endif
		return(CTAPI_ERR_CT);
	}
	
	// get count of available readers
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	if(rv != SCARD_S_SUCCESS)
	{
		#ifdef DEBUG
		printf("CT_init: SCardListReaders failed (0x%08X) (%s)\n",
					rv, pcsc_stringify_error(rv));
		#endif
		return(CTAPI_ERR_CT);
	}
	
	// allocate memory for reader list 
	szReaders = (char *)malloc(dwReaders);
	if(szReaders == NULL)
	{
		#ifdef DEBUG
		printf("CT_init: Out of memory\n");
		#endif 
		return(CTAPI_ERR_MEMORY);
	}
	
	// get reader list 
	rv = SCardListReaders(hContext, NULL, szReaders, &dwReaders);
	if(rv != SCARD_S_SUCCESS)
	{
		#ifdef DEBUG
		printf("CT_init: SCardListReaders failed (0x%08X) (%s)\n",
					rv, pcsc_stringify_error(rv));
		#endif
		return(CTAPI_ERR_CT);
	}
	
	// get readers count
	// only counts until first reader name is ended
	// as nameA\0NameB\0NameC\0\0 is returned
	nbReaders = 0;
	ptr = szReaders;
        if (*ptr == '\0') {
		// no readers found?
		#ifdef DEBUG
		printf("CT_init: No readers found\n");
		#endif
		return(CTAPI_ERR_CT);
        } else {
	   // as long as we did not reach end of multi-reader list
	   while(*ptr != '\0')
  	    {
		#ifdef DEBUG
		printf("CT_init: processing \"%s\", counter=%d \n",ptr,nbReaders);
		#endif
		nbReaders++;
		if (nbReaders == pn) break;
		ptr += strlen(ptr)+1;
            }
	}   
	if (*ptr != '\0') {
 	  char* temp = malloc(strlen(ptr)+1);
	  strcpy(temp,ptr);
	  if (szReaders != NULL) free(szReaders);
	  szReaders = temp;
        }

	#ifdef DEBUG
	printf("CT_init: selected reader: \"%s\"\n",szReaders);
	#endif
 	
	bConnected = FALSE;
	myCtn = ctn;
	return(CTAPI_OK);
}

IS8 CT_data(IU16 ctn, IU8 *dad, IU8 *sad, IU16 lenc, IU8 *command, IU16 *lenr, IU8 *response)
{
	long rv;
	
	#ifdef DEBUG
	printf("CT_data: Called (ctn %d, dad %d, sad %d, lenc %d, lenr %d)\n",
				ctn, *dad, *sad, lenc, *lenr);
	printf("\tCommand: ");
	int i;
	for(i=0; i<lenc; i++)
		printf("%02X", command[i]);
	printf("\n");
	#endif

	// check parameters
	if((ctn != myCtn) || (dad == NULL) || (sad == NULL) || (command == NULL)
		|| (lenr == NULL) || (response == NULL) || (lenc < 4) || (*lenr < 2)
		|| (*sad != 2))
	{
		#ifdef DEBUG
		printf("CT_data: Invalid parameters\n");
		#endif
		return(CTAPI_ERR_INVALID);
	}
	
	switch(*dad)
	{
	case 1:
		// command goes to chipcard terminal
		if(command[0] == 0x20)
		{
			// CT-BCS command 
			switch(command[1])
			{
			case 0x11:
				// reset CT
				if(command[2] == 0x00)
				{
					#ifdef DEBUG 
					printf("CT_data: reset CT - no action\n");
					#endif
					// not needed, just return OK
					*lenr = 2;
					response[0] = 0x90;
					response[1] = 0x00;
					*sad = *dad;
					*dad = 0x02;
					return(CTAPI_OK);
				}
				else if(bConnected)
				{
					DWORD dwActiveProtocol;
					rv = SCardReconnect(hCard, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, SCARD_RESET_CARD, &dwActiveProtocol);
					if(rv != SCARD_S_SUCCESS)
					{
						#ifdef DEBUG
						printf("CT_data: SCardReconnect failed (0x%08X) (%s)\n",
									rv,pcsc_stringify_error(rv));
						#endif
						return(CTAPI_ERR_CT);
					}
					
					if(command[3] == 0x01)
					{
						// return atr
						BYTE pbAtr[MAX_ATR_SIZE];
						DWORD dwZero = 0, dwProtocol, dwAtrSize = sizeof(pbAtr);
						rv = SCardStatus(hCard, NULL, &dwZero, &dwZero, &dwProtocol, pbAtr, &dwAtrSize);
						if(rv != SCARD_S_SUCCESS)
						{
							#ifdef DEBUG 
							printf("CT_data: SCardStatus failed (0x%08X) (%s)\n",
										rv, pcsc_stringify_error(rv));
							#endif
							return(CTAPI_ERR_CT);
						}
						if(2 + dwAtrSize > *lenr)
						{
							#ifdef DEBUG 
							printf("CT_data: ERR at %s:%d\n",__FILE__,__LINE__);
							#endif
							return(CTAPI_ERR_MEMORY);
						}
						else
						{
							*lenr = dwAtrSize + 2;
							memcpy(response, pbAtr, dwAtrSize);
							response[dwAtrSize] = 0x90;
							response[dwAtrSize+1] = 0x00;
							*sad = *dad;
							*dad = 0x02;
							#ifdef DEBUG 
							printf("CT_data: return 0x9000 at %s:%d\n",__FILE__,__LINE__);
							#endif
							return(CTAPI_OK);
						}
					}
					else if(command[3] == 0x02)
					{
						// return HB
						#ifdef DEBUG 
						printf("CT_data: Historical bytes not supported\n");
						#endif 
						return(CTAPI_ERR_INVALID);
					}
					else 
					{
						#ifdef DEBUG
						printf("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
						#endif
						// return nothing
						*lenr = 2;
						response[0] = 0x90;
						response[1] = 0x00;
						return(CTAPI_OK);
					}
				}
				break;
			
			case 0x12:
				// request ICC
				if(!bConnected && !ConnectCard())
				{
					#ifdef DEBUG 
					printf("CT_data: ERR: card not connect or cannot connect to card.\n");
					#endif
					*lenr = 2;
					response[0] = 0x62;
					response[1] = 0x00;
					*sad = *dad;
					*dad = 0x02;
					return(CTAPI_OK);
				}
				
				BYTE pbAtr[MAX_ATR_SIZE];
				DWORD dwZero = 0, dwState, dwProtocol, dwAtrSize = sizeof(pbAtr);
				rv = SCardStatus(hCard, NULL, &dwZero, &dwState, &dwProtocol, pbAtr, &dwAtrSize);
			        if (rv != SCARD_S_SUCCESS) {
					#ifdef DEBUG
					printf("SCardState not successfull with error %X (%s) at %s:%d\n",rv, pcsc_stringify_error(rv), __FILE__,__LINE__);
					#endif
					rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
					#ifdef DEBUG
					printf("  Disconnect: %s\n",pcsc_stringify_error(rv));
					#endif
					ConnectCard();
				        dwZero = 0, dwState, dwProtocol, dwAtrSize = sizeof(pbAtr);
					rv = SCardStatus(hCard, NULL, &dwZero, &dwState, &dwProtocol, pbAtr, &dwAtrSize);
					#ifdef DEBUG
					if (rv != SCARD_S_SUCCESS) 
						printf("SCardState (II.) not successfull with error %X (%s) at %s:%d\n",rv, pcsc_stringify_error(rv), __FILE__,__LINE__);
					#endif
				}
				
				// card present?
				if(dwState &= SCARD_PRESENT)
				{
					if(command[3] == 0x01)
					{
						if(2 + dwAtrSize > *lenr)
						{
							#ifdef DEBUG
							printf("CT_data: ERR at %s:%d\n",__FILE__,__LINE__);
							#endif
							return(CTAPI_ERR_MEMORY);
						}
						else
						{
							#ifdef DEBUG
							printf("CT_data: result\n  atr: %X\n  state:%X\n  Proto:%X\n   at %s:%d\n",response, dwState, dwProtocol, __FILE__,__LINE__);
							#endif
							*lenr = dwAtrSize + 2;
							memcpy(response, pbAtr, dwAtrSize);
							response[dwAtrSize] = 0x90;
							response[dwAtrSize+1] = 0x00;
							*sad = *dad;
							*dad = 0x02;
							return(CTAPI_OK);
						}
					}
					else if(command[3] == 0x02)
					{
						// return HB
						#ifdef DEBUG 
						printf("CT_data: Historical bytes not supported\n");
						#endif 
						return(CTAPI_ERR_INVALID);
					}
					else
					{
						#ifdef DEBUG 
						printf("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
						#endif
						*lenr = 2;
						response[0] = 0x90;
						response[1] = 0x00;
						*sad = *dad;
						*dad = 0x02;
						return(CTAPI_OK);
					}
				}
				else
				{
					#ifdef DEBUG 
					printf("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
					#endif
					*lenr = 2;
					response[0] = 0x62;
					response[1] = 0x00;
					*sad = *dad;
					*dad = 0x02;
					return(CTAPI_OK);
				}					
				break;
				
			case 0x15:
				#ifdef DEBUG 
				printf("CT_data: eject\n");
				#endif
				// eject ICC
				if(bConnected)
					SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
				*lenr = 2;
				response[0] = 0x90;
				response[1] = 0x00;
				*sad = *dad;
				*dad = 0x02;
				return(CTAPI_OK);
				break;
			};
		}
		break;
	
	default:
		// command goes to card
		if(!bConnected && !ConnectCard())
		{
			#ifdef DEBUG 
			printf("CT_data: ERR: no card. at %s:%d\n",__FILE__,__LINE__);
			#endif
			*lenr = 2;
			response[0] = 0x62;
			response[1] = 0x00;
			*sad = *dad;
			*dad = 0x02;
			return(CTAPI_OK);
		}
		
		#ifdef DEBUG 
		printf("CT_data: Sending command to card terminal\n");
		#endif			
		
		SCARD_IO_REQUEST pioRecvPci;
		DWORD dwRecvLength = *lenr, dwInpLength = lenc;
		rv = SCardTransmit(hCard, SCARD_PCI_T1, command, dwInpLength, &pioRecvPci, response, &dwRecvLength);
		if(rv != SCARD_S_SUCCESS)
		{
			#ifdef DEBUG
			printf("CT_data: SCardTransmit failed (0x%08x) (%s)\n",
					rv, pcsc_stringify_error(rv));
			#endif
			return(CTAPI_ERR_INVALID);
		}
		
		#ifdef DEBUG 
		printf("CT_data: Received answer:\n\t");
		int i;
		for(i=0; i<dwRecvLength; i++)
			printf("%02X", response[i]);
		printf("\n");
		#endif
		
		*lenr = dwRecvLength;
		*sad = *dad;
		*dad = 0x02;
		return(CTAPI_OK);
		break;
	};
	
	#ifdef DEBUG
	printf("CT_data: Request not handled\n");
	#endif
	return(CTAPI_ERR_INVALID);
}

IS8 CT_close(IU16 ctn)
{
	#ifdef DEBUG
	printf("CT_close: Called (ctn %d)\n",
				ctn);
	#endif 
	
	if(ctn == myCtn)
	{
		if(bConnected)
			SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
		if(szReaders != NULL)
			free(szReaders);
		SCardReleaseContext(hContext);
		myCtn = -1;
		return(CTAPI_OK);
	}
	else
	{
		return(CTAPI_ERR_INVALID);
	}
}
