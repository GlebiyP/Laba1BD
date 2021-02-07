#pragma once
#include <stdio.h>
#include "Seller.h"
#include "Structures.h"

void printMaster(struct Seller seller)
{
	printf("Seller\'s name: %s\n", seller.name);
	printf("Seller\'s city: %d\n", seller.city);
}

void printCarsale(struct Carsale carsale, struct Seller seller)
{
	printf("Seller: %s, %d scores\n", seller.name, seller.city);
	printf("Customer: %d\n", carsale.customerId);
	printf("Car: %d\n", carsale.carId);
}