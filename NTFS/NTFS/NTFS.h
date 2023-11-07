#ifndef NTFS_H
#define NTFS_H

#include "config.h"

class Utility {
public:
	// read sector (partition boot sector/MFT)
	static int readSector(LPCWSTR drive, unsigned int readPoint, BYTE*& sector, bool MFT = false);

	// get n bytes from offset position
	static long long getBytes(BYTE* sector, int offset, int n);

	// convert binary to string (use for get: signal, name of file, content of file .txt)
	static string toString(BYTE* data, int offset, int number);

	// convert decimal to binary
	static string toBinary(int n);

	// get name of file by id
	static string getFileName(int id);

	// print folder tree
	static void printFolderTree(int index, int tab, int pos);

	// preprocessing for print folder tree
	static void showFolderInfor(unsigned int lengthMFT, unsigned int MFTStart, LPCWSTR disk);

	// print sector table
	static void printSectorTable(BYTE* sector);
};

class NTFS {
public:

	// read BIOS Parameter Block
	static void readBPB(BYTE* sector, LPCWSTR disk);

	// read $MFT entry
	static void readMFTEntry(unsigned int firstPhysicSector, LPCWSTR disk);

	// read attribute $STANDAR_INFORMATION
	static int readAttributeSI(BYTE* entry, int start);

	// read attribute $FILE_NAME
	static int readAtrributeFileName(BYTE* entry, int start, string &_fileName);

	// read attribute $DATA
	static void readAtrributeData(BYTE* entry, int start, string _fileName);
};
#endif // NTFS_H


