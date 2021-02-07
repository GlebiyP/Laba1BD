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
	reopenDatabase(database);						// Змінюємо режим на "читання з та запис у будь-яке місце"

	struct Carsale previous;

	fseek(database, seller.firstCarsaleAddress, SEEK_SET);

	for (int i = 0; i < seller.carsalesCount; i++)		    // Пробігаємомо зв'язаний список до останнього продажу авто
	{
		fread(&previous, CARSALE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}

	previous.nextAddress = carsale.selfAddress;				// Зв'язуємо адреси
	fwrite(&previous, CARSALE_SIZE, 1, database);				// Заносимо оновлений запис назад до файлу
}

void relinkAddresses(FILE* database, struct Carsale previous, struct Carsale carsale, struct Seller* seller)
{
	if (carsale.selfAddress == seller->firstCarsaleAddress)		// Немає попередника (перший запис)...
	{
		if (carsale.selfAddress == carsale.nextAddress)			// ...ще й немає наступника (запис лише один)
		{
			seller->firstCarsaleAddress = -1;					// Неможлива адреса для безпеки
		}
		else                                                // ...а наступник є,
		{
			seller->firstCarsaleAddress = carsale.nextAddress;  // робимо його першим
		}
	}
	else                                                    // Попередник є...
	{
		if (carsale.selfAddress == carsale.nextAddress)			// ...але немає наступника (останній запис)
		{
			previous.nextAddress = previous.selfAddress;    // Робимо попередник останнім
		}
		else                                                // ... а разом з ним і наступник
		{
			previous.nextAddress = carsale.nextAddress;		// Робимо наступник наступником попередника
		}

		fseek(database, previous.selfAddress, SEEK_SET);	// Записуємо оновлений попередник
		fwrite(&previous, CARSALE_SIZE, 1, database);			// назад до таблички
	}
}

void noteDeletedCarsale(long address)
{
	FILE* garbageZone = fopen(CARSALE_GARBAGE, "rb");			// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	long* delAddresses = malloc(garbageCount * sizeof(long)); // Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%ld", delAddresses + i);		// Заповнюємо його
	}

	fclose(garbageZone);									// За допомогою цих двох команд
	garbageZone = fopen(CARSALE_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%ld", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %ld", delAddresses[i]);		// Заносимо "сміттєві" адреси назад...
	}

	fprintf(garbageZone, " %d", address);					// ...і дописуємо до них адресу щойно видаленого запису
	free(delAddresses);										// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Carsale* record)
{
	long* delIds = malloc(garbageCount * sizeof(long));		// Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його
	}

	record->selfAddress = delIds[0];						// Для запису замість логічно видаленої "сміттєвої"
	record->nextAddress = delIds[0];

	fclose(garbageZone);									// За допомогою цих двох команд
	fopen(CARSALE_GARBAGE, "wb");							    // повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// Записуємо решту "сміттєвих" адрес
	}

	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int insertCarsale(struct Seller seller, struct Carsale carsale, char* error)
{
	carsale.exists = 1;

	FILE* database = fopen(CARSALE_DATA, "a+b");
	FILE* garbageZone = fopen(CARSALE_GARBAGE, "rb");

	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)											// Наявні видалені записи
	{
		overwriteGarbageAddress(garbageCount, garbageZone, &carsale);
		reopenDatabase(database);								// Змінюємо режим доступу файлу
		fseek(database, carsale.selfAddress, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису
	}
	else                                                        // Видалених немає, пишемо в кінець файлу
	{
		fseek(database, 0, SEEK_END);

		int dbSize = ftell(database);

		carsale.selfAddress = dbSize;
		carsale.nextAddress = dbSize;
	}

	fwrite(&carsale, CARSALE_SIZE, 1, database);					// Записуємо продаж авто до свого файлу

	if (!seller.carsalesCount)								    // Продажів авто ще немає, пишемо адресу першого
	{
		seller.firstCarsaleAddress = carsale.selfAddress;
	}
	else                                                        // Продажі авто вже є, оновлюємо "адресу наступника" останнього
	{
		linkAddresses(database, seller, carsale);
	}

	fclose(database);											// Закриваємо файл

	seller.carsalesCount++;										// Стало на один продаж авто більше
	updateSeller(seller, error);								// Оновлюємо запис продавця

	return 1;
}

int getCarsale(struct Seller seller, struct Carsale* carsale, int carId, char* error)
{
	if (!seller.carsalesCount)									// У продаця немає продажів авто
	{
		strcpy(error, "This master has no slaves yet");
		return 0;
	}

	FILE* database = fopen(CARSALE_DATA, "rb");


	fseek(database, seller.firstCarsaleAddress, SEEK_SET);		// Отримуємо перший запис
	fread(carsale, CARSALE_SIZE, 1, database);

	for (int i = 0; i < seller.carsalesCount; i++)				// Шукаємо потрібний запис по коду авто
	{
		if (carsale->carId == carId)						// Знайшли
		{
			fclose(database);
			return 1;
		}

		fseek(database, carsale->nextAddress, SEEK_SET);
		fread(carsale, CARSALE_SIZE, 1, database);
	}

	strcpy(error, "No such slave in database");					// Не знайшли
	fclose(database);
	return 0;
}

// На вхід подається продаж авто з оновленими значеннями, яку треба записати у файл
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

	do		                                                    // Шукаємо попередника запису (його може й не бути,
	{															// тоді в попередника занесеться сам запис)
		fread(&previous, CARSALE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	} while (previous.nextAddress != carsale.selfAddress && carsale.selfAddress != seller.firstCarsaleAddress);

	relinkAddresses(database, previous, carsale, &seller);
	noteDeletedCarsale(carsale.selfAddress);						// Заносимо адресу видаленого запису у "смітник"

	carsale.exists = 0;											// Логічно не існуватиме

	fseek(database, carsale.selfAddress, SEEK_SET);				// але фізично
	fwrite(&carsale, CARSALE_SIZE, 1, database);					// записуємо назад
	fclose(database);

	seller.carsalesCount--;										// Зменщуємо кількість продажів авто на один
	updateSeller(seller, error);

}