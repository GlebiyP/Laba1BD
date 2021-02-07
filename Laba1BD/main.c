#include <stdio.h>
#include "Structures.h"
#include "Seller.h"
#include "Carsale.h"
#include "Input.h"
#include "Output.h"

int main()
{
	struct Seller seller;
	struct Carsale carsale;

	while (1)
	{
		int choice;
		int id;
		char error[51];
		printf("Choose option:\n0 - Quit\n1 - Insert Seller\n2 - Get Seller\n3 - Update Seller\n4 - Delete Seller\n5 - Insert Carsale\n6 - Get Carsale\n7 - Update Carsale\n8 - Delete Carsale\n9 - Info\n");
		scanf("%d", &choice);
		switch (choice)
		{
		case 0:
			return 0;

		case 1:
			readSeller(&seller);
			insertSeller(seller);
			break;

		case 2:
			printf("Enter ID: ");
			scanf("%d", &id);
			getSeller(&seller, id, error) ? printSeller(seller) : printf("Error: %s\n", error);
			break;

		case 3:
			printf("Enter ID: ");
			scanf("%d", &id);

			seller.id = id;

			readSeller(&seller);
			updateSeller(seller, error) ? printf("Updated successfully\n") : printf("Error: %s\n", error);
			break;

		case 4:
			printf("Enter ID: ");
			scanf("%d", &id);
			deleteSeller(id, error) ? printf("Deleted successfully\n") : printf("Error: %s\n", error);
			break;

		case 5:
			printf("Enter seller\'s ID: ");
			scanf("%d", &id);

			if (getSeller(&seller, id, error))
			{
				carsale.sellerId = id;
				printf("Enter car ID: ");
				scanf("%d", &id);

				if (checkKeyPairUniqueness(seller, id))		// Унікальність по коду авто
				{
					carsale.carId = id;
					readCarsale(&carsale);
					insertCarsale(seller, carsale, error);
					printf("Inserted successfully. To access, use seller\'s and car\'s IDs\n");
				}
				else
				{
					printf("Error: non-unique car key\n");
				}
			}
			else
			{
				printf("Error: %s\n", error);
			}
			break;

		case 6:
			printf("Enter seller\'s ID: ");
			scanf("%d", &id);

			if (getSeller(&seller, id, error))
			{
				printf("Enter car ID: ");
				scanf("%d", &id);
				getCarsale(seller, &carsale, id, error) ? printCarsale(carsale, seller) : printf("Error: %s\n", error);
			}
			else
			{
				printf("Error: %s\n", error);
			}
			break;

		case 7:
			printf("Enter seller\'s ID: ");
			scanf("%d", &id);

			if (getSeller(&seller, id, error))
			{
				printf("Enter car ID: ");
				scanf("%d", &id);

				if (getCarsale(seller, &carsale, id, error))
				{
					readCarsale(&carsale);
					updateCarsale(carsale, id, error);
					printf("Updated successfully\n");
				}
				else
				{
					printf("Error: %s\n", error);
				}
			}
			else
			{
				printf("Error: %s\n", error);
			}
			break;

		case 8:
			printf("Enter seller\'s ID: ");
			scanf("%d", &id);

			if (getSeller(&seller, id, error))
			{
				printf("Enter car ID: ");
				scanf("%d", &id);

				if (getCarsale(seller, &carsale, id, error))
				{
					deleteCarsale(seller, carsale, id, error);
					printf("Deleted successfully\n");
				}
				else
				{
					printf("Error: %s\n", error);
				}
			}
			else
			{
				printf("Error: %s\n", error);
			}
			break;

		case 9:
			info();
			break;
		default:
			printf("Invalid input, please try again\n");
		}

		printf("---------\n");
	}

	return 0;
}