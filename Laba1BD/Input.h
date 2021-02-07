#pragma once
#include <stdio.h>
#include <string.h>
#include "Structures.h"
#include "Seller.h"

void readSeller(struct Seller* seller)
{
	char name[16];
	char city[16];

	name[0] = '\0';

	printf("Enter seller\'s name: ");
	scanf("%s", name);

	strcpy(seller->name, name);

	printf("Enter seller\'s city: ");
	scanf("%d", &city);

	strcpy(seller->city, city);
}

void readCarsale(struct Carsale* carsale)
{
	int x;

	printf("Enter customer's ID: ");
	scanf("%d", &x);

	carsale->customerId = x;

	printf("Enter car's ID: ");
	scanf("%d", &x);

	carsale->carId = x;
}