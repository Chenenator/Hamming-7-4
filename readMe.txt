Johnny Chen

raid.c

Commands to run it: 
	./raid -f test.txt 
	./raid -f completeShakespeare.txt
for debug ./raid -d -f "the test files"

Essential functions that does the processing:

fileToNibble: This function read each character from the input file, then it splits each character's hex value into 2 nibbles using bitwise operators: high and low nibble. The High nibble will be the left half and the low nibble be the right half. Then it calls the endcodeHamming function. The buffer[7] array  accumulate bits for each 7 file. The bitCount[7] keeps track of how many bits have been collected for each buffer.

Example: Assume the character read is A. A is 01000001 in binary so the high nibble will be 4 (0100) and low nibble 1 (001). These nibbles are passed to encodingHamming, which processes them along with their buffer and bitCount. I use debug here to see if the nibble are calculated correctly (High Nibble: 4, Low Nibble: 1)

encodeHamming: This function process the nibbles into data bits (d1, d2, d3, d4) and even parity bits (p1, p2, p4). Then put them into an array called hammingBits[] so I can format it like this {p1, p2, d1, p4, d2, d3, d4}-like the chart on the instruction. Still hasn't use their buffer and bitCount yet; the next function will use them.

Example: High nibble from fileToNibble is processed first so the hammingBits[] = {1, 0, 0, 1, 1, 0, 0}. Then it passes to storeHammingBits. Next is the low nibble so the hammingBits[] = {1, 1, 0, 1, 0, 0, 1} then it passes to storeHammingBits. I used debug here to find if my data bits and parity bits are correct. 

storeHammingBits: The function iterates over the hammingBits[], calling writeBitToFile for each bit iteration. It passes the respective buffer and bit count for each file from buffer and bitCount array as it calls writeBitToFile. 

Example: For High nibble, Its hammingBits[] = {1, 0, 0, 1, 1, 0, 0}.
	After it goes through the array...
buffer[0] = 00000001 (P1)	bitCounts[0] = 1
buffer[1] = 00000000 (P2)	bitCounts[1] = 1
buffer[2] = 00000000 (D1)	bitCounts[2] = 1
buffer[3] = 00000001 (P4)	bitCounts[3] = 1
buffer[4] = 00000001 (D2)	bitCounts[4] = 1
buffer[5] = 00000000 (D3)	bitCounts[5] = 1
buffer[6] = 00000000 (D4)	bitCounts[6] = 1

writeBitToFile: This function is where it writes the bits into the 7 files using the buffer. 

Example: When storeHammingBits calls this function for the first iteration. It gets 00000001 from buffer[0] and 1 from bitCounts[0].
Now it will shift 00000001 to the left by 1 until bitCount = 8 (byte). So the value should be 10000000 and then store it at file[0] which is the p1 file using fputc. After writing, both bitCount and buffer are reset to prepare for the next byte of bits which is at buffer[1] and bitCounts[1].

"Special features:" For the writeBitToFile function, my original thought was to get the corresponding bit in each buffer index and write it out. 
For instance, I was gonna put 00000001 into an another array and loop through it until index = 8 and extract that element out then use fputc to the file. I tried to implement my logic and it turned out to be a mess. I looked online to see if there is any way to write a bit to a file and according to my research, C cannot write individual bit. However you can do manipulate a byte to write out its bit using bitwise operator. In addition, I also found out for the parity bit calculation, p1 = d1 ^ d2 ^ d4 is equal to p1 = (d1 + d2 + d4) % 2. I feel like the bitwise XOR will be quicker since there is "less math."


diar.c

Commands to run it:
	./diar -f test.txt -s 8
	./diar -f completeShakespeare.txt -s 5694072
	./diar -d -f "the test file" -s size

Essential functions that does the processing:

decodeToFile: This is the function that main will call after opening up the 7 files. It decodes the data in each iteration by reading two nibbles (High and Low), and calls decodeHamming function to get their 4 bits. Then it combines these nibble to form a byte and write it out to the output file using fputc until it reaches the size of the original file in bytes. The buffer and bitCount arrays work similarly to that of raid.c. 

Example: Suppose the RAID part files encode "ABCD"
		This function will write 
		A into the output file for 1st iteration
		B for second and so on.

getBitFromBuffer: This function extracts one bit at a time from the buffer of the RAID files. If the buffer is empty, the function reads the next byte from the file and resets the bitCount to 8. Then the most significant bit is extracted by shifting the byte to the right by 7 and masking the result with 0x01. After that the extracted bit is returned, and pass on to decodeHamming so it too can read the bit. The buffer is left-shifted for the next bit extraction and the remaining bits will be *bitCount - 1 and so on.     

Example: Assume a file contains 10101010 11001100. This function will do this:
Extracted bit: 1, remaining bits: 7 (return 1)
Extracted bit: 0, remaining bits: 6 (return 0)
Extracted bit: 1, remaining bits: 5     .
Extracted bit: 0, remaining bits: 4     .
Extracted bit: 1, remaining bits: 3     .
Extracted bit: 0, remaining bits: 2
Extracted bit: 1, remaining bits: 1
Extracted bit: 0, remaining bits: 0 

Extracted bit: 1, remaining bits: 7 
Extracted bit: 1, remaining bits: 6
Extracted bit: 0, remaining bits: 5
Extracted bit: 0, remaining bits: 4
Extracted bit: 1, remaining bits: 3
Extracted bit: 1, remaining bits: 2
Extracted bit: 0, remaining bits: 1
Extracted bit: 0, remaining bits: 0


decodeHamming: This function will calculate the hamming bits for the high and low nibble from the 7 files and gives it for decodetoFile to combine it. First it reads the bit from the getBitFromBuffer and store it into a hammingBits[7] array. Next it assign the data bits to variables according to this format: {p1, p2, d1, p4, d2, d3, d4}. So d1 = hammingBits[2], d2 = hammingBits[4] and so on. For parity bits, its XOR operation on those data bits. To correct errors occur in data bits, it is computed by comparing the expected parity bits with the actual values read from the hammingBits. The result is stored in the errorDetect variable. If there is a issue, errorDetect will indicate which bit needs to be flipped. If errorDetect is not zero, the corresponding bit in the hammingBits array is flipped to correct the error. Then it reconstruct the original data bits after correction. After that, it combines the data bits to a nibble using bitwise operator. 

Example: Suppose hammingBits[] = {0, 1, 0, 0, 1, 0, 1} then after calling this function:
	d1 = 0
	d2 = 1
	d3 = 0
	d4 = 1
	
	p1 = 0
	p2 = 1
	p4 = 0
If there are no errors it should return 
(d1 << 3) | (d2 << 2) | (d3 << 1) | d4
= (0 << 3) | (1 << 2) | (0 << 1) | 1 
= 0000 | 0100 | 0000 | 0001 
= 0101 (5)

"special feature:" 
The decoding part is where I was stuck on the most. I went online and read on wiki and discussion forms online about methods to decode hamming(7,4). Many people said you can do the decoding and error correction part using matrix multiplication. Without implementing the error correction first, I tried just to make a matrix that consist of data bits. It seemed I can get the data by reading the chart across. So I made a 2d array and store the data bits read from the 7 files. I was able to get the output "ABCD" but not the remaning "abc" from the text.txt. This was the road block, so I decided to scrap the idea. I asked my classmate Dennis Wong to see what his approach was and he said read each byte from the files then bit shift to get the correct data bit. 

It took me a while to realize something and I was kind of surprised: diar.c is just backwards raid.c so I just need to code backwards.




