#! /usr/bin/python3
# Readme: First argument is the folder with all the text files
# Second argument is the folder where all the outputs are required
# Automatically deals with html tags
# Creates a file with a single line with all the words

import re
import sys
import os

inputFolder = sys.argv[1]
outputFolder = sys.argv[2]

def cleanAndWriteFile(inputFile, outputFile):
	print('Starting..\nInput Filename : ' + inputFile + "\nOutput Filename : " + outputFile)

	cleanLines = ""
        isTxt = inputFile.endswith('.txt')
	TAG_RE = re.compile(r'<[^>]+>')

	with open(inputFile) as fileHandle:
		for line in fileHandle:
			if(not isTxt):
				line = TAG_RE.sub('', line)

		        lineWithoutApost = re.sub('\'', '', line)
			alphaNumeric = re.sub('[^0-9a-zA-Z]+', ' ', lineWithoutApost)

			# Remove tabs, new lines extra white spaces
			clean = re.sub( '\s+', ' ', alphaNumeric)
			cleanLines = cleanLines + '\n' + clean

	finalString = re.sub(' +',' ', cleanLines)
	outFileHandle = open(outputFile, 'w')
	outFileHandle.write(finalString)
	outFileHandle.close()



i = 1;
for fileName in os.listdir(inputFolder):
	outputFileName = outputFolder + str(i) + ".txt"
        inputFileName = inputFolder + fileName 
	cleanAndWriteFile(inputFileName, outputFileName)
        i = i + 1
