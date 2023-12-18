#include <stdio.h>
#include <stdlib.h>

#include <winscard.h>
#include <iostream>
#pragma comment(lib, "Winscard")

#define MAX_READER_NAME_SIZE 40

#ifndef MAX_ATR_SIZE
#define MAX_ATR_SIZE 33
#endif

bool contextEstablished(DWORD sCardScope, LPSCARDCONTEXT sCardContext) {
	printf("SCardEstablishContext : ");
	if (SCardEstablishContext(sCardScope, NULL, NULL, sCardContext) != SCARD_S_SUCCESS) {
		printf("failed\n");
		return false;
	}
	else {
		printf("success\n");
		return true;
	}
}

bool cardReadersCollected(SCARDCONTEXT sCardContext, LPCWSTR sReaderGroups, LPWSTR sReturnReaders, LPDWORD sBufferLength) {
	printf("SCardListReaders : ");

	if (SCardListReaders(sCardContext, sReaderGroups, sReturnReaders, sBufferLength) != SCARD_S_SUCCESS) {
		printf("failed\n");
		return false;
	}
	else {
		printf("success\n");
		return true;
	}
}

bool connectedToReader(SCARDCONTEXT sCardContext, LPCWSTR sReaderTarget, LPSCARDHANDLE sCardConnectionHandler, LPDWORD sActiveProtocol) {
	printf("SCardConnect : ");

	if (SCardConnect(sCardContext, sReaderTarget,
		SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		sCardConnectionHandler, sActiveProtocol) != SCARD_S_SUCCESS) {
		printf("failed\n");
		return false;
	}
	else {
		printf("success\n");
		return true;
	}
}

bool receivedCardStatus(SCARDHANDLE sCardHandle, LPWSTR sReaderNamesOut, LPDWORD sReaderLength, LPDWORD sSmartState, LPDWORD sCurrentProtocol, LPBYTE sPointerATR, LPDWORD sATRLength) {
	printf("SCardStatus : ");

	if (SCardStatus(sCardHandle, sReaderNamesOut, sReaderLength, sSmartState,
		sCurrentProtocol, sPointerATR, sATRLength) != SCARD_S_SUCCESS) {
		printf("failed\n");
		return false;
	}
	else {
		printf("success\n");
		return true;
	}
}

bool commandTransitSuccessful(SCARDHANDLE sCardHandle, LPCBYTE sBufferPointer, DWORD sSendLength, LPBYTE sReturnPointer, LPDWORD sLength) {
	printf("SCardTransmit : ");

	if (SCardTransmit(sCardHandle, SCARD_PCI_T0, sBufferPointer,
		sSendLength, NULL, sReturnPointer, sLength) != SCARD_S_SUCCESS) {
		printf("failed\n");
		return false;
	}
	else {
		printf("success\n");
		return true;
	}
}

int main(int argc, char** argv)
{
	SCARDCONTEXT hContext;
	SCARDHANDLE hCard;

	SCARD_READERSTATE_A rgReaderStates[1];

	DWORD dwReaderLen, dwState, dwProt, dwAtrLen;
	DWORD dwPref, dwReaders, dwRespLen;

	LPWSTR pcReaders;

	LPWSTR mszReaders;
	LPWSTR* mszReadersP;

	BYTE pbAtr[MAX_ATR_SIZE];

	BYTE pbResp1[30];
	BYTE pbResp2[30];
	BYTE pbResp3[30];
	BYTE pbResp4[30];
	BYTE pbResp5[200];

	LPCWSTR mszGroups;
	LONG rv;

	int i, p, iReader;
	int iReaders[16];

	//komendy


	//zarzadca
	if (!contextEstablished(SCARD_SCOPE_SYSTEM, &hContext))
		return -1;

	// pobranie wielkoœci ci¹gu, jaki bêdzie potrzebny na listê 
	// dostêpnych czytników w systemie
	mszGroups = 0;
	
	if (!cardReadersCollected(hContext, mszGroups, 0, &dwReaders)) {
		SCardReleaseContext(hContext);
		return -1;
	}

	// alokacja pamiêci
	// czy to bd to samo?
	mszReadersP = new LPWSTR[dwReaders];
	mszReaders = (LPWSTR)malloc(sizeof(char) * dwReaders);

	// pobranie listy czytników
	if (!cardReadersCollected(hContext, mszGroups, mszReaders, &dwReaders)) {
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	// wydruk listy znalezionych czytników
	p = 0;
	for (i = 0; i < dwReaders - 1; ++i)	{
		iReaders[++p] = i;
		std::cout << "Reader " << p << ": " << &mszReaders[i] << std::endl;
		// printf("Reader %02d: %s\n", p, &mszReaders[i]);
		// przesuniêcie bufora do kolejnej nazwy czytnika
		while (mszReaders[++i] != '\0');
	}

	do {
		printf("Select reader : ");
		std::cin >> iReader;
		// scanf("%d", &iReader);
	} while (iReader > p || iReader <= 0);


	//iReader = 1;
	// nawi¹zanie po³¹czenia z czytnikiem
	if (!connectedToReader(hContext, &mszReaders[iReaders[iReader]], &hCard, &dwPref)) {
		SCardReleaseContext(hContext);
		free(mszReaders); 
		delete[] mszReadersP;
		return -1;
	}

	// sprawdzenie stanu karty w czytniku
	printf("SCardStatus : ");
	dwReaderLen = MAX_READER_NAME_SIZE;
	pcReaders = (LPWSTR)malloc(sizeof(char) * MAX_READER_NAME_SIZE);

	if (!receivedCardStatus(hCard, pcReaders, &dwReaderLen, &dwState,
		&dwProt, pbAtr, &dwAtrLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		free(pcReaders);
		return -1;
	}

	// wydruk pobranych informacji
	printf("Reader name : %s\n", pcReaders);
	printf("Reader state : %lx\n", dwState);
	printf("Reader protocol : %lx\n", dwProt - 1);
	printf("Reader ATR size : %d\n", dwAtrLen);
	printf("Reader ATR value : ");

	// wydruk ATR
	for (i = 0; i < dwAtrLen; i++)
	{
		printf("%02X ", pbAtr[i]);
	}
	printf("\n");
	//free(pcReaders);

	// rozpoczêcie transakcji z kart¹
	printf("SCardBeginTransaction : ");
	if (SCardBeginTransaction(hCard) != SCARD_S_SUCCESS) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		printf("failed\n");
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}
	else printf("success\n");


	BYTE SELECT_TELECOM[] = { 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x7F, 0x10 };
	// przes³anie do karty komendy 
	dwRespLen = 30;

	if (!commandTransitSuccessful(hCard, SELECT_TELECOM, 7,
		pbResp1, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("Response APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp1[i]);
	}
	printf("\n");


	BYTE GET_RESPONSE[] = { 0xA0, 0xC0, 0x00, 0x00, 0x1A };
	// przes³anie do karty komendy 
	dwRespLen = 30;

	if (!commandTransitSuccessful(hCard, GET_RESPONSE, 5,
		pbResp2, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("Response APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp2[i]);
	}
	printf("\n");

	BYTE SELECT_SMS[] = { 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x6F, 0x3C };
	BYTE SELECT_CONTACTS[] = { 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x6F, 0x3A };
	// przes³anie do karty komendy 
	dwRespLen = 30;

	if (!commandTransitSuccessful(hCard, SELECT_SMS, 7,
		pbResp3, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("Response APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp3[i]);
	}
	printf("\n");


	BYTE GET_RESPONSE2[] = { 0xA0, 0xC0, 0x00, 0x00, 0x0F };
	// przes³anie do karty komendy
	dwRespLen = 30;

	if (!commandTransitSuccessful(hCard, GET_RESPONSE2, 5,
		pbResp4, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("Response APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp4[i]);
	}
	printf("\n");

	BYTE READ_RECORD[] = { 0xA0, 0xB2, 0x01, 0x04, 0xB0 };
	for (int i = 1; i < 4; i++) {
		// przes³anie do karty komendy
		dwRespLen = 178;
		READ_RECORD[2] = i;

		if (!commandTransitSuccessful(hCard, READ_RECORD, 5,
			pbResp5, &dwRespLen)) {
			SCardDisconnect(hCard, SCARD_RESET_CARD);
			SCardReleaseContext(hContext);
			free(mszReaders);
			delete[] mszReadersP;
			return -1;
		}

		std::cout << "\nSMS" << i << "\nResponse APDU : ";
		// printf("\nSMS1\nResponse APDU : ");

		// wydruk odpowiedzi karty
		for (i = 0; i < dwRespLen; i++)
		{
			printf("%02X ", pbResp5[i]);
		}
		printf("\n");
	}
	
	/*
	// przes³anie do karty komendy
	dwRespLen = 178;

	if (!commandTransitSuccessful(hCard, READ_RECORD, 5,
		pbResp5, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("\nSMS1\nResponse APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp5[i]);
	}
	printf("\n");
	//-----------------------------------

	BYTE READ_RECORD2[] = { 0xA0, 0xB2, 0x02, 0x04, 0xB0 };
	// przes³anie do karty komendy 
	dwRespLen = 178;

	if (!commandTransitSuccessful(hCard, READ_RECORD2, 5,
		pbResp5, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("\nSMS2\nResponse APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp5[i]);
	}
	printf("\n");


	BYTE READ_RECORD3[] = { 0xA0, 0xB2, 0x03, 0x04, 0xB0 };
	// przes³anie do karty komendy 
	dwRespLen = 178;

	if (!commandTransitSuccessful(hCard, READ_RECORD3, 5,
		pbResp5, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("\nSMS3\nResponse APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp5[i]);
	}
	printf("\n");
	//}


	BYTE READ_RECORD4[] = { 0xA0, 0xB2, 0x04, 0x04, 0xB0 };
	// przes³anie do karty komendy 
	dwRespLen = 178;

	if (!commandTransitSuccessful(hCard, READ_RECORD4, 5,
		pbResp5, &dwRespLen)) {
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}

	printf("\nSMS4\nResponse APDU : ");

	// wydruk odpowiedzi karty
	for (i = 0; i < dwRespLen; i++)
	{
		printf("%02X ", pbResp5[i]);
	}
	printf("\n");
	*/

	// zakoñczenie transakcji z kart¹
	printf("SCardEndTransaction : ");
	rv = SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
	if (rv != SCARD_S_SUCCESS)
	{
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		SCardReleaseContext(hContext);
		printf("failed\n");
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}
	else printf("success\n");

	// od³¹czenie od czytnika
	printf("SCardDisconnect : ");
	rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);

	if (rv != SCARD_S_SUCCESS)
	{
		SCardReleaseContext(hContext);
		printf("failed\n");
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}
	else printf("success\n");

	// od³¹czenie od zarz¹dcy zasobów PC/SC
	printf("SCardReleaseContext : ");
	rv = SCardReleaseContext(hContext);

	if (rv != SCARD_S_SUCCESS)
	{
		printf("failed\n");
		free(mszReaders);
		delete[] mszReadersP;
		return -1;
	}
	else printf("success\n");

	//free(mszReaders);
	return 0;
}