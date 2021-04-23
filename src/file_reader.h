#ifndef FILE_READER_H
#define FILE_READER_H

/*
 * Code for reading text files.  Nothing about processing that data, which happens in data_reader or elsewhere.
 */


char *readFileToString(const char *filename);
unsigned char *readBinaryFileToCharStar(const char *filename, unsigned long *length);
int fileExists(const char *filename);

#endif
