#!/usr/bin/env python3

import json
import os
import sys
import time
import zipfile

import wx
import wx.html

WINDOW_WIDTH  = 700
WINDOW_HEIGHT = 500

# String definitions
TITLE            = "End-User License Agreement"
SUBJECT          = "License Agreement"
EXPLAIN          = "Please read the folowing license agreement carefully."
ACCEPT_CHK       = "I Accept the terms in the license agreement"

# File path
EULA_DESCRIPTION = "text/eula_description.txt"
LOGO_IMAGE       = "image/SPRESENSE.png"

# Event ID definitions
OK_BTN_ID        = 1
CANCEL_BTN_ID    = 2
ACCEPT_CHK_ID    = 3

# Name       : EULAWindow
# Description: Show EULA binary Update Window
class EULAWindow(wx.Frame):
	def __init__(self, updater):

		# EULA description file path
		if hasattr(sys, '_MEIPASS'):
			eula_desc_file = os.path.join(sys._MEIPASS, EULA_DESCRIPTION)
			logo_image_file = os.path.join(sys._MEIPASS, LOGO_IMAGE)
		else:
			eula_desc_file = EULA_DESCRIPTION
			logo_image_file = LOGO_IMAGE

		# store accept checkbox status
		self.is_accepted = False

		wx.Frame.__init__(self, None, -1, TITLE, style=wx.STAY_ON_TOP|wx.DEFAULT_FRAME_STYLE^wx.RESIZE_BORDER)
		self.updater = updater

		# Get current display size
		display_size = wx.GetDisplaySize()

		# Set window size for layout
		window_width = WINDOW_WIDTH
		window_height = WINDOW_HEIGHT

		# Centering in display
		window_pos_x = (display_size[0] - window_width) / 2
		window_pos_y = (display_size[1] - window_height) / 2

		# Set main window layout
		self.SetSize(window_width, window_height)
		self.SetPosition((window_pos_x, window_pos_y))

		# Define 3 panels layout
		(top_x, top_y, top_w, top_h) = (0, 0, window_width, window_height / 10)
		(htm_x, htm_y, htm_w, htm_h) = (0, top_y + top_h, window_width, window_height * 7.5 / 10)
		(ope_x, ope_y, ope_w, ope_h) = (0, htm_y + htm_h, window_width, window_height - top_h - htm_h)

		# Top panel (for subject)
		top_panel = wx.Window(self, -1, style=wx.SIMPLE_BORDER)
		top_panel.SetBackgroundColour(wx.WHITE)
		top_panel.SetSize(top_w, top_h)
		top_panel.SetPosition((top_x, top_y))

		subject_txt = wx.StaticText(top_panel, -1, SUBJECT)
		font = subject_txt.GetFont()
		font.SetWeight(wx.FONTWEIGHT_BOLD)
		font.SetPointSize(14)
		subject_txt.SetFont(font)
		subject_txt.SetSize(top_w, top_h / 2)
		subject_txt.SetPosition((20, 0))

		desc_txt = wx.StaticText(top_panel, -1, EXPLAIN)
		font = desc_txt.GetFont()
		font.SetPointSize(14)
		desc_txt.SetFont(font)
		desc_txt.SetSize(top_w, top_h / 2)
		desc_txt.SetPosition((40, top_h / 2))

		s_logo_img = wx.Image(logo_image_file)
		image_size = s_logo_img.GetSize()
		image_bitmap = s_logo_img.ConvertToBitmap()
		logo_l = wx.StaticBitmap(top_panel, -1, image_bitmap)
		logo_l.SetSize(image_size[0], image_size[1])
		logo_l.SetPosition((top_w - image_size[0] - 10, 5))

		# HTML panel (for display EULA description)
		htm_panel = wx.Window(self, -1, style=wx.SIMPLE_BORDER)
		htm_panel.SetBackgroundColour(wx.Colour(0xF3, 0xEC, 0xD8, 0xFF))
		htm_panel.SetSize(htm_w, htm_h)
		htm_panel.SetPosition((htm_x, htm_y))

		# Operation panel (for take event)
		ope_panel = wx.Window(self, -1, style=wx.SIMPLE_BORDER)
		ope_panel.SetBackgroundColour(wx.Colour(0xF3, 0xEC, 0xD8, 0xFF))
		ope_panel.SetSize(ope_w, ope_h)
		ope_panel.SetPosition((ope_x, ope_y))

		accept_chk = wx.CheckBox(ope_panel, ACCEPT_CHK_ID, ACCEPT_CHK)
		accept_chk.SetValue(False)
		font = accept_chk.GetFont()
		font.SetPointSize(14)
		accept_chk.SetFont(font)
		accept_chk.SetSize(ope_w, ope_h / 2 - 10)
		accept_chk.SetPosition((20, 0))

		self.ok_btn = wx.Button(ope_panel, OK_BTN_ID, "OK")
		font = self.ok_btn.GetFont()
		font.SetPointSize(14)
		self.ok_btn.SetFont(font)
		self.ok_btn.SetSize(70, 40)
		self.ok_btn.SetPosition((ope_w - 180, ope_h / 2 - 10))

		# Disable 'OK' button by default
		self.ok_btn.Disable()

		cn_btn = wx.Button(ope_panel, CANCEL_BTN_ID, "Cancel")
		font = cn_btn.GetFont()
		font.SetPointSize(14)
		cn_btn.SetFont(font)
		cn_btn.SetSize(70, 40)
		cn_btn.SetPosition((ope_w - 100, ope_h / 2 - 10))

		# Set event handler
		self.Bind(wx.EVT_BUTTON, self.eulaEventHandler, self.ok_btn)
		self.Bind(wx.EVT_BUTTON, self.eulaEventHandler, cn_btn)
		self.Bind(wx.EVT_CHECKBOX, self.eulaEventHandler, accept_chk)

		# Create HTML view
		html = wx.html.HtmlWindow(htm_panel)
		html.SetSize(htm_w, htm_h)
		html.SetPosition((0, 0))

		# Set EULA HTML view from file
		eula_file = open(eula_desc_file, encoding='utf-8')
		html.SetPage(eula_file.read())
		eula_file.close()

		# Show
		self.Fit()
		self.Show()

	# Name       : eulaEventHandler
	# Description: Handle EULA window events
	def eulaEventHandler(self, evt):
		if evt.Id == ACCEPT_CHK_ID:
			self.is_accepted = evt.IsChecked()
			if self.is_accepted:
				self.ok_btn.Enable()
			else:
				self.ok_btn.Disable()
		elif evt.Id == OK_BTN_ID:
			if self.is_accepted:
				print("License agreement accepted.")
				self.updater.update()
				self.Destroy()
			else:
				print("License agreement not accepted yet.")
		elif evt.Id == CANCEL_BTN_ID:
			print("License agreement canceled.")
			self.Destroy()

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
	def update(self):
		ret = False
		file_path = os.path.join(self.firmware_path, "firmware.zip")
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

