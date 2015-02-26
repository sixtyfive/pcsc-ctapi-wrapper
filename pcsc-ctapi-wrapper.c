/*
 * pcsc-ctapi-wrapper.c - v0.3
 * (c) 2005 Patrick Schlangen <info@b1g.de>
 * (c) 2008 Michael Braun <michael-dev@fami-braun.de>
 * (c) 2008 Mathias Kosch <info@mkosch.de>
 *
 * Allows you to use your PCSC-only-cardreader in a CTAPI-only-application
 * Tested with Moneyplex + PCSC-lite + SCM SCR243 PCMCIA + T1-HBCI-card
 *             StarMoney on Linux + PCSC-lite + Chipdrive micro 130
 *             Hibiscus on Linux + PCSC-lite + SPR 532/CHIPDRIVE pinpad 532 (using pinpad)
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
#include <reader.h>			// provided by pcsc-lite
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <arpa/inet.h>





#define CTAPI_OK			0
#define CTAPI_ERR_INVALID	-1
#define CTAPI_ERR_CT		-8
#define CTAPI_ERR_TRANS		-10
#define CTAPI_ERR_MEMORY	-11
#define CTAPI_ERR_HOST		-127
#define CTAPI_ERR_HTSI		-128


#ifdef DEBUG
	#define DEBUG_MESSAGE(format, args...) \
			fprintf(stderr, format, ##args)
#else
	#define DEBUG_MESSAGE(format, args...)
#endif





typedef unsigned char IU8;
typedef char IS8;
typedef unsigned short IU16;





int myCtn = -1;							// my CTN
SCARDCONTEXT hContext;					// PCSC context
SCARDHANDLE hCard;						// PCSC card handle
bool bConnected = false;				// connected?
char *szReaders;						// reader list





extern IS8 CT_init(IU16 ctn, IU16 pn);


extern IS8 CT_data(IU16 ctn, IU8 *dad, IU8 *sad, IU16 lenc, IU8 *command, IU16 *lenr, IU8 *response);


extern IS8 CT_close(IU16 ctn);


static bool ConnectCard();


static IS8 PerformVerification(IU16 ctn, IU8 *dad, IU8 *sad, IU16 lenc, IU8 *command, IU16 *lenr, IU8 *response);





extern IS8 CT_init(IU16 ctn, IU16 pn)
{
	char *ptr;
	long rv;
	DWORD dwReaders;
	int nbReaders;
	
	DEBUG_MESSAGE("CT_init: Called (ctn %d, pn %d)\n",
				ctn, pn);
	
	// Handling the case one card terminal has already been opened:
	if (myCtn != -1)
	{
		DEBUG_MESSAGE("CT_init: Card terminal has already been opened. Closing at first.\n");
		
		// Simulatneous operations on more than one terminal is curently not supported:
		if (ctn != myCtn)
		{
			DEBUG_MESSAGE("CT_init: Opening more than one card terminal at once not supported.\n");
			return CTAPI_ERR_CT;
		}
		
		// Closing terminal:
		if (CT_close(myCtn) != CTAPI_OK)
		{
			DEBUG_MESSAGE("CT_init: Failed to close previously opened card terminal.\n");
			return CTAPI_ERR_CT;
		}
	}
	
	// initialize PCSC handle
	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	if(rv != SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE("CT_init: SCardEstablishContext failed (0x%08lX) (%s)\n",
					rv, pcsc_stringify_error(rv));
		return(CTAPI_ERR_CT);
	}
	
	// get count of available readers
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	if(rv != SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE("CT_init: SCardListReaders failed (0x%08lX) (%s)\n",
					rv, pcsc_stringify_error(rv));
		return(CTAPI_ERR_CT);
	}
	
	// allocate memory for reader list 
	szReaders = (char *)malloc(dwReaders);
	if(szReaders == NULL)
	{
		DEBUG_MESSAGE("CT_init: Out of memory\n");
		return(CTAPI_ERR_MEMORY);
	}
	
	// get reader list 
	rv = SCardListReaders(hContext, NULL, szReaders, &dwReaders);
	if(rv != SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE("CT_init: SCardListReaders failed (0x%08lX) (%s)\n",
					rv, pcsc_stringify_error(rv));
		return(CTAPI_ERR_CT);
	}
	
	// get readers count
	// only counts until first reader name is ended
	// as nameA\0NameB\0NameC\0\0 is returned
	nbReaders = 0;
	ptr = szReaders;
        if (*ptr == '\0') {
		// no readers found?
		DEBUG_MESSAGE("CT_init: No readers found\n");
		return(CTAPI_ERR_CT);
        } else {
	   // as long as we did not reach end of multi-reader list
	   while(*ptr != '\0')
  	    {
		DEBUG_MESSAGE("CT_init: processing \"%s\", counter=%d \n",ptr,nbReaders);
		nbReaders++;
#ifdef USE_PORT_BASE1
		if (nbReaders == pn) break;
#endif
#ifdef USE_PORT_BASE0
		if (nbReaders-1 == pn) break;
#endif
#ifdef USE_CTN_BASE1
		if (nbReaders == ctn) break;
#endif
#ifdef USE_CTN_BASE0
		if (nbReaders-1 == ctn) break;
#endif
		ptr += strlen(ptr)+1;
            }
	}   
	if (*ptr != '\0') {
 	  char* temp = malloc(strlen(ptr)+1);
	  strcpy(temp,ptr);
	  if (szReaders != NULL) free(szReaders);
	  szReaders = temp;
        }

	DEBUG_MESSAGE("CT_init: selected reader: \"%s\"\n",szReaders);
 	
	bConnected = false;
	myCtn = ctn;
	return(CTAPI_OK);
}


extern IS8 CT_data(IU16 ctn, IU8 *dad, IU8 *sad, IU16 lenc, IU8 *command, IU16 *lenr, IU8 *response)
{
	long rv;
	
#ifdef DEBUG
	DEBUG_MESSAGE("CT_data: Called (ctn %d, dad %d, sad %d, lenc %d, lenr %d)\n",
				ctn, *dad, *sad, lenc, *lenr);
	DEBUG_MESSAGE("\tCommand: ");
	int i;
	for(i=0; i<lenc; i++)
		DEBUG_MESSAGE("%02X", command[i]);
	DEBUG_MESSAGE("\n");
#endif

	// check parameters
	if((ctn != myCtn) || (dad == NULL) || (sad == NULL) || (command == NULL)
		|| (lenr == NULL) || (response == NULL) || (lenc < 4) || (*lenr < 2)
		|| (*sad != 2))
	{
		DEBUG_MESSAGE("CT_data: Invalid parameters\n");
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
					DEBUG_MESSAGE("CT_data: reset CT - no action\n");
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
						DEBUG_MESSAGE("CT_data: SCardReconnect failed (0x%08lX) (%s)\n",
									rv,pcsc_stringify_error(rv));
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
							DEBUG_MESSAGE("CT_data: SCardStatus failed (0x%08lX) (%s)\n",
										rv, pcsc_stringify_error(rv));
							return(CTAPI_ERR_CT);
						}
						if(2 + dwAtrSize > *lenr)
						{
							DEBUG_MESSAGE("CT_data: ERR at %s:%d\n",__FILE__,__LINE__);
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
							DEBUG_MESSAGE("CT_data: return 0x9000 at %s:%d\n",__FILE__,__LINE__);
							return(CTAPI_OK);
						}
					}
					else if(command[3] == 0x02)
					{
						// return HB
						DEBUG_MESSAGE("CT_data: Historical bytes not supported\n");
						return(CTAPI_ERR_INVALID);
					}
					else 
					{
						DEBUG_MESSAGE("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
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
					DEBUG_MESSAGE("CT_data: ERR: card not connect or cannot connect to card.\n");
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
					DEBUG_MESSAGE("SCardState not successfull with error %lX (%s) at %s:%d\n",rv, pcsc_stringify_error(rv), __FILE__,__LINE__);
					rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
					DEBUG_MESSAGE("  Disconnect: %s\n",pcsc_stringify_error(rv));
					ConnectCard();
				        dwZero = 0, dwState, dwProtocol, dwAtrSize = sizeof(pbAtr);
					rv = SCardStatus(hCard, NULL, &dwZero, &dwState, &dwProtocol, pbAtr, &dwAtrSize);
					if (rv != SCARD_S_SUCCESS)
						DEBUG_MESSAGE("SCardState (II.) not successfull with error %lX (%s) at %s:%d\n",rv, pcsc_stringify_error(rv), __FILE__,__LINE__);
				}
				
				// card present?
				if(dwState &= SCARD_PRESENT)
				{
					if(command[3] == 0x01)
					{
						if(2 + dwAtrSize > *lenr)
						{
							DEBUG_MESSAGE("CT_data: ERR at %s:%d\n",__FILE__,__LINE__);
							return(CTAPI_ERR_MEMORY);
						}
						else
						{
							DEBUG_MESSAGE("CT_data: result\n  atr: %p\n  state:%lX\n  Proto:%lX\n   at %s:%d\n",response, dwState, dwProtocol, __FILE__,__LINE__);
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
						DEBUG_MESSAGE("CT_data: Historical bytes not supported\n");
						return(CTAPI_ERR_INVALID);
					}
					else
					{
						DEBUG_MESSAGE("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
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
					DEBUG_MESSAGE("CT_data: dummy result at %s:%d\n",__FILE__,__LINE__);
					*lenr = 2;
					response[0] = 0x62;
					response[1] = 0x00;
					*sad = *dad;
					*dad = 0x02;
					return(CTAPI_OK);
				}					
				break;
				
			case 0x15:
				DEBUG_MESSAGE("CT_data: eject\n");
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
				
			/*
			 * PERFORM VERIFICATION command:
			 */
			case 0x18:
				return PerformVerification(ctn, dad, sad, lenc, command, lenr, response);
			}
		}
		break;
	
	default:
		// command goes to card
		if(!bConnected && !ConnectCard())
		{
			DEBUG_MESSAGE("CT_data: ERR: no card. at %s:%d\n",__FILE__,__LINE__);
			*lenr = 2;
			response[0] = 0x62;
			response[1] = 0x00;
			*sad = *dad;
			*dad = 0x02;
			return(CTAPI_OK);
		}
		
		DEBUG_MESSAGE("CT_data: Sending command to card terminal\n");
		
		SCARD_IO_REQUEST pioRecvPci;
		DWORD dwRecvLength = *lenr, dwInpLength = lenc;
		rv = SCardTransmit(hCard, SCARD_PCI_T1, command, dwInpLength, &pioRecvPci, response, &dwRecvLength);
		if(rv != SCARD_S_SUCCESS)
		{
			DEBUG_MESSAGE("CT_data: SCardTransmit failed (0x%08lx) (%s)\n",
					rv, pcsc_stringify_error(rv));
			return(CTAPI_ERR_INVALID);
		}
		
#ifdef DEBUG 
		DEBUG_MESSAGE("CT_data: Received answer:\n\t");
		int i;
		for(i=0; i<dwRecvLength; i++)
			DEBUG_MESSAGE("%02X", response[i]);
		DEBUG_MESSAGE("\n");
#endif
		
		*lenr = dwRecvLength;
		*sad = *dad;
		*dad = 0x02;
		return(CTAPI_OK);
	};
	
	DEBUG_MESSAGE("CT_data: Request not handled\n");
	return(CTAPI_ERR_INVALID);
}


extern IS8 CT_close(IU16 ctn)
{
	DEBUG_MESSAGE("CT_close: Called (ctn %d)\n",
				ctn);
	
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


static bool ConnectCard()
{
	DWORD dwActiveProtocol = -1;
	long rv;
	DEBUG_MESSAGE("ConnectCard()");
	rv = SCardConnect(hContext, szReaders, SCARD_SHARE_EXCLUSIVE,
			SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);
	if(rv == SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE(": true\n");
		bConnected = true;
		return(true);
	}
	else
	{
		DEBUG_MESSAGE(": false (%s)\n",pcsc_stringify_error(rv));
		bConnected = false;
		return(false);
	}
}


static IS8 PerformVerification(IU16 ctn, IU8 *dad, IU8 *sad, IU16 lenc, IU8 *command, IU16 *lenr, IU8 *response)
{
	DEBUG_MESSAGE("CT_data: PERFORM VERIFICATION\n");
	
	// Currently only the pinpad is supported:
	if (command[3] != 0x00)
	{
		if (command[3] == 0x01)
			DEBUG_MESSAGE("CT_data: Biometric verification is not supported.\n");
		else
			DEBUG_MESSAGE("CT_data: Unknown command qualifier.\n");
		return CTAPI_ERR_INVALID;
	}
	if ((lenc < 5) || (lenc < 5+command[4]))
	{
		DEBUG_MESSAGE("CT_data: Invalid command length.\n");
		return CTAPI_ERR_INVALID;
	}
	
	// Reading DOs from data field:
	size_t nSize = command[4];
	uint8_t *pData = &command[5];
	bool bCommandToPerform = false;
	uint8_t nPinLength = 0, nPinInsertPos = 0, nPinBlockSize = 0;
	uint8_t nPinApduLength = 0, *pPinApdu = NULL;
	while ((nSize >= 2) && (nSize >= 2+pData[1]))
	{
		switch (pData[0])		// DO tag
		{
		case 0x52:		// Command-to-perform
			if ((bCommandToPerform) || (pData[1] < 2))
			{
				DEBUG_MESSAGE("CT_data: Invalid command.\n");
				return CTAPI_ERR_INVALID;
			}
			bCommandToPerform = true;
			
			// PIN length:
			nPinLength = (pData[2] & 0xF0) >> 4;
			
			// PIN encoding currently only supports BCD:
			if (pData[2] & 0x0F)
			{
				DEBUG_MESSAGE("CT_data: Requested PIN encoding not supported.\n");
				return CTAPI_ERR_INVALID;
			}
			
			// PIN insertion position:
			nPinInsertPos = pData[3];
			if ((nPinInsertPos < 6) || (nPinInsertPos > 0x0F))
			{
				DEBUG_MESSAGE("CT_data: PIN insertion position not supported.\n");
				return CTAPI_ERR_INVALID;
			}
			nPinInsertPos -= 6;		// New position starts at first byte after "Lc"
			
			// Command to be sent to ICC:
			nPinApduLength = pData[1]-2;
			pPinApdu = pData+4;
			if (nPinApduLength < 5)
			{
				DEBUG_MESSAGE("CT_data: Requested ICC command not supported.\n");
				return CTAPI_ERR_INVALID;
			}
			
			// Guessing PIN block size:
			nPinBlockSize = nPinApduLength-5-nPinInsertPos;
			if (nPinInsertPos > 0x0F)
			{
				DEBUG_MESSAGE("CT_data: PIN block size can't be guessed.\n");
				return CTAPI_ERR_INVALID;
			}
			break;
			
		default:
			DEBUG_MESSAGE("CT_data: DO %" PRIX8 " not handled.\n", pData[0]);
			break;
		}
		
		nSize -= pData[1]+2;
		pData += pData[1]+2;
	}
	if (nSize)
	{
		DEBUG_MESSAGE("CT_data: Invalid command length.\n");
		return CTAPI_ERR_INVALID;
	}
	if (!bCommandToPerform)		// "Command-to-perform" is mandatory
	{
		DEBUG_MESSAGE("CT_data: Command-to-perform missing.\n");
		return CTAPI_ERR_INVALID;
	}
	
	// Requesting ICC:
	if ((!bConnected) && (!ConnectCard()))
	{
		DEBUG_MESSAGE("CT_data: Cannot connect to card.\n");
		return CTAPI_ERR_INVALID;
	}
	
	size_t nPos;
	LONG nResult;
	DWORD dwControlCode = 0, nBytesReturned;
	uint8_t aOutput[1024];
	
	// Looking up supported features and their control codes.
	// Currently only "FEATURE_VERIFY_PIN_DIRECT" is supported.
	nResult = SCardControl(hCard, CM_IOCTL_GET_FEATURE_REQUEST, NULL, 0, aOutput, sizeof(aOutput), &nBytesReturned);
	if (nResult != SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE("CT_data: SCardControl(CM_IOCTL_GET_FEATURE_REQUEST) failed. (0x%08lx) (%s)\n", nResult, pcsc_stringify_error(nResult));
		return CTAPI_ERR_INVALID;
	}
	nPos = 0;
	bool bControlCodeSet = false;
	while (nPos < nBytesReturned)
	{
		if ((nBytesReturned-nPos < 6) || (aOutput[nPos+1] != 4))
		{
			DEBUG_MESSAGE("CT_data: SCardControl(CM_IOCTL_GET_FEATURE_REQUEST) returned unrecognized data.\n");
			return CTAPI_ERR_INVALID;
		}
		if (aOutput[nPos] == FEATURE_VERIFY_PIN_DIRECT)
		{
			dwControlCode = ntohl(*(uint32_t*)&aOutput[nPos+2]);
			bControlCodeSet = true;
		}
		nPos += 2+aOutput[nPos+1];
	}
	if (!bControlCodeSet)
	{
		DEBUG_MESSAGE("CT_data: Direct pin verification feature is not supported.\n");
		return CTAPI_ERR_INVALID;
	}
	DEBUG_MESSAGE("CT_data: Control code for FEATURE_VERIFY_PIN_DIRECT is %lu\n", dwControlCode);
	
	// Defining structure for PIN verification:
	PIN_VERIFY_STRUCTURE *pPVS = alloca(sizeof(PIN_VERIFY_STRUCTURE)+nPinApduLength);
	pPVS->bTimerOut = 15;
	pPVS->bTimerOut2 = 5;
	pPVS->bmFormatString = 0x81 | (nPinInsertPos << 3);		// ?
	pPVS->bmPINBlockString = nPinBlockSize;		// ?
	pPVS->bmPINLengthFormat = 0x10;		// ?
	pPVS->wPINMaxExtraDigit = (nPinLength) ?
			HOST_TO_CCID_16(((uint16_t)nPinLength << 8) | nPinLength) : HOST_TO_CCID_16(0x0408);
	pPVS->bEntryValidationCondition = 0x02;
	pPVS->bNumberMessage = 0x01;
	pPVS->wLangId = HOST_TO_CCID_16(0x0904);		// ?
	pPVS->bMsgIndex = 0x00;
	pPVS->bTeoPrologue[0] = 0x00;		// ?
	pPVS->bTeoPrologue[1] = 0x00;		// ?
	pPVS->bTeoPrologue[2] = 0x00;		// ?
	pPVS->ulDataLength = nPinApduLength;
	memcpy(&pPVS->abData, pPinApdu, nPinApduLength);
	
	// Performing verification:
	nBytesReturned = 0;
	nResult = SCardControl(hCard, dwControlCode, pPVS, sizeof(PIN_VERIFY_STRUCTURE)-sizeof(pPVS->abData)+nPinApduLength,
			aOutput, sizeof(aOutput), &nBytesReturned);
	if (nResult != SCARD_S_SUCCESS)
	{
		DEBUG_MESSAGE("CT_data: SCardControl failed. (0x%08lx) (%s)\n", nResult, pcsc_stringify_error(nResult));
		return CTAPI_ERR_INVALID;
	}
	if (nBytesReturned != 2)
	{
		DEBUG_MESSAGE("CT_data: Unexpected data output.\n");
		return CTAPI_ERR_INVALID;
	}
	if (aOutput[0] == 0x64)
	{
		switch (aOutput[1])
		{
		case 0x00:
			DEBUG_MESSAGE("CT_data: Operation timed out.\n");
			break;
		case 0x01:
			DEBUG_MESSAGE("CT_data: Operation canceled.\n");
			break;
		case 0x03:
			DEBUG_MESSAGE("CT_data: PIN entered is too short or too long.\n");
			aOutput[1] = 0x01;		// Mapping for CT-API
			break;
		}
	}
	else if ((aOutput[0] == 0x6B) && (aOutput[1] == 0x80))
		DEBUG_MESSAGE("CT_data: Invalid parameter.\n");
	else
		DEBUG_MESSAGE("CT_data: Result = (%" PRIu8 ", %" PRIu8 ")\n", aOutput[0], aOutput[1]);
	
	// Returning response:
	*lenr = 2;
	response[0] = aOutput[0];
	response[1] = aOutput[1];
	*sad = *dad;
	*dad = 0x02;
	return CTAPI_OK;
}
