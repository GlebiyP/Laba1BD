#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Checks.h"
#include "Carsale.h"

#define SELLER_IND "seller.ind"
#define SELLER_DATA "seller.fl"
#define SELLER_GARBAGE "seller_garbage.txt"
#define INDEXER_SIZE sizeof(struct Indexer)
#define SELLER_SIZE sizeof(struct Seller)

void noteDeletedSeller(int id)
{
	FILE* garbageZone = fopen(SELLER_GARBAGE, "rb");		// "rb": ��������� ������� ���� ��� �������

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	int* delIds = malloc(garbageCount * sizeof(int));		// �������� ���� �� ������ "�������" �������

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// ���������� ����
	}

	fclose(garbageZone);									// �� ��������� ��� ���� ������
	garbageZone = fopen(SELLER_GARBAGE, "wb");				// ������� ������� ���� � "������"
	fprintf(garbageZone, "%d", garbageCount + 1);			// �������� ���� ������� "�������" �������

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// �������� "�����" ������� �����...
	}

	fprintf(garbageZone, " %d", id);						// ...� �������� �� ��� ������ ����� ���������� ������
	free(delIds);											// ��������� ������� �� ����� ���'���
	fclose(garbageZone);									// ��������� ����
}

void overwriteGarbageId(int garbageCount, FILE* garbageZone, struct Seller* record)
{
	int* delIds = malloc(garbageCount * sizeof(int));		// �������� ���� �� ������ "�������" �������

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// ���������� ����
	}

	record->id = delIds[0];									// ��� ������ ������ ������ ���������� "��������"

	fclose(garbageZone);									// �� ��������� ��� ���� ������
	fopen(SELLER_GARBAGE, "wb");							// ������� ������� ���� � "������"
	fprintf(garbageZone, "%d", garbageCount - 1);			// �������� ���� ������� "�������" �������

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// �������� ����� "�������" �������
	}

	free(delIds);											// ��������� ������� �� ����� ���'���
	fclose(garbageZone);									// ��������� ����
}

int insertSeller(struct Seller record)
{
	FILE* indexTable = fopen(SELLER_IND, "a+b");			// "a+b": ������� ������� ����
	FILE* database = fopen(SELLER_DATA, "a+b");				// ��� ������ � ����� �� �������
	FILE* garbageZone = fopen(SELLER_GARBAGE, "rb");		// "rb": ������� ������� ���� ��� �������
	struct Indexer indexer;
	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)										// ����� "�����" ������, ���������� ������ � ���
	{
		overwriteGarbageId(garbageCount, garbageZone, &record);

		fclose(indexTable);									// ��������� ����� ��� ����
		fclose(database);									// ������ ������� � ����������

		indexTable = fopen(SELLER_IND, "r+b");				// ����� ��������� � ������� ����� ��
		database = fopen(SELLER_DATA, "r+b");				// "������� � �� ����� � ������� ���� �����"

		fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(database, indexer.address, SEEK_SET);			// ������� ������ �� "�����" ��� ���������� ����������	
	}
	else                                                    // ��������� ������ ����
	{
		long indexerSize = INDEXER_SIZE;

		fseek(indexTable, 0, SEEK_END);						// ������� ������ � ����� ����� ��������

		if (ftell(indexTable))								// ����� �������� �������� ���������� (������� �� �������)
		{
			fseek(indexTable, -indexerSize, SEEK_END);		// ������� ������ �� ������� ����������
			fread(&indexer, INDEXER_SIZE, 1, indexTable);	// ������ ������� ����������

			record.id = indexer.id + 1;						// �������� ����� ��������� ��������
		}
		else                                                // �������� �������� �������
		{
			record.id = 1;									// ��������� ��� ����� �� ������
		}
	}

	record.firstCarsaleAddress = -1;
	record.carsalesCount = 0;

	fwrite(&record, SELLER_SIZE, 1, database);				// �������� � ������� ���� ��-�������� �������� ���������

	indexer.id = record.id;									// ������� ����� ������ � ����������
	indexer.address = (record.id - 1) * SELLER_SIZE;		// ������� ������ ������ � ����������
	indexer.exists = 1;										// ��������� ��������� ������

	printf("Your seller\'s id is %d\n", record.id);

	fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// �������� ���������� � �������� ��������, ���� �����
	fclose(indexTable);										// ��������� �����
	fclose(database);

	return 1;
}

int getSeller(struct Seller* seller, int id, char* error)
{
	FILE* indexTable = fopen(SELLER_IND, "rb");				// "rb": ������� ������� ����
	FILE* database = fopen(SELLER_DATA, "rb");				// ����� ��� �������

	if (!checkFileExistence(indexTable, database, error))
	{
		return 0;
	}

	struct Indexer indexer;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// �������� ���������� �������� ������
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// �� �������� �������

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				// �������� ������� ����� � ��-��������
	fread(seller, sizeof(struct Seller), 1, database);		// �� ��������� �������
	fclose(indexTable);										// ��������� �����
	fclose(database);

	return 1;
}

int updateSeller(struct Seller seller, char* error)
{
	FILE* indexTable = fopen(SELLER_IND, "r+b");			// "r+b": ������� ������� ����
	FILE* database = fopen(SELLER_DATA, "r+b");				// ��� ������� �� ������

	if (!checkFileExistence(indexTable, database, error))
	{
		return 0;
	}

	struct Indexer indexer;
	int id = seller.id;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// �������� ���������� �������� ������
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// �� �������� �������

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				// ������������� �� ������� ������ � ��
	fwrite(&seller, SELLER_SIZE, 1, database);				// ��������� �����
	fclose(indexTable);										// ��������� �����
	fclose(database);

	return 1;
}

int deleteSeller(int id, char* error)
{
	FILE* indexTable = fopen(SELLER_IND, "r+b");			// "r+b": ������� ������� ����
															// ��� ������� �� ������	
	if (indexTable == NULL)
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	struct Seller seller;
	getSeller(&seller, id, error);

	struct Indexer indexer;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// �������� ���������� �������� ������
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// �� �������� �������

	indexer.exists = 0;										// ����� ������ �� ����������...

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);

	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// ...��� ������� ���� ��������
	fclose(indexTable);										// ��������� ���� [NB: ���� �� �������, �������� �� ���������]

	noteDeletedSeller(id);									// �������� ������ ���������� ������ �� "������ ����"


	if (seller.carsalesCount)								// ���� ������ ����, ��������� ��
	{
		FILE* carsalesDb = fopen(CARSALE_DATA, "r+b");
		struct Carsale carsale;

		fseek(carsalesDb, seller.firstCarsaleAddress, SEEK_SET);

		for (int i = 0; i < seller.carsalesCount; i++)
		{
			fread(&carsale, CARSALE_SIZE, 1, carsalesDb);
			fclose(carsalesDb);
			deleteCarsale(seller ,carsale, carsale.carId, error);

			carsalesDb = fopen(CARSALE_DATA, "r+b");
			fseek(carsalesDb, carsale.nextAddress, SEEK_SET);
		}

		fclose(carsalesDb);
	}
	return 1;
}