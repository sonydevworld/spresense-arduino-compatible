#!/usr/bin/env python3

import json
import os
import sys
import time
import webbrowser
import zipfile

import wx

# Name       : EULAFileDropHandler
# Description: For handle file drop event
class EULAFileDropHandler(wx.FileDropTarget):

	# Name       : __init__
	# Description: Initialize
	def __init__(self, window, updater):
		wx.FileDropTarget.__init__(self)
		self.window = window
		self.updater = updater

	# Name       : OnDropFiles
	# Description: Recieve file drop event
	def OnDropFiles(self, x, y, files):
		# Check all files
		for file in files:
			if self.updater.update(file):
				# If this is update file, drop a window
				self.window.Destroy()
				return True
		# There are nothing updater, send retry event
		self.window.retry()
		return False

TUTRIAL_IMAGE = "image/tutrial_image.png"

# Name       : EULAWindow
# Description: Show EULA binary Update Window
class EULAWindow(wx.Frame):
	def __init__(self, updater):
		wx.Frame.__init__(self, None, -1, "End User License Agreement", style=wx.STAY_ON_TOP|wx.DEFAULT_FRAME_STYLE)
		self.updater = updater

		# TODO: Layout will be update with layout manager
		display_size = wx.GetDisplaySize()

		# Image file path
		if hasattr(sys, '_MEIPASS'):
			tutrial_image_file = os.path.join(sys._MEIPASS, TUTRIAL_IMAGE)
		else:
			tutrial_image_file = TUTRIAL_IMAGE

		# Load label image
		image = wx.Image(tutrial_image_file)
		image_size = image.GetSize()

		window_width = image_size[0]
		window_height = image_size[1] + 50

		window_pos_x = (display_size[0] - window_width) / 2
		window_pos_y = (display_size[1] - window_height) / 2

		# parts
		image_bitmap = image.ConvertToBitmap()
		label = wx.StaticBitmap(self, -1, image_bitmap, (0,0))

		web_button = wx.Button(self, -1, "Go to download page")

		# Set Event hander
		label.SetDropTarget(EULAFileDropHandler(self, self.updater))
		self.Bind(wx.EVT_BUTTON, self.jumpToDOwnloadURL, web_button)

		# Set Layout
		self.SetSize(window_width, window_height)
		web_button.SetSize(window_width, 50)
		web_button.SetPosition((0, window_height - 50))

		self.SetPosition((window_pos_x, window_pos_y))
		self.Fit()
		self.Show()
	def retry(self):
		print("Retry")

	def jumpToDOwnloadURL(self, evt):
		webbrowser.open(self.updater.getDownloadURL())

# Name       : EULABinaryUpdater
# Description: Handle EULA binaries update from version.json
class EULABinaryUpdater():

	def __init__(self, firmware_path):
		self.firmware_path = firmware_path
		self.download_url = ""
		self.loader_version = ""

	# Name       : check
	# Description: Check current version and Requested version
	#   True     : different or nothing
	#   False    : Same
	def check(self):
		is_need_to_update = False
		version_file_name = "%s/version.json" % self.firmware_path
		current_file_name = "%s/stored_version.json" % self.firmware_path
		if os.path.isfile(version_file_name):
			version_file = open(version_file_name)
			version_json = json.load(version_file)
			version_file.close()
			version = version_json["LoaderVersion"]
			self.loader_version = version
			self.download_url = version_json["DownloadURL"]
			if os.path.isfile(current_file_name):
				current_file = open(current_file_name)
				current_json = json.load(current_file)
				current_file.close()
				current_version = current_json["LoaderVersion"]
				if version != current_version:
					is_need_to_update = True
			else:
				is_need_to_update = True
		return is_need_to_update

	# Name       : getTargetVersion
	# Description: Get target binary version
	def getTargetVersion(self):
		return self.loader_version

	# Name       : getDownloadURL
	# Description: Get download web URL
	def getDownloadURL(self):
		return self.download_url

	# Name       : update
	# Description: Update EULA binaries from zip archive
	#   True     : Update succeed
	#   False    : Update failed
	def update(self, file_path):
		ret = False
		print("FIle: %s" % file_path)
		# Check file is zip archive or not
		if file_path.endswith(".zip"):
			# Open zip archive
			binzip = zipfile.ZipFile(file_path)

			# Check stored_version.json contain or not
			if "stored_version.json" in binzip.namelist():
				# Check stored_version.json for compare with target version
				update_line = binzip.read("stored_version.json").decode('utf-8')
				update_json = json.loads(update_line)
				update_version = update_json["LoaderVersion"]
				if update_version == self.loader_version:
					# If same with target version, do update
					print("UPDATE")
					binzip.extractall(self.firmware_path)
					ret = True
		return ret

# Name       : EULAMain
# Description: Handle EULA updater
class EULAMain():

	# Name       : __init__
	# Description: Initialize
	def __init__(self, firmware_path):
		self.updater = EULABinaryUpdater(firmware_path)

	# Name       : main
	# Description: main routine for binary updater
	def main(self):
		# Check current binary version
		is_update_requred = self.updater.check()

		# If update is requred, show updater
		if is_update_requred:
			app = wx.App()
			EULAWindow(self.updater)
			app.MainLoop()

