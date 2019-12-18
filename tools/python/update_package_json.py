#!/usr/bin/env python3
# -*- coding:utf-8 -*-

# Json file structure
#  {
#    "packages": [										List of board packages
#      "name": "<package name>",						Name of package (Arduino15/packages/<package name>)
#      "maintainer": "<Group or Vendor>",				Maintainer group of package
#      "websiteURL": "<URL>",							Maintainer's web URL
#      "help": {
#        "online": "<HELP URL>"							This packages support URL
#      },
#      "platforms": [									List of board platforms
#        {
#          "category": "Contributed",					Category of platform. Arduino or Contributed
#          "name": "<Platform name>",					Name of platform(Core library)
#          "url": "<Core library archive URL>",			URL of core library archive
#          "version": "1.0.0",							Version of platform
#          "architecture": "<Archtecture>",				Archtecture of platform (Arduino15/packages/*/hardware/<Archtecture>)
#          "archiveFileName": "<Core library file>",	Archive file name of core library
#          "size": "<Size>"								Size of archive file
#          "help": {
#            "online": "<Online HELP>"					Online help URL of this platform
#          },
#          "boards": [									Boards of using this platform
#            {
#              "name": "<Board name>"					Name of board
#            }
#          ],
#          "toolsDependencies": [						Tool dependencies of this platform
#            {
#              "packager": "<package name>",			Name of packager.Packager need to same as Package name
#              "version": "1.0.0",						Required version of this tool
#              "name": "spresense-sdk"					Name of this required tool
#            }
#          ]
#        }
#      ]
#      "tools": [										List of tools for this package
#        {
#          "version": "1.0.0",							Version of this tool
#          "name": "spresense-sdk",						Name of this tool
#          "systems": [									List of supported systems(Host OS)
#            {
#              "url": "<Tool archive URL>",				URL of this tool archive
#              "host": "i686-mingw32",					Host name of supported system
#              "archiveFileName": "<file name>",		Archive file name of this tool
#              "size": "7093913"						Size of this tool
#            }
#          ]
#        }
#      ]
#    ]
#  }

import argparse
import copy
import hashlib
import json
import os


# Generate tuple by x.y.z version number for sort
def getVersionTuple(version):
	ret = ()
	splt = version['version'].split(".")

	for v in splt:
		ret = ret.__add__((int(v), ))
	return ret

# Load json variable from json file
def load_json(filename):
	file = open(filename)
	pkg_json = json.load(file)
	file.close()

	return pkg_json

# Output json variable into file
def output_json(pkg_json, filename):
	if filename != None:
		file = open(filename, "w")
		json.dump(pkg_json, file, indent=2, sort_keys=True)
		file.close()
	else:
		print(json.dumps(pkg_json, indent=2, sort_keys=True))

# List up archive file into archive path
def get_update_archives(path):
	return os.listdir(path)

# Get archive file size
def get_archive_size(filename):
	return os.path.getsize(filename)

# Get archive file Sha256 sum
def get_archive_sha256_sum(filename):
	file = open(filename, "rb")
	read = file.read()
	file.close()

	return "SHA-256:%s" % hashlib.sha256(read).hexdigest()

# Get first package from package_inedx.json
def get_first_package(pkg_json):
	return pkg_json['packages'][0];

# Get platform definitions from package
def get_platforms(package):
	return package['platforms']

# Get base platform from package_index.json's platform
def get_base_platform(platforms, base_version):
	for platform in platforms:
		if platform['version'] == base_version:
			return platform
	return platforms[-1]

# Get tool definitions from package
def get_tools(package):
	return package['tools']

# Get tool dependencies from platform
def get_tool_dependencies(platform):
	ret = {}
	tools = platform['toolsDependencies']
	for tool in tools:
		ret[tool['name']] = tool
	return ret

# Get upload URL base from platform URL.
# platform URL: http://aaa.com/bbb/ccc/v1.2.3/spresense.tar.gz -> http://aaa.com/bbb/ccc
def get_url_base(platform, version):
	platform_url = platform['url']
	base = os.path.dirname(os.path.dirname(platform_url))
	return "%s/v%s" % (base, version)

# Main process
if __name__ == "__main__":

	# Option
	parser = argparse.ArgumentParser(description=None)
	parser.add_argument('-a', '--archive', metavar='', default=None, type=str, help='archive file path')
	parser.add_argument('-u', '--url', metavar='http://localhost/arduino', default='', type=str, help='Download root URL')
	parser.add_argument('-i', '--input_json', metavar='package_index.json', default='package_index.json', type=str, help='Input package_*_index.json path')
	parser.add_argument('-o', '--output_json', metavar='package_index.json', default='package_index.json', type=str, help='Output package_*_index.json path')
	parser.add_argument('-v', '--version', metavar='x.y.z', default=None, type=str, help='Version number for add package')
	parser.add_argument('-b', '--base_version', metavar='x.y.z', default=None, type=str, help='Version number for package base')
	parser.add_argument('-p', '--platform_name', metavar='', default="", type=str, help='Platform name')
	parser.add_argument('-m', '--maintainer_name', metavar='', default="", type=str, help='Maintainer name')
	parser.add_argument('-s', '--package_suffix', metavar='', default="", type=str, help='Package suffix text')
	args = parser.parse_args()

	if args.archive == None:
		print("ERROR: Updated archive path missing.")
		exit()

	# Created archives
	update_archives = get_update_archives(args.archive)

	if len(update_archives) == 0:
		print("ERROR: Updated archive nothing.")
		exit()

	# New version number is required, if not set it program will exit.
	if args.version == None:
		print("ERROR: Version number missing.")
		exit()

	# Open current board manager json file
	pkg_json = load_json(args.input_json)

	# Spresense include only one package "SPRESENSE", so take first element
	package = get_first_package(pkg_json)

	# Append local name
	if args.package_suffix != "":
		package["name"] = "%s%s" % (package["name"], args.package_suffix)

	# Append maintainer name
	if args.maintainer_name != "":
		package["maintainer"] = args.maintainer_name

	# Add new platform item(New item are required if core lib is not updated.)
	# Take current platforms list
	platforms = get_platforms(package)

	# Sort platform list by version number
	platforms.sort(key=lambda x: getVersionTuple(x))

	# Get base platform by base_version
	base_platform = get_base_platform(platforms, args.base_version)

	# If argument 'url' is not set, use package_index.json's first URL.
	if args.url != '':
		base_url = args.url
	else:
		base_url = get_url_base(base_platform, args.version);

	# Clone platform item for create new platform
	new_platform = copy.deepcopy(base_platform)
	new_platform['version'] = args.version

	# Check platform file(packages/*/hardware/) update
	archive = "%s-v%s%s.tar.gz" % (new_platform['architecture'], args.version, args.package_suffix)
	if archive in update_archives:
		url = "%s/%s" % (base_url, archive)
		new_platform['archiveFileName'] = archive
		new_platform['url'] = url
		new_platform['size'] = get_archive_size("%s/%s" % (args.archive, archive))
		new_platform['checksum'] = get_archive_sha256_sum("%s/%s" % (args.archive, archive))

		# Append platform name
		if args.platform_name != "":
			new_platform["name"] = args.platform_name

	# Check tools dependency updates
	for tool in new_platform['toolsDependencies']:
		archive = "%s-v%s%s.tar.gz" % (tool['name'], args.version, args.package_suffix)
		if archive in update_archives:
			tool['packager'] = "%s%s" % (tool['packager'], args.package_suffix)
			tool['version'] = args.version

	if args.package_suffix != "":
		# If local package creating, clear all versions
		platforms.clear()

	# Add new platform into package_index.json
	platforms.append(new_platform)

	# Add new tool item(If tool is not updated, keep current)
	tools = get_tools(package)

	# Sort tool list by version number
	tools.sort(key=lambda x: getVersionTuple(x))

	# Check tools achive update
	toolDeps = get_tool_dependencies(base_platform)
	for key in toolDeps:
		archive = "%s-v%s%s.tar.gz" % (key, args.version, args.package_suffix)
		if archive in update_archives:
			# Clone tool item for create new item
			tool = copy.deepcopy(tools[0])
			tool['name'] = key
			tool['version'] = args.version

			systems = tool['systems']
			for system in systems:
				system['archiveFileName'] = archive
				system['url'] = "%s/%s" % (base_url, archive)
				system['size'] = get_archive_size("%s/%s" % (args.archive, archive))
				system['checksum'] = get_archive_sha256_sum("%s/%s" % (args.archive, archive))

			# Add new tool
			tools.append(tool)

	# Sort all elements
	platforms.sort(key=lambda x: getVersionTuple(x))
	tools.sort(key=lambda x: (x['name'], getVersionTuple(x)))

	# Write package_index.json
	output_json(pkg_json, args.output_json)
