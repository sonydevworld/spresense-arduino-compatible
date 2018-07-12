#!/usr/bin/env python3
# -*- coding:utf-8 -*-

# load_tool_variable_from_json.py:
# This tool is for loading Arduino IDE tool information
# from package_*_index.json.
# Reference format is Arduino IDE 1.8.5
# Please see more detail format in next link.
# https://github.com/arduino/arduino/wiki/arduino-ide-1.6.x-package_index.json-format-specification
# Usage: load_tool_variable_from_json.py -j <package_*_index.json> -p <package name> -b <board name> -t <tool name> -H <host name> -k <key of value>

import argparse
import json

# function     : load
# json_filename: json file name(type: str)
# return       : Json contents root
def load(json_filename):
	file = open(json_filename, "r")
	json_contents = json.load(file)
	file.close()
	return json_contents

# function: findlist_with_version
# n_list  : Json element(type: dic)
# category: Key of child node(type: str)
# key     : Key of search trigger(type: str)
# val     : Match word(type: str)
# version : version of tool
# return  : Json contents root
def findlist_with_version(n_list, category, key, val, version):
	ret = None

	# Get List from key
	rets = n_list[category]

	# Search required element
	for r in rets:
		# Check required value
		# If version=None, not using version trigger
		if r[key] == val and (version == None or r['version'] == version):
			ret = r
			break

	# If cannot pickup, do nothing and exit
	if ret == None:
		exit()
	return ret

# function: findlist
# n_list  : Json element(type: dic)
# category: Key of child node(type: str)
# key     : Key of search trigger(type: str)
# val     : Match word(type: str)
# return  : Json contents root
def findlist(n_list, category, key, val):
	# Search without using version
	return findlist_with_version(n_list, category, key, val, None)

if __name__ == '__main__':
	# Option
	parser = argparse.ArgumentParser(description=None)
	parser.add_argument('-j', '--json', metavar='package_index.json', type=str, help='Board Manager Json file name')
	parser.add_argument('-p', '--package', metavar='spresense', type=str, help='Arduino package name')
	parser.add_argument('-b', '--board', metavar='spresense', type=str, help='Arduino board name')
	parser.add_argument('-t', '--tool', metavar='spresense-sdk', type=str, help='Arduino tool name')
	parser.add_argument('-H', '--host', metavar='i686-mingw32', type=str, help='Arduino IDE host name')
	parser.add_argument('-k', '--key', metavar='version', type=str, help='Key of value')
	args = parser.parse_args()

	# Input error handling
	if args.json == None or args.package == None or args.board == None or args.tool == None or args.key == None:
		# Nothing to do
		exit()

	# Load json contents
	json_root = load(args.json)

	# Search tools dependency
	# Pickup packages = args.package
	package = findlist(json_root, 'packages', 'name', args.package)

	# Pickup packages(args.package) -> platforms = args.board
	platform = findlist(package, 'platforms', 'architecture', args.board)

	# Pickup packages(args.package) -> platforms(args.board) -> toolsDependencies = args.tool
	tool = findlist(platform, 'toolsDependencies', 'name', args.tool)

	# Get tool version target required
	tool_version = tool['version']

	# print tool version if requested
	if args.key == 'version':
		print (tool_version)
		exit() 

	# Pickup packages(args.package) -> tools = args.tool, version = tool_version
	tool = findlist_with_version(package, 'tools', 'name', args.tool, tool_version)

	# Transfer host easy name to host name
	if args.host == 'Windows':
		arduino_host = 'i686-mingw32'
	elif args.host == 'Linux32':
		arduino_host = 'i686-pc-linux-gnu'
	elif args.host == 'Linux64':
		arduino_host = 'x86_64-pc-linux-gnu'
	elif args.host == 'Mac':
		arduino_host = 'i386-apple-darwin11'
	else:
		# If invalid host name, do nothing and exit
		exit()

	# Pickup packages(args.package) -> tools(args.tool,tool_version) -> host = arduino_host
	system = findlist(tool, 'systems', 'host', arduino_host)

	# Print value
	print (system[args.key])
