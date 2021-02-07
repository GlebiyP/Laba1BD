#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Structures.h"
#include "Checks.h"
#include "Seller.h"

#define CARSALE_DATA "carsale.fl"
#define CARSALE_GARBAGE "carsale_garbage.txt"
#define CARSALE_SIZE sizeof(struct Carsale)

void reopenDatabase(FILE* database)
{
	fclose(database);
	database = fopen(CARSALE_DATA, "r+b");
}

void linkAddresses(FILE* database, struct Seller seller, struct Carsale carsale)
{
	reopenDatabase(database);						// ������� ����� �� "������� � �� ����� � ����-��� ����"

	struct Carsale previous;

	fseek(database, seller.firstCarsaleAddress, SEEK_SET);

	for (int i = 0; i < seller.carsalesCount; i++)		    // ���������� ��'������ ������ �� ���������� ������� ����
	{
		fread(&previous, CARSALE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}

	previous.nextAddress = carsale.selfAddress;				// ��'����� ������
	fwrite(&previous, CARSALE_SIZE, 1, database);				// �������� ��������� ����� ����� �� �����
}

void relinkAddresses(FILE* database, struct Carsale previous, struct Carsale carsale, struct Seller* seller)
{
	if (carsale.selfAddress == seller->firstCarsaleAddress)		// ���� ����������� (������ �����)...
	{
		if (carsale.selfAddress == carsale.nextAddress)			// ...�� � ���� ���������� (����� ���� ����)
		{
			seller->firstCarsaleAddress = -1;					// ��������� ������ ��� �������
		}
		else                                                // ...� ��������� �,
		{
			seller->firstCarsaleAddress = carsale.nextAddress;  // ������ ���� ������
		}
	}
	else                                                    // ���������� �...
	{
		if (carsale.selfAddress == carsale.nextAddress)			// ...��� ���� ���������� (������� �����)
		{
			previous.nextAddress = previous.selfAddress;    // ������ ���������� �������
		}
		else                                                // ... � ����� � ��� � ���������
		{
			previous.nextAddress = carsale.nextAddress;		// ������ ��������� ����������� �����������
		}

		fseek(database, previous.selfAddress, SEEK_SET);	// �������� ��������� ����������
		fwrite(&previous, CARSALE_SIZE, 1, database);			// ����� �� ��������
	}
}

void noteDeletedCarsale(long address)
{
	FILE* garbageZone = fopen(CARSALE_GARBAGE, "rb");			// "rb": ��������� ������� ���� ��� �������

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	long* delAddresses = malloc(garbageCount * sizeof(long)); // �������� ���� �� ������ "�������" �����

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%ld", delAddresses + i);		// ���������� ����
	}

	fclose(garbageZone);									// �� ��������� ��� ���� ������
	garbageZone = fopen(CARSALE_GARBAGE, "wb");				// ������� ������� ���� � "������"
	fprintf(garbageZone, "%ld", garbageCount + 1);			// �������� ���� ������� "�������" �����

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %ld", delAddresses[i]);		// �������� "�����" ������ �����...
	}

	fprintf(garbageZone, " %d", address);					// ...� �������� �� ��� ������ ����� ���������� ������
	free(delAddresses);										// ��������� ������� �� ����� ���'���
	fclose(garbageZone);									// ��������� ����
}

void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Carsale* record)
{
	long* delIds = malloc(garbageCount * sizeof(long));		// �������� ���� �� ������ "�������" �����

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// ���������� ����
	}

	record->selfAddress = delIds[0];						// ��� ������ ������ ������ �������� "������"
	record->nextAddress = delIds[0];

	fclose(garbageZone);									// �� ��������� ��� ���� ������
	fopen(CARSALE_GARBAGE, "wb");							    // ������� ������� ���� � "������"
	fprintf(garbageZone, "%d", garbageCount - 1);			// �������� ���� ������� "�������" �����

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// �������� ����� "�������" �����
	}

	free(delIds);											// ��������� ������� �� ����� ���'���
	fclose(garbageZone);									// ��������� ����
}

int insertCarsale(struct Seller seller, struct Carsale carsale, char* error)
{
	carsale.exists = 1;

	FILE* database = fopen(CARSALE_DATA, "a+b");
	FILE* garbageZone = fopen(CARSALE_GARBAGE, "rb");

	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)											// ����� ������� ������
	{
		overwriteGarbageAddress(garbageCount, garbageZone, &carsale);
		reopenDatabase(database);								// ������� ����� ������� �����
		fseek(database, carsale.selfAddress, SEEK_SET);			// ������� ������ �� "�����" ��� ���������� ����������
	}
	else                                                        // ��������� ����, ������ � ����� �����
	{
		fseek(database, 0, SEEK_END);

		int dbSize = ftell(database);

		carsale.selfAddress = dbSize;
		carsale.nextAddress = dbSize;
	}

	fwrite(&carsale, CARSALE_SIZE, 1, database);					// �������� ������ ���� �� ����� �����

	if (!seller.carsalesCount)								    // ������� ���� �� ����, ������ ������ �������
	{
		seller.firstCarsaleAddress = carsale.selfAddress;
	}
	else                                                        // ������ ���� ��� �, ��������� "������ ����������" ����������
	{
		linkAddresses(database, seller, carsale);
	}

	fclose(database);											// ��������� ����

	seller.carsalesCount++;										// ����� �� ���� ������ ���� �����
	updateSeller(seller, error);								// ��������� ����� ��������

	return 1;
}

int getCarsale(struct Seller seller, struct Carsale* carsale, int carId, char* error)
{
	if (!seller.carsalesCount)									// � ������� ���� ������� ����
	{
		strcpy(error, "This master has no slaves yet");
		return 0;
	}

	FILE* database = fopen(CARSALE_DATA, "rb");


	fseek(database, seller.firstCarsaleAddress, SEEK_SET);		// �������� ������ �����
	fread(carsale, CARSALE_SIZE, 1, database);

	for (int i = 0; i < seller.carsalesCount; i++)				// ������ �������� ����� �� ���� ����
	{
		if (carsale->carId == carId)						// �������
		{
			fclose(database);
			return 1;
		}

		fseek(database, carsale->nextAddress, SEEK_SET);
		fread(carsale, CARSALE_SIZE, 1, database);
	}

	strcpy(error, "No such slave in database");					// �� �������
	fclose(database);
	return 0;
}

// �� ���� �������� ������ ���� � ���������� ����������, ��� ����� �������� � ����
int updateCarsale(struct Carsale carsale, int carId)
{
	FILE* database = fopen(CARSALE_DATA, "r+b");

	fseek(database, carsale.selfAddress, SEEK_SET);
	fwrite(&carsale, CARSALE_SIZE, 1, database);
	fclose(database);

	return 1;
}

int deleteCarsale(struct Seller seller, struct Carsale carsale, int carId, char* error)
{
	FILE* database = fopen(CARSALE_DATA, "r+b");
	struct Carsale previous;

	fseek(database, seller.firstCarsaleAddress, SEEK_SET);

	do		                                                    // ������ ����������� ������ (���� ���� � �� ����,
	{															// ��� � ����������� ���������� ��� �����)
		fread(&previous, CARSALE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	} while (previous.nextAddress != carsale.selfAddress && carsale.selfAddress != seller.firstCarsaleAddress);

	relinkAddresses(database, previous, carsale, &seller);
	noteDeletedCarsale(carsale.selfAddress);						// �������� ������ ���������� ������ � "������"

	carsale.exists = 0;											// ������ �� ����������

	fseek(database, carsale.selfAddress, SEEK_SET);				// ��� �������
	fwrite(&carsale, CARSALE_SIZE, 1, database);					// �������� �����
	fclose(database);

	seller.carsalesCount--;										// �������� ������� ������� ���� �� ����
	updateSeller(seller, error);

}