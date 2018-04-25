#! /usr/bin/python3
# Readme: First argument is the folder with all the text files
# Second argument is the folder where all the outputs are required
# Automatically deals with html tags
# Creates a file with a single line with all the words

import re
import sys
import os

inputFileName = sys.argv[1]
outputFileName = sys.argv[2]

def cleanAndWriteFile(inputFile, outputFile):
	print('Starting..\nInput Filename : ' + inputFile + "\nOutput Filename : " + outputFile)

	cleaned = ""
	buf = ''
	i = 0

	with open(inputFile) as fileHandle:
		for line in fileHandle:
			removedTime = re.sub('Running for (\w) data size and rT (\w)\n', ' ',line)
			#removedNodes = re.sub(r'Running for (.*) nodes (.*) reader (\w+) mapper threads and (\w+) data size\n', r' ', line)
			#removedTime = re.sub(r'Time ', '', removedNodes)
			removedNewLine = re.sub('\n', ' ', removedTime)
		        buf = buf + removedNewLine

			i = i + 1
			#v = i % 22
			v = i % 16			
			if(v == 0):
				cleaned = cleaned + buf + '\n'
				buf = ''

	finalString = cleaned + buf
	outFileHandle = open(outputFile, 'w')
	outFileHandle.write(finalString)
	outFileHandle.close()





cleanAndWriteFile(inputFileName, outputFileName)

