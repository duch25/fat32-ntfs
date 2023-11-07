#include "NTFS.h";

int main()
{
	// choose volume to read information
	int choice = -1;
	int minIdxChoice = 1, maxIdxChoice = 1;
	map<int, wstring> volumes = {{1, L"F"}};

	do
	{
		cout << "========== AVAILABLE VOLUMES ==========" << endl;
		cout << "1/ F:" << endl;
		cout << "=======================================" << endl << endl;

		cout << "Which volume to use (please type a number): ";
		cin >> choice;

		if (choice < minIdxChoice || choice > maxIdxChoice)
		{
			system("cls");
			cout << "Invalid choice! Please try again!" << endl;
		}
	} while (choice < minIdxChoice || choice > maxIdxChoice);

	wstring diskName = L"\\\\.\\" + volumes[choice] + L":";
	LPCWSTR drive = diskName.c_str();

	// read volume information
	BYTE *sector = new BYTE[512];

	// read Partion Boot Sector informtation
	if (Utility::readSector(drive, 0, sector))
	{
		Utility::printSectorTable(sector);

		// read BIOS Parameter Block information
		NTFS::readBPB(sector, drive);
	}
	else
		cout << "Read unsuccessfully!" << endl;

	delete[] sector;
	return 0;
}