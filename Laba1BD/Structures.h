#pragma once
struct Seller
{
	int id;
	char name[16];
	char city[16];
	long firstCarsaleAddress;
	int carsalesCount;
};

struct Carsale
{
	int sellerId;
	int customerId;
	int carId;
	int exists;
	long selfAddress;
	long nextAddress;
};

struct Indexer
{
	int id;
	int address;
	int exists;
};