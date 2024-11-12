// Johnny Chen
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to create and open 7 files for RAID2
void make7Files(char *inputFileName, FILE *fileList[])
{
    char fileName[30];
    for (int i = 0; i < 7; i++)
    {
        sprintf(fileName, "%s.part%d", inputFileName, i);
        fileList[i] = fopen(fileName, "wb");
    }
}

// Function to write the bits to the file from buffer
void writeBitToFile(int bit, unsigned char *buffer, int *bitCount, FILE *file)
{
    *buffer = (*buffer << 1) | (bit & 0x01); // Shift left and set bit
    ++*bitCount;                             // keep track of the number of left shifting

    // write as a byte after collecting 8 bits
    if (*bitCount == 8)
    {
        fputc(*buffer, file);
        *bitCount = 0; // Reset
        *buffer = 0;   // Reset
    }
}

// Store each Hamming-encoded bit to its respective file as bytes
void storeHammingBits(unsigned char hammingBits[], FILE *fileList[],
                      unsigned char buffer[], int bitCount[])
{
    // writeBitToBuffer(p1, &buffers[0], &bitCounts[0], fileList[0]);  // P1
    // writeBitToBuffer(p2, &buffers[1], &bitCounts[1], fileList[1]);  // P2
    // writeBitToBuffer(d1, &buffers[2], &bitCounts[3], fileList[2]);  // P4
    // writeBitToBuffer(p4, &buffers[3], &bitCounts[2], fileList[3]);  // D1
    // writeBitToBuffer(d2, &buffers[4], &bitCounts[4], fileList[4]);  // D2
    // writeBitToBuffer(d3, &buffers[5], &bitCounts[5], fileList[5]);  // D3
    // writeBitToBuffer(d4, &buffers[6], &bitCounts[6], fileList[6]);  // D4

    for (int i = 0; i < 7; i++)
    {
        writeBitToFile(hammingBits[i], &buffer[i], &bitCount[i], fileList[i]);
    }
}

// Encode a nibble and store in the files
void encodeHamming(unsigned char nibble, FILE *fileList[], unsigned char buffer[],
                   int bitCounts[], int debug)
{
    unsigned char d1, d2, d3, d4;
    unsigned char p1, p2, p4;

    // Split nibble into data bits
    d1 = (nibble >> 3) & 0x01;
    d2 = (nibble >> 2) & 0x01;
    d3 = (nibble >> 1) & 0x01;
    d4 = (nibble >> 0) & 0x01;

    // even parity bits
    p1 = d1 ^ d2 ^ d4; // alternative: p1 = (d1 + d2 + d4) % 2;
    p2 = d1 ^ d3 ^ d4;
    p4 = d2 ^ d3 ^ d4;

    // Array for formatting, similar to the chart
    unsigned char hammingBits[] = {p1, p2, d1, p4, d2, d3, d4};

    // Store the Hamming bits to the files
    storeHammingBits(hammingBits, fileList, buffer, bitCounts);

    // Checkif the data and parity bits are correct
    if (debug)
    {
        printf("Nibble: %d | p1: %d, p2: %d, d1: %d, p4: %d, d2: %d, d3: %d, d4: %d\n", nibble, p1, p2, d1, p4, d2, d3, d4);
    }
}

// Process the file, read each byte, and split into two nibbles for encoding
void fileToNibble(FILE *input, FILE *fileList[], int debug)
{
    unsigned char highNibble;
    unsigned char lowNibble;
    int character;

    unsigned char buffer[7] = {0}; // Initialize buffers for each file
    int bitCounts[7] = {0};        // Bit counter for each file

    if (debug)
    {
        printf("Debug mode enabled.\n");
    }

    // Read each character from the input file and split into two nibbles
    while ((character = fgetc(input)) != EOF)
    {
        highNibble = (character >> 4) & 0x0F; // Higher 4 bits
        lowNibble = character & 0x0F;         // Lower 4 bits

        // Check if High Nibble and Low Nibble are processed correctly
        if (debug)
        {
            printf("High Nibble: %d, Low Nibble: %d\n", highNibble, lowNibble);
        }

        // Encode and store both nibbles
        encodeHamming(highNibble, fileList, buffer, bitCounts, debug);
        encodeHamming(lowNibble, fileList, buffer, bitCounts, debug);
    }
}

// Close the 7 files after writing
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
    FILE *filePtr = NULL;
    FILE *fileList[7];
    int c = 0;
    int debug = 0;

    while ((c = getopt(argc, argv, "df:")) != -1)
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
        default:
            break;
        }
    }
    // Default input file if none provided
    if (inFile == NULL)
    {
        inFile = "completeShakespeare.txt";
    }

    // Open the input file
    filePtr = fopen(inFile, "r");

    // Create the 7 RAID2 files
    make7Files(inFile, fileList);

    // Process the input file, encode the data, and put into the 7 files
    fileToNibble(filePtr, fileList, debug);

    // Close the 7 RAID2 files
    close7Files(fileList);

    // Close the input file
    fclose(filePtr);

    return 0;
}