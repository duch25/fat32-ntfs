#include "NTFS.h";

vector<int> fileID;
vector<int> parentFileID;
vector<string> fileNames;
bool isTxt = false;

#pragma region Utility
// read sector (partition boot sector/MFT)
int Utility::readSector(LPCWSTR drive, unsigned int readPoint, BYTE*& sector, bool MFT) {
	int retCode = 0;
	DWORD bytesRead;
	HANDLE device = NULL;

	device = CreateFile(
		drive,				// drive to open 
		GENERIC_READ,		// access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,		// share mode
		NULL,				// security descriptor
		OPEN_EXISTING,		// how to create
		0,					// file attributes
		NULL				// handle to template
	);

	// open error
	if (device == INVALID_HANDLE_VALUE) {
		cout << "CreateFile: " << GetLastError() << endl << endl;
		return 0;
	}

	// set a point to read

	LARGE_INTEGER li;
	if (MFT)
		li.QuadPart = readPoint * 512;
	else
		li.QuadPart = readPoint;
	SetFilePointerEx(device, li, NULL, FILE_BEGIN);

	// read unsuccess
	if (!ReadFile(device, sector, 512, &bytesRead, NULL)) {
		cout << "ReadFile: " << GetLastError() << endl << endl;
		return 0;
	}

	CloseHandle(device);

	// read success
	if (!MFT)
		cout << endl << "Read successfully!" << endl << endl;

	return 1;
}

// print sector table
void Utility::printSectorTable(BYTE* sector) {
	int cnt = 0;
	int num = 0;
	string header = "0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F";

	cout << "==================== BOOT SECTOR TABLE ==================== " << endl << endl;
	cout << setfill(' ') << setw(9 + header.size()) << header << endl;

	cout << "0x0" << num << "0 ";
	bool flag = 0;

	for (int i = 0; i < 512; ++i) {
		cnt++;

		if (i % 8 == 0)
			cout << "  ";

		printf("%02X ", sector[i]);

		if (i == 255) {
			flag = 1;
			num = 0;
		}

		if (i == 511) break;

		if (cnt == 16) {
			int index = i;
			cout << endl;

			if (flag == 0) {
				num++;
				if (num < 10)
					cout << "0x0" << num << "0 ";
				else {
					char hex = char(num - 10 + 'A');
					cout << "0x0" << hex << "0 ";
				}
			}
			else {
				if (num < 10)
					cout << "0x1" << num << "0 ";
				else {
					char hex = char(num - 10 + 'A');
					cout << "0x1" << hex << "0 ";
				}

				num++;
			}

			cnt = 0;
		}
	}

	cout << endl;
}

// get n bytes from offset position
long long Utility::getBytes(BYTE* sector, int offset, int n) {
	long long res = 0;
	memcpy(&res, sector + offset, n);
	return res;
}

// convert binary to string (use for get: signal, name of file, content of file .txt)
string Utility::toString(BYTE* data, int offset, int number) {
	char* tmp = new char[number + 1];
	memcpy(tmp, data + offset, number);

	string s = "";

	for (int i = 0; i < number; ++i) {
		if (tmp[i] != 0x00) {
			s += tmp[i];
		}
	}

	return s;
}

// convert decimal to binary
string Utility::toBinary(int n) {
	string pattern = "";

	while (n) {
		pattern = to_string(n & 1) + pattern;
		n >>= 1;
	}

	return pattern;
}

// get name of file by id
string Utility::getFileName(int id) {
	string res = "";
	int pos = -1;

	for (int i = 0; i < fileID.size(); ++i) {
		if (fileID[i] == id) {
			pos = i; 
			break;
		}
	}

	if (pos != -1)
		res = fileNames[pos];

	return res;
}

// print folder tree
void Utility::printFolderTree(int index, int level, int pos) {
	level++;
	for (int i = 0; i < level; ++i)
		cout << "\t";

	cout << Utility::getFileName(index) << endl;

	fileID[pos] = -1;
	parentFileID[pos] = -1;

	vector<int> child;
	vector<int> childPos;

	// handle child archive
	for (int j = 0; j < fileID.size(); ++j) {
		if (parentFileID[j] == index) {
			child.push_back(fileID[j]);
			childPos.push_back(j);
		}
	}

	if (child.empty() == true) {
		return;
	}
	
	// print child tree
	for (int i = 0; i < child.size(); ++i) {
		Utility::printFolderTree(child[i], level, childPos[i]);
	}
}

// show information of folder
void Utility::showFolderInfor(unsigned int firstSector, unsigned int lastSector, LPCWSTR drive) {
	int cnt = 1;
	for (int i = 2; i < lastSector - firstSector; i += 2) {
		isTxt = false;
		int currentSector = firstSector + i;
		BYTE* currentEntry = new BYTE[512];
		Utility::readSector(drive, currentSector, currentEntry, true);

		if (Utility::toString(currentEntry, 0x00, 4) == "FILE") {
			int ID = Utility::getBytes(currentEntry, 0x02C, 4);
			if (ID > 38) { // skip the system files 
				cout << "File " << cnt++ << ":" << endl;

				string _fileName = "";
				int firstAddrOfSI = Utility::getBytes(currentEntry, 0x014, 2);
				int sizeOfSI = NTFS::readAttributeSI(currentEntry, firstAddrOfSI);

				if (sizeOfSI == -1)
					continue;

				int firstAddrOfFN = firstAddrOfSI + sizeOfSI;
				int sizeOfFN = NTFS::readAtrributeFileName(currentEntry, firstAddrOfFN, _fileName);

				int firstAddrOfDATA = firstAddrOfFN + sizeOfFN;
				// check if $OBJECT_ID then jump to $DATA
				if (Utility::getBytes(currentEntry, firstAddrOfDATA, 4) == 64) {
					firstAddrOfDATA += Utility::getBytes(currentEntry, firstAddrOfDATA + 4, 4); // jump
				}
				else {
					while (Utility::getBytes(currentEntry, firstAddrOfDATA, 4) != 128) // find signal of $DATA
						firstAddrOfDATA += 4;
				}

				NTFS::readAtrributeData(currentEntry, firstAddrOfDATA, _fileName);
				fileID.push_back(ID);
			}
		}

		delete[] currentEntry;
	}

	// print folder tree
	cout << "============== FOLDER TREE INFORMATION OF VOLUME ==========" << endl << endl;
	for (int i = 0; i < fileID.size(); ++i) {
		if (fileID[i] != -1 && parentFileID[i] != -1) {
			Utility::printFolderTree(fileID[i], -1, i);
		}
	}
}
#pragma endregion


#pragma region NTFS
// read BIOS Parameter Block
void NTFS::readBPB(BYTE* sector, LPCWSTR volume) {

	// important information of volume (first 73 bytes)
	unsigned int bytesPerSector = Utility::getBytes(sector, 0x0B, 2);
	unsigned int sectorsPerCluster = Utility::getBytes(sector, 0x0D, 1);
	unsigned int sectorsPerTrack = Utility::getBytes(sector, 0x18, 2);
	unsigned int numberOfHead = Utility::getBytes(sector, 0x1A, 2);
	unsigned int totalSectors = Utility::getBytes(sector, 0x28, 8);
	unsigned int firstSector = Utility::getBytes(sector, 0x1C, 4);
	unsigned int firstClusterOfMFT = Utility::getBytes(sector, 0x30, 8);
	unsigned int firstClusterOfMFTMirror = Utility::getBytes(sector, 0x38, 8);

	cout << endl << "============ PARTITION BOOT SECTOR INFORMATION ============ " << endl << endl;
	cout << "Size of sector:\t\t\t" << bytesPerSector << endl;
	cout << "Sectors per cluster:\t\t" << sectorsPerCluster << endl;
	cout << "Sectors per track:\t\t" << sectorsPerTrack << endl;
	cout << "Number of head:\t\t\t" << numberOfHead << endl;
	cout << "Total sectors of logic disk:\t" << totalSectors << endl;
	cout << "First sector of logic disk:\t" << firstSector << endl;
	cout << "First cluster of MFT:\t\t" << firstClusterOfMFT << endl;
	cout << "First cluster of MFTMirror:\t" << firstClusterOfMFTMirror << endl;

	// convert first cluster of MFT to sector physic indicator
	unsigned int firstPhysicSector = firstClusterOfMFT * sectorsPerCluster;

	// read $MFT Entry after found first cluster of MFT 
	NTFS::readMFTEntry(firstPhysicSector, volume);
}

// read $MFT entry
void NTFS::readMFTEntry(unsigned int firstPhysicSector, LPCWSTR volume) {

	// read MFT entry
	BYTE* MFT = new BYTE[512];
	Utility::readSector(volume, firstPhysicSector, MFT, true);

	cout << endl << "=============== LIST OF IMPORTANT ATTRIBUTES ==============" << endl << endl;

	int cnt = 1;

	// $STANDAR_INFORMATION - first attribute of MFT
	int firstAddrOfSI = Utility::getBytes(MFT, 0x014, 2);
	int sizeOfSI = Utility::getBytes(MFT, 0x048, 4);

	cout << cnt++ << ". " << "$STANDAR_INFORMATION" << endl;
	cout << "\tStart at: \t\t" << firstAddrOfSI << endl;
	cout << "\tSize of content: \t" << sizeOfSI << endl << endl;

	// $FILE_NAME
	int firstAddrOfFN = firstAddrOfSI + sizeOfSI;
	int sizeOfFN = Utility::getBytes(MFT, 0x09C, 4);

	cout << cnt++ << ". " << "$FILE_NAME" << endl;
	cout << "\tStart at: \t\t" << firstAddrOfFN << endl;
	cout << "\tSize of content: \t" << sizeOfFN << endl << endl;

	// $DATA
	int typeID = Utility::getBytes(MFT, 0x108, 4);
	int firstAddrOfDATA = 0;
	int sizeOfDATA = 0;

	if (typeID == 64) { // $OBJECT_ID => jump to next attribute
		firstAddrOfDATA = firstAddrOfFN + sizeOfFN + Utility::getBytes(MFT, 0x10C, 4);
		sizeOfDATA = Utility::getBytes(MFT, 0x134, 4);
	}
	else { // typeID = 128 => attribute $DATA
		firstAddrOfDATA = firstAddrOfFN + sizeOfFN;
		sizeOfDATA = Utility::getBytes(MFT, 0x10C, 4);
	}

	cout << cnt++ << ". " << "$DATA" << endl;
	cout << "\tStart at: \t\t" << firstAddrOfDATA << endl;
	cout << "\tSize of content: \t" << sizeOfDATA << endl;

	cout << endl << "=================== LIST OF FILE/FOLDER ===================" << endl << endl;

	// calculate last physical sector of MFT
	unsigned int lastPhysicSector = firstPhysicSector + (Utility::getBytes(MFT, firstAddrOfDATA + 24, 8)) * 8;
	Utility::showFolderInfor(firstPhysicSector, lastPhysicSector, volume);
}

// read attribute $STANDAR_INFORMATION
int NTFS::readAttributeSI(BYTE* entry, int start) {

	// get state of file/folder
	cout << "\tState: ";
	int value = Utility::getBytes(entry, start + 56, 4);

	if (value == 32) cout << "Archive";
	else if (value == 0) cout << "Directory";
	else if (value == 1) cout << "Read only";
	else if (value == 2) {
		cout << "Hidden" << endl;
		return -1;
	}
	else if (value == 4) {
		cout << "System" << endl;
		return -1;
	}
	cout << endl;
	// size of attribute $STANDAR_INFORMATION
	return Utility::getBytes(entry, start + 4, 4);
}

// read attribute $FILE_NAME
int NTFS::readAtrributeFileName(BYTE* entry, int start, string &_fileName) {
	int size = Utility::getBytes(entry, start + 4, 4);
	int parentFile = Utility::getBytes(entry, start + 24, 6);

	parentFileID.push_back(parentFile);

	// get file name
	int lengthFileName = Utility::getBytes(entry, start + 88, 1);
	string fileName = Utility::toString(entry, start + 90, lengthFileName * 2); // file name be formatted by unicode
	_fileName = fileName;
	cout << "\tName of file: " << fileName << endl;
	fileNames.push_back(fileName);

	// get file type by ext
	string ext = "";
	int i = fileName.length() - 1;
	while (i >= 0 && fileName[i] != '.') ext += fileName[i--];
	reverse(ext.begin(), ext.end());

	vector<string> exts = { "doc", "docx", "xls", "xlsx", "ppt", "pptx", "sln", "pdf" };
	for (int i = 0; i < exts.size(); ++i) {
		if (ext == exts[i]) {
			cout << "\tUse the corresponding software to open this file!" << endl;
		}
	}

	if (ext == "txt") isTxt = true;

	return size;
}

// read attribute $DATA
void NTFS::readAtrributeData(BYTE* entry, int start, string _fileName) {

	// print content of text file
	if (isTxt == true) {
		int isResident = Utility::getBytes(entry, start + 8, 1);

		// read MFT entry
		if (isResident == 0) {
			// get file size
			int fileSize = Utility::getBytes(entry, start + 16, 4);
			cout << "\tSize of file: " << fileSize << endl;

			int contentSize = Utility::getBytes(entry, start + 16, 4);
			int contentStart = Utility::getBytes(entry, start + 20, 2);
			string content = Utility::toString(entry, start + contentStart, contentSize);

			cout << "\tFile content: " << content << endl;
		}
		else { // open file and read 
			ifstream fin; 
			_fileName = "F:\\" + _fileName;
			fin.open(_fileName);
			fin.seekg(0, ios::end);
			int fileSize = fin.tellg();

			cout << "\tSize of file: " << fileSize << endl;

			// read content
			cout << "\tFile content: ";
			fin.seekg(0, ios::beg);
			string line;
			while (getline(fin, line)) {
				cout << line << endl;
			}
			fin.close();
		}
	}
	cout << endl;
}
#pragma endregion 
