#!/usr/bin/env python3

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

# Warning push/pop blocks in the main header wrap its includes: they are hoisted around the whole amalgamation
WARNING_PUSH_BLOCK_MATCHER = re.compile(r'^#if\b[^\n]*\n#pragma warning\(push\)\n(?:#pragma warning\([^\n]*\n)*#endif[ \t]*$', re.MULTILINE)
WARNING_POP_BLOCK_MATCHER = re.compile(r'^#if\b[^\n]*\n#pragma warning\(pop\)\n#endif[ \t]*$', re.MULTILINE)
MAIN_HEADER_NAME = "LuaBridge.h"
CONFIG_HEADER_NAME = "Config.h"

GUARDED_INCLUDES = [
	{ "header": "version" },
	{ "header": "coroutine", "condition": "(__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))" },
	{ "header": "ranges", "condition": "(__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))" },
	{ "header": "span", "condition": "(__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))" },
	{ "header": "flat_map", "condition": "(__cplusplus >= 202302L || (defined(_MSC_VER) && _HAS_CXX23))" },
	{ "header": "flat_set", "condition": "(__cplusplus >= 202302L || (defined(_MSC_VER) && _HAS_CXX23))" },
	{ "header": "expected", "condition": "(__cplusplus >= 202302L || (defined(_MSC_VER) && _HAS_CXX23))" },
	{ "header": "move_only_function", "condition": "(__cplusplus >= 202302L || (defined(_MSC_VER) && _HAS_CXX23))" },
	{ "header": "Luau/Common.h", "inplace": True },
]

def GetGuardedInclude(header):
	for guarded in GUARDED_INCLUDES:
		if guarded["header"] == header:
			return guarded
	return None

def IsInPlaceInclude(header):
	guarded = GetGuardedInclude(header)
	return guarded is not None and guarded.get("inplace", False)

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
		self.warningPushBlock = None
		self.warningPopBlock = None

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
				elif not IsInPlaceInclude(includeResult[0]):
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

			text = RemoveComments(source.read()).replace("\r\n", "\n").replace("\r", "\n")

			if self.IsMainHeader(path):
				text = self.StripWarningBlocks(text)

			lines = text.split("\n")

			for line in lines:
				result = INCLUDE_FILE_MATCHER.findall(line)
				if result and not IsInPlaceInclude(result[0]):
					continue

				result = PRAGMA_ONCE_MATCHER.findall(line)
				if result:
					continue

				stripped_line = line.strip()
				if stripped_line or not lastLineWasEmpty:
					stream.write(f"{line}\n")

				lastLineWasEmpty = not stripped_line

			self.WriteEndFileHeader(path, stream)

			if os.path.basename(path) == CONFIG_HEADER_NAME and self.warningPushBlock:
				stream.write(f"\n{self.warningPushBlock}\n")

	def IsMainHeader(self, path):
		return os.path.normpath(path) == os.path.normpath(os.path.join(self.baseDir, MAIN_HEADER_NAME))

	def ExtractWarningBlocks(self):
		mainHeaderPath = os.path.join(self.baseDir, MAIN_HEADER_NAME)
		if not os.path.exists(mainHeaderPath):
			return

		with open(mainHeaderPath, 'r') as source:
			text = RemoveComments(source.read()).replace("\r\n", "\n").replace("\r", "\n")

		pushMatch = WARNING_PUSH_BLOCK_MATCHER.search(text)
		popMatch = WARNING_POP_BLOCK_MATCHER.search(text)
		if pushMatch is not None and popMatch is not None:
			self.warningPushBlock = pushMatch.group(0)
			self.warningPopBlock = popMatch.group(0)

	def StripWarningBlocks(self, text):
		text = WARNING_PUSH_BLOCK_MATCHER.sub("", text, count=1)
		return WARNING_POP_BLOCK_MATCHER.sub("", text, count=1)

	def WriteAlgamationFiles(self):
		headerPath = os.path.join(self.outputDir, self.outputName + self.headerFileExt)

		self.LogMessage(f"Creating source Amalgamation: {headerPath}")

		with open (headerPath , 'w') as headerAmalgamation:
			current_year = datetime.date.today().year
			headerAmalgamation.write(f"// https://github.com/kunitoki/LuaBridge3\n")
			headerAmalgamation.write(f"// Copyright {current_year}, kunitoki\n")
			headerAmalgamation.write(f"// SPDX-License-Identifier: MIT\n\n")
			headerAmalgamation.write(f"// clang-format off\n\n")
			headerAmalgamation.write(f"#pragma once\n\n")

			systemHeaders = list(self.systemHeaders)
			for header in sorted(systemHeaders):
				if GetGuardedInclude(header) is None:
					headerAmalgamation.write(f"#include <{header}>\n")
			headerAmalgamation.write("\n")

			for header in reversed(sorted(systemHeaders)):
				guard = GetGuardedInclude(header)
				if guard is not None:
					headerAmalgamation.write(f"#if defined(__has_include) && __has_include(<{header}>)")
					if "condition" in guard and guard["condition"] is not None:
						headerAmalgamation.write(f" && {guard['condition']}\n")
					else:
						headerAmalgamation.write("\n")
					headerAmalgamation.write(f"#include <{header}>\n")
					headerAmalgamation.write(f"#endif\n\n")
			headerAmalgamation.write("\n")

			self.ExtractWarningBlocks()
			self.AmalgamateQueue(self.headerQueue, headerAmalgamation)

			if self.warningPopBlock:
				headerAmalgamation.write(f"\n{self.warningPopBlock}\n\n")

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
