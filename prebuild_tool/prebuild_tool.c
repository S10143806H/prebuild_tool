#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "dirent.h"	// https://codeyarns.com/tech/2014-06-06-how-to-use-dirent-h-with-visual-studio.html#gsc.tab=0

#define PRINT_DEBUG 1

#ifdef _WIN32
const char PATH_SEPERATOR = '\\';
#else
const char PATH_SEPERATOR = '/';
#endif

int isDirExists(const char* path);
void copyFile(const char* sourcePath, const char* destinationPath);
void copyDirectory(const char* sourcePath, const char* destinationPath);
void deleteDirectory(const char* path);

int main(int argc, char *argv[]) { 
	// Check if the number of input arguments is correct 
	if (argc != 5) { 
#if PRINT_DEBUG
		printf("[Error] Incorrect number of input parameters. Expected 4 parameters.\r\n"); 
#endif
		exit(1);
	} 

	// Retrieve the input parameters 
	const char* toolchain_p1_path = argv[1];
	const char* toolchain_p2_path = argv[2];
	char *toolchain_p1 = argv[3];
	char *toolchain_p2 = argv[4];

#if PRINT_DEBUG
	// Print the input parameters 
	printf("Parameter 1 toolchain p1 path: %s\n", toolchain_p1_path);
	printf("Parameter 2 toolcahin p2 path: %s\n", toolchain_p2_path);
	printf("Parameter 3 toolchain_p1: %s\n", toolchain_p1);
	printf("Parameter 4 toolchain_p2: %s\n", toolchain_p2);
#endif

	// check whether directory exists
	if (isDirExists(toolchain_p1_path) && isDirExists(toolchain_p2_path)) { 
		copyDirectory(toolchain_p2_path, toolchain_p1_path);
	}

	deleteDirectory(toolchain_p2_path);
	
	// mkdir(toolchain_p2_path, 0755);

	return 0;
}

/**
*/
int isDirExists(const char* path) {
	DIR* dir;
	struct dirent* ent;
	int count = 0;
	if ((dir = opendir(path)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_DIR) {
				count++;
#if PRINT_DEBUG
				printf("[%s] Folder:%s\n", __func__, ent->d_name);
#endif
			}
			if (ent->d_type == DT_REG) {
				count++;
#if PRINT_DEBUG
				printf("[%s] File:%s\n", __func__, ent->d_name);
#endif
			}
		}
		if (count == 2) {
			/* Empty directory that contains "." and ".." only */
#if PRINT_DEBUG
			printf("[%s][Error] Empty directory found\n", __func__);
#endif
			return 0;
		}
		closedir(dir);
		return 1;
	}
	else if (ENOENT == errno) {
#if PRINT_DEBUG
		printf("[%s][Error] Directory does not exist\n", __func__);
#endif
		return 0;
	}
	else {
		/* opendir() failed for some other reason. */
		return 0;
	}
}

void copyFile(const char* sourcePath, const char* destinationPath) {
	FILE* sourceFile = fopen(sourcePath, "rb");
	FILE* destinationFile = fopen(destinationPath, "wb");

	if (sourceFile == NULL || destinationFile == NULL) {
#if PRINT_DEBUG
		printf("[%s][Error] filed to open file\n", __func__);
#endif
		return;
	}

	char buffer[1024];
	size_t bytesRead;

	while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
		fwrite(buffer, 1, bytesRead, destinationFile);
	}

	fclose(sourceFile);
	fclose(destinationFile);
#if PRINT_DEBUG
	printf("[%s][INFO] file copy done\n", __func__);
#endif
}

void copyDirectory(const char* sourcePath, const char* destinationPath) {
	DIR* sourceDir = opendir(sourcePath);

	if (sourceDir == NULL) {
#if PRINT_DEBUG
		printf("[%s][Error] filed to open file\n", __func__);
#endif
		return;
	}
	struct dirent* entry;

	while ((entry = readdir(sourceDir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		char sourceFilePath[1024];
		char destinationFilePath[1024];
		snprintf(sourceFilePath, sizeof(sourceFilePath), "%s/%s", sourcePath, entry->d_name);
		snprintf(destinationFilePath, sizeof(destinationFilePath), "%s/%s", destinationPath, entry->d_name);

		if (entry->d_type == DT_DIR) {
			mkdir(destinationFilePath, 0755);
			copyDirectory(sourceFilePath, destinationFilePath);
		}
		else {
			copyFile(sourceFilePath, destinationFilePath);
		}
	}

	closedir(sourceDir);
}

void deleteDirectory(const char* path) {
	DIR* dir = opendir(path);
	struct dirent* entry;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		char filePath[1024];
		snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
		if (entry->d_type == DT_DIR) {
			deleteDirectory(filePath);
#if PRINT_DEBUG
			printf("[%s][INFO] Directory delete!\r\n", __func__);
#endif
		}
		else {
			remove(filePath);
#if PRINT_DEBUG
			printf("[%s][INFO] File delete!\r\n", __func__);
#endif
		}
	}
	closedir(dir);
	rmdir(path);
#ifdef _WIN32 // TODO: rewrite using PATH_SEPERATOR
	strcat(path, "\\..");
#else
	strcat(path, "/..");
#endif
	rmdir(path);
}
