// Johnny Chen
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Open the 7 encoded files
void open7Files(char *fileName, FILE *fileList[])
{
    char fileNames[30];
    for (int i = 0; i < 7; i++)
    {
        sprintf(fileNames, "%s.part%d", fileName, i);
        fileList[i] = fopen(fileNames, "rb");
    }
}

// Function to extract a single bit from the buffer of a file
int getBitFromBuffer(unsigned char *buffer, int *bitCount, FILE *file, int debug)
{
    if (*bitCount == 0)
    {
        unsigned char byte = fgetc(file);
        if (byte == EOF)
        {
            return EOF; // Return EOF if no more bytes to read
        }
        *buffer = byte;
        *bitCount = 8;
    }

    int bit = (*buffer >> 7) & 0x01;
    *buffer <<= 1;
    (*bitCount)--;

    // Debug for printing out the decoded byte and its character representation
    if (debug)
    {
        printf("Extracted bit: %d, remaining bits: %d\n", bit, *bitCount);
    }

    return bit;
}

// // Function to decode encoded data from 7 files
unsigned char decodeHamming(FILE *fileList[], unsigned char buffers[], int bitCounts[], int debug)
{

    unsigned char d1, d2, d3, d4;
    unsigned char p1, p2, p4;
    // Array to store the data bits
    unsigned char hammingBits[7];

    // Read one bit from each of the 7 files
    for (int i = 0; i < 7; i++)
    {
        hammingBits[i] = getBitFromBuffer(&buffers[i], &bitCounts[i], fileList[i], debug);
        if (hammingBits[i] == EOF)
        {
            return 0;
        }
    }

    // extract the original data bits (d1, d2, d3, d4) from Hamming code
    d1 = hammingBits[2];
    d2 = hammingBits[4];
    d3 = hammingBits[5];
    d4 = hammingBits[6];

    // Get the even parity bits from them
    p1 = d1 ^ d2 ^ d4;
    p2 = d1 ^ d3 ^ d4;
    p4 = d2 ^ d3 ^ d4;

    // Error checker
    unsigned char errorDetect = (hammingBits[0] != p1) | ((hammingBits[1] != p2) << 1) | ((hammingBits[3] != p4) << 2);

    if (errorDetect != 0)
    {
        hammingBits[errorDetect - 1] ^= 1; // Flip the bit at position [errorDectect - 1]
    }

    // Reconstruct the original data bits after correction
    d1 = hammingBits[2];
    d2 = hammingBits[4];
    d3 = hammingBits[5];
    d4 = hammingBits[6];

    // Debug to print out the Hamming bits
    if (debug)
    {
        printf("p1:\tp2:\td1:\tp4:\td2:\td3:\td4:\n");
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               hammingBits[0], hammingBits[1], hammingBits[2],
               hammingBits[3], hammingBits[4], hammingBits[5], hammingBits[6]);
    }

    // Combine the data bits back into a nibble
    return (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
}

// Function to decode RAID files into an output file, byte by byte
void decodeToFile(FILE *output, FILE *fileList[], int numberOfBytes, int debug)
{
    unsigned char buffer[7] = {0}; // Buffer for each file
    int bitCount[7] = {0};         // Bit counters for each file
    int byteCount = 0;             // Track how many bytes have been decoded

    while (byteCount < numberOfBytes)
    {
        unsigned char highNibble = decodeHamming(fileList, buffer, bitCount, debug);
        unsigned char lowNibble = decodeHamming(fileList, buffer, bitCount, debug);
        int byte = (highNibble << 4) | lowNibble;
        fputc(byte, output);

        // Debug for printing out the decoded byte and its character representation
        if (debug)
        {
            printf("Decoded byte: %d (%c)\n\n", byte, byte);
        }

        byteCount++; // Increment byte count after writing a full byte
    }
}

// Close the 7 encoded files
void close7Files(FILE *fileList[])
{
    for (int i = 0; i < 7; i++)
    {
        fclose(fileList[i]);
    }
}

int main(int argc, char *argv[])
{
    char *inFile = NULL;
    FILE *fileList[7];
    FILE *outfilePtr = NULL;
    int c = 0;
    int numberOfBytes = 0;
    int debug = 0;

    while ((c = getopt(argc, argv, "df:s:")) != -1)
    {
        switch (c)
        {
        case 'd':
            debug = 1;
            break;
        case 'f':
            inFile = (char *)malloc(sizeof(char) * (strlen(optarg) + 1));
            strcpy(inFile, optarg);
            break;
        case 's':
            numberOfBytes = atoi(optarg);
            break;
        default:
            break;
        }
    }

    // Open the output file
    char outputFile[30];
    sprintf(outputFile, "%s.2", inFile);
    outfilePtr = fopen(outputFile, "wb");

    // Open the 7 RAID2 files
    open7Files(inFile, fileList);

    // Decode from RAID files and write to output file
    decodeToFile(outfilePtr, fileList, numberOfBytes, debug);

    // Close all files
    close7Files(fileList);

    // Close output file
    fclose(outfilePtr);
    return 0;
}
