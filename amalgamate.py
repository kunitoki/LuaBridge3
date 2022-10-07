import os
import re
import argparse
import datetime
from collections import deque

PARSE_FILE = 0
EXTERNAL_FILE = 1
ALREADY_SCANNED = 2

CPP_HEADER_FILE_EXT = set([".hpp" , ".h" , ".hxx" , ".hh" , ".inl"])

PRAGMA_ONCE_MATCHER = re.compile(r'#pragma once')
INCLUDE_FILE_MATCHER = re.compile(r'#include\s*[<\"]([\w.\\/]*)[>\"]')
LOCAL_INCLUDE_FILE_MATCHER = re.compile(r'#include\s*\"([\w.\\/]*)\"')


def IsCppHeaderFile(ext):
	return  ext in CPP_HEADER_FILE_EXT

def AdjustFileExtension(ext):
	if ext[0] != '.':
		ext = '.' + ext

def RemoveComments(text):
	def BlotOutNonNewlines(strIn):
		if strIn.startswith("/*"):
			return "\n"
		else:
			return ""

	def Replacer(match):
		s = match.group(0)
		if s.startswith('/'):
			return BlotOutNonNewlines(s)
		else:
			return s

	pattern = re.compile(
		r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
		re.DOTALL | re.MULTILINE
	)

	return re.sub(pattern, Replacer, text)

class SourceInfo:
	def __init__(self, baseDir , outputDir, outputName):
		self.includeDirs = list()
		self.headerQueue = deque()
		self.systemHeaders = set()
		self.scannedFiles = set()

		self.baseDir = baseDir
		self.outputDir = outputDir

		self.outputName = outputName
		self.headerFileExt = ".h"

		self.AddIncludeDirectory(self.baseDir)
		self.AddIncludeDirectory(os.path.join(self.baseDir, "detail"))

	def LogMessage(self, message):
		print(message)

	def AddIncludeDirectory(self, path):
		if not os.path.exists(path):
			return False

		self.LogMessage(f"Include Directory Added: {path}")

		self.includeDirs.append(path)
		return True

	def GetAbsoluteSourcePath(self, pwd, include):
		if os.path.isabs(include):
			return include

		for includeDir in self.includeDirs:
			absPath = os.path.normpath(os.path.join(includeDir, include))
			if os.path.exists(absPath):
				return absPath

		return None

	def ShouldParseFile(self , path , ext):
		if (path in self.scannedFiles): return ALREADY_SCANNED

		if not IsCppHeaderFile(ext) or not os.path.exists(path):
			return EXTERNAL_FILE

		return PARSE_FILE

	def ScanSourceFile(self, path , depth):
		dirpath, filename = os.path.split(path)
		ext = os.path.splitext(filename)[1]

		info = self.ShouldParseFile(path, ext)
		if info != PARSE_FILE:
			return info

		self.LogMessage(f"Scan file: {path}")
		self.scannedFiles.add(path)

		with open (path , "r") as src:
			lines = src.readlines()

			for line in lines:
				includeResult = INCLUDE_FILE_MATCHER.findall(line)
				if not includeResult:
					continue

				localResult = LOCAL_INCLUDE_FILE_MATCHER.findall(line)
				if localResult:
					includeFile = self.GetAbsoluteSourcePath(dirpath, localResult[0])
					if includeFile is None:
						continue

					call = self.ScanSourceFile(includeFile , depth + 1)
					if call == EXTERNAL_FILE:
						self.scannedFiles.add(includeFile)
				else:
					self.systemHeaders.add(includeResult[0])

		self.AddFileToQueue(path, ext)

		return info

	def ParseDirectories(self):
		all_files = []

		for sourceDirectory in self.includeDirs:
			for root, _, files in os.walk(sourceDirectory):
				for filename in files:
					all_files.append(os.path.join(root, filename))

		for path in sorted(all_files):
			self.ScanSourceFile(path, 0)


	def WriteBeginFileHeader(self, filename, stream):
		stream.write(f"\n// Begin File: {filename}\n")

	def WriteEndFileHeader(self, filename, stream):
		stream.write(f"\n// End File: {filename}\n")

	def AddFileToQueue(self, filename, ext):
		if IsCppHeaderFile(ext):
			self.LogMessage(f"Enqueue header file: {filename}")
			self.headerQueue.append(filename)

	def AmalgamateQueue(self, queue, stream):
		while (len(queue) > 0):
			path = queue.popleft()
			self.WriteFileToStream(path, stream)

	def WriteFileToStream(self, path, stream):
		self.LogMessage(f"Write File: {path}")

		with open (path, 'r') as source:
			self.WriteBeginFileHeader(path, stream)

			lastLineWasEmpty = False

			text = RemoveComments(source.read())
			lines = text.replace("\r", "\n").split("\n")

			for line in lines:
				result = INCLUDE_FILE_MATCHER.findall(line)
				if result:
					continue

				result = PRAGMA_ONCE_MATCHER.findall(line)
				if result:
					continue

				stripped_line = line.strip()
				if stripped_line or not lastLineWasEmpty:
					stream.write(f"{line}\n")

				lastLineWasEmpty = not stripped_line

			self.WriteEndFileHeader(path, stream)

	def WriteAlgamationFiles(self):
		headerPath = os.path.join(self.outputDir, self.outputName + self.headerFileExt)

		self.LogMessage(f"Creating source Amalgamation: {headerPath}")

		with open (headerPath , 'w') as headerAmalgamation:
			current_year = datetime.date.today().year
			headerAmalgamation.write(f"// https://github.com/kunitoki/LuaBridge3\n")
			headerAmalgamation.write(f"// Copyright {current_year}, Lucio Asnaghi\n")
			headerAmalgamation.write(f"// SPDX-License-Identifier: MIT\n\n")
			headerAmalgamation.write(f"// clang-format off\n\n")
			headerAmalgamation.write(f"#pragma once\n\n")

			for header in sorted(list(self.systemHeaders)):
				headerAmalgamation.write(f"#include <{header}>\n")
			headerAmalgamation.write("\n")

			self.AmalgamateQueue(self.headerQueue, headerAmalgamation)

			headerAmalgamation.write("// clang-format on\n\n")

		return headerPath

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Amalgamate LuaBridge.')
	parser.add_argument('--base', action='store', default="Source/LuaBridge/")
	parser.add_argument('--output', action='store', default="Distribution/LuaBridge/")
	parser.add_argument('--name', action='store', default="LuaBridge")

	args = parser.parse_args()

	sourceInfo = SourceInfo(args.base, args.output, args.name)
	sourceInfo.ParseDirectories()
	sourceInfo.WriteAlgamationFiles()
