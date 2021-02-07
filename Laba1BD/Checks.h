#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Carsale.h"

int checkFileExistence(FILE* indexTable, FILE* database, char* error)
{
	if (indexTable == NULL || database == NULL)				// Файли БД ще не існують
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	return 1;
}

int checkIndexExistence(FILE* indexTable, char* error, int id)
{
	fseek(indexTable, 0, SEEK_END);

	long indexTableSize = ftell(indexTable);

	if (indexTableSize == 0 || id * sizeof(struct Indexer) > indexTableSize)
	{
		strcpy(error, "no such ID in table");				// Такого номеру в табличці нема
		return 0;
	}

	return 1;
}

int checkRecordExistence(struct Indexer indexer, char* error)
{
	if (!indexer.exists)									// Запис було вилучено
	{
		strcpy(error, "the record you\'re looking for has been removed");
		return 0;
	}

	return 1;
}

int checkKeyPairUniqueness(struct Seller seller, int carId)
{
	FILE* carsalesDb = fopen(CARSALE_DATA, "r+b");
	struct Carsale carsale;

	fseek(carsalesDb, seller.firstCarsaleAddress, SEEK_SET);

	for (int i = 0; i < seller.carsalesCount; i++)
	{
		fread(&carsale, CARSALE_SIZE, 1, carsalesDb);
		fclose(carsalesDb);

		if (carsale.carId == carId)
		{
			return 0;
		}

		carsalesDb = fopen(CARSALE_DATA, "r+b");
		fseek(carsalesDb, carsale.nextAddress, SEEK_SET);
	}

	fclose(carsalesDb);

	return 1;
}

void info()
{
	FILE* indexTable = fopen("seller.ind", "rb");

	if (indexTable == NULL)
	{
		printf("Error: database files are not created yet\n");
		return;
	}

	int sellerCount = 0;
	int carsaleCount = 0;

	fseek(indexTable, 0, SEEK_END);
	int indAmount = ftell(indexTable) / sizeof(struct Indexer);

	struct Seller seller;

	char dummy[51];

	for (int i = 1; i <= indAmount; i++)
	{
		if (getSeller(&seller, i, dummy))
		{
			sellerCount++;
			carsaleCount += seller.carsalesCount;

			printf("Seller #%d has %d carsale(s)\n", i, seller.carsalesCount);
		}
	}

	fclose(indexTable);

	printf("Total masters: %d\n", sellerCount);
	printf("Total slaves: %d\n", carsaleCount);
}