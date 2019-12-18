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

FONT_SIZE = 14
BOX_SIZE = FONT_SIZE * 1.3
PADDING_H = 20
PADDING_V = 5

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

class CheckBox(wx.Panel):
	# Static definition for checkbox color metrix
	STATUS_DEFAULT_ID = 1
	STATUS_OVER_ID    = 2
	STATUS_PRESS_ID   = 3
	BOX_COLOR = {
		STATUS_DEFAULT_ID : {"forground": wx.Colour( 69,  69,  69), "background": wx.Colour(255, 255, 255)},
		STATUS_OVER_ID    : {"forground": wx.Colour( 23, 132, 219), "background": wx.Colour(255, 255, 255)},
		STATUS_PRESS_ID   : {"forground": wx.Colour( 18,  97, 161), "background": wx.Colour(204, 228, 247)},
	}

	# Checkbox status
	isChecked = False

	# Checkbox event callback
	cbCallback = None

	def createResizedBox(self, parent, decrease):
		size = BOX_SIZE - decrease * 2
		box = wx.Panel(parent, id=wx.ID_ANY)

		box.SetSize((size, size))
		box.SetPosition((decrease, decrease))

		return box

	def __init__(self, parent, id=-1, label=""):
		wx.Panel.__init__(self, parent, id)

		# Spacer for top
		spacer = wx.Panel(self, id=wx.ID_ANY)
		spacer.SetMinSize((PADDING_V, PADDING_V))

		# Box
		self.fore = self.createResizedBox(self, 0)
		self.back = self.createResizedBox(self.fore, 2)
		self.check = self.createResizedBox(self.fore, 5)

		# Label
		self.text = wx.StaticText(self, id=wx.ID_ANY, label=label)

		# Keep the size of checkbox
		def size(evt):
			self.fore.SetPosition(self.fore.GetPosition())
			self.fore.SetSize((BOX_SIZE, BOX_SIZE))
		self.fore.Bind(wx.EVT_SIZE, size)

		# Checkbox layout
		sizer = wx.GridBagSizer(0, PADDING_H / 2)

		sizer.Add(spacer,    (0, 0), (1, 2), flag=wx.EXPAND)
		sizer.Add(self.fore, (1, 0), (1, 1), flag=wx.GROW)
		sizer.Add(self.text, (1, 1), (1, 1), flag=wx.GROW)

		self.SetSizer(sizer)

		# Register event
		self.Bind(wx.EVT_ENTER_WINDOW, self.uiEventhandler)
		self.Bind(wx.EVT_LEAVE_WINDOW, self.uiEventhandler)
		self.Bind(wx.EVT_LEFT_DOWN,    self.uiEventhandler)
		self.Bind(wx.EVT_LEFT_UP,      self.uiEventhandler)

		# Initial state
		self.isEntered = False
		self.isPressed = False

		# Update UI by initial state
		self.UpdateCheckBoxStatus()

	def UpdateCheckBoxColor(self, id):
		self.fore.SetBackgroundColour(self.BOX_COLOR[id]["forground"])
		if self.isChecked:
			self.check.SetBackgroundColour(self.BOX_COLOR[id]["forground"])
		else:
			self.check.SetBackgroundColour(self.BOX_COLOR[id]["background"])
		self.back.SetBackgroundColour(self.BOX_COLOR[id]["background"])

		self.Refresh()

	def UpdateCheckBoxStatus(self):
		if self.isPressed and self.isEntered:
			self.UpdateCheckBoxColor(self.STATUS_PRESS_ID)
		elif self.isPressed:
			self.UpdateCheckBoxColor(self.STATUS_OVER_ID)
		elif self.isEntered:
			self.UpdateCheckBoxColor(self.STATUS_OVER_ID)
		else:
			self.UpdateCheckBoxColor(self.STATUS_DEFAULT_ID)

	def uiEventhandler(self, evt):
		if evt.EventType == wx.EVT_ENTER_WINDOW.typeId:
			self.isEntered = True
		elif evt.EventType == wx.EVT_LEAVE_WINDOW.typeId:
			self.isEntered = False
		elif evt.EventType == wx.EVT_LEFT_DOWN.typeId:
			self.isPressed = True
		elif evt.EventType == wx.EVT_LEFT_UP.typeId:
			self.isPressed = False
			if self.isEntered and self.cbCallback:
				self.isChecked = not self.isChecked
				self.cbCallback(self)
		self.UpdateCheckBoxStatus()

	def Bind(self, evtid, func):
		if evtid == wx.EVT_CHECKBOX:
			self.cbCallback = func
		else:
			wx.Panel.Bind(self, evtid, func)
			self.text.Bind(evtid, func)
			self.back.Bind(evtid, func)
			self.check.Bind(evtid, func)

	def SetFont(self, font):
		self.text.SetFont(font)

	def SetValue(self, value):
		self.isChecked = value

	def IsChecked(self):
		return self.isChecked

# Name       : EULAWindow
# Description: Show EULA binary Update Window
class EULAWindow(wx.Frame):

	def __init__(self, updater):

		# store accept checkbox status
		self.is_accepted = False

		wx.Frame.__init__(self, None, id=wx.ID_ANY, title=TITLE, style=wx.DEFAULT_FRAME_STYLE^wx.RESIZE_BORDER)

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

		# Top panel (for subject)
		top_panel = self.getDialogHeader(self)

		# HTML panel (for display EULA description)
		htm_panel = self.getDialogBody(self)

		# Footer panel
		footer_panel = self.getDialogFooter(self)

		# Layout
		main_sizer = wx.FlexGridSizer(rows=3, cols=1, gap=(0, 0))
		main_sizer.Add(top_panel, flag=wx.GROW)
		main_sizer.Add(htm_panel, flag=wx.GROW)
		main_sizer.Add(footer_panel, flag=wx.GROW)

		# Set free size for HTML view
		main_sizer.AddGrowableRow(1)
		main_sizer.AddGrowableCol(0)

		self.SetSizer(main_sizer)

		# Show
		self.Show()

	# Name       : getDialogHeader
	# Description: Create a header component
	def getDialogHeader(self, parent):
		# EULA description file path
		if hasattr(sys, '_MEIPASS'):
			logo_image_file = os.path.join(sys._MEIPASS, LOGO_IMAGE)
		else:
			logo_image_file = LOGO_IMAGE

		# Header anel (for subject)
		header = wx.Panel(parent, id=wx.ID_ANY, style=wx.SIMPLE_BORDER)

		# Backgroud color = White
		header.SetBackgroundColour(wx.WHITE)

		# Top label
		top = wx.Panel(header, id=wx.ID_ANY)

		# Subject label
		subject_txt = wx.StaticText(top, id=wx.ID_ANY, label=SUBJECT)
		self.setFontStyle(subject_txt, FONT_SIZE, wx.FONTWEIGHT_BOLD)

		# Spresense logo
		logo_l = self.getImageLabel(top, logo_image_file)

		# Size change event handler
		def logoResizer(evt):
			# Get current size
			base_size = evt.GetSize()
			image_size = logo_l.GetSize()

			# Set subject size/location
			subject_txt.SetPosition((0, 0))
			subject_txt.SetSize((base_size[0], image_size[1] * 2))

			# Set logo location
			logo_l.SetPosition((base_size[0] - image_size[0] - PADDING_V * 2, PADDING_V))

		# Set size change event handler
		top.Bind(wx.EVT_SIZE, logoResizer)

		# Description label
		desc_txt = wx.StaticText(header, id=wx.ID_ANY, label=EXPLAIN)
		self.setFontStyle(desc_txt, FONT_SIZE, wx.FONTWEIGHT_NORMAL)

		# Layout
		top_sizer = wx.BoxSizer(wx.VERTICAL)
		top_sizer.Add(top, 0, wx.EXPAND | wx.LEFT, PADDING_H)
		top_sizer.Add(desc_txt, 0, wx.EXPAND | wx.LEFT, PADDING_H * 2)

		header.SetSizer(top_sizer)

		return header

	# Name       : getDialogFooter
	# Description: Create a footer component
	def getDialogFooter(self, parent):
		# Footer panel (for take event)
		footer = wx.Panel(parent, id=wx.ID_ANY, style=wx.SIMPLE_BORDER)

		# Background color = ivory
		footer.SetBackgroundColour(wx.Colour(0xF3, 0xEC, 0xD8, 0xFF))

		# Accept check box
		accept_chk = CheckBox(footer, ACCEPT_CHK_ID, ACCEPT_CHK)
		self.setFontStyle(accept_chk, FONT_SIZE, wx.FONTWEIGHT_NORMAL)

		# Operation buttons panel
		buttons = wx.Panel(footer, id=wx.ID_ANY)

		# OK button
		self.ok_btn = wx.Button(buttons, OK_BTN_ID, "OK")
		self.setFontStyle(self.ok_btn, FONT_SIZE, wx.FONTWEIGHT_NORMAL)

		# Cancel button
		cn_btn = wx.Button(buttons, CANCEL_BTN_ID, "Cancel")
		self.setFontStyle(cn_btn, FONT_SIZE, wx.FONTWEIGHT_NORMAL)

		# Operation buttons ayout
		btn_sizer = wx.BoxSizer(wx.HORIZONTAL)
		btn_sizer.Add(self.ok_btn, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, PADDING_V)
		btn_sizer.Add(cn_btn, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, PADDING_V)
		buttons.SetSizer(btn_sizer)

		# Footer layout
		sizer = wx.BoxSizer(wx.VERTICAL)
		sizer.Add(accept_chk, 0, wx.LEFT, PADDING_H)
		sizer.Add(buttons, 0, wx.ALIGN_RIGHT | wx.RIGHT, PADDING_H)
		footer.SetSizer(sizer)

		# Set event handler
		parent.Bind(wx.EVT_BUTTON, self.eulaEventHandler)
		accept_chk.Bind(wx.EVT_CHECKBOX, self.eulaEventHandler)

		# Set default state
		accept_chk.SetValue(False)
		self.ok_btn.Disable()

		return footer

	# Name       : getDialogBody
	# Description: Create a dialog main contents
	def getDialogBody(self, parent):
		# EULA description file path
		if hasattr(sys, '_MEIPASS'):
			eula_desc_file = os.path.join(sys._MEIPASS, EULA_DESCRIPTION)
		else:
			eula_desc_file = EULA_DESCRIPTION

		# HTML panel (for display EULA description)
		htm_panel = wx.Panel(self, id=wx.ID_ANY, style=wx.SIMPLE_BORDER)

		# Create HTML view
		html = wx.html.HtmlWindow(htm_panel)

		# Set EULA HTML view from file
		eula_file = open(eula_desc_file, encoding='utf-8')
		html.SetPage(eula_file.read())
		eula_file.close()

		# Layout
		sizer = wx.BoxSizer(wx.HORIZONTAL)
		sizer.Add(html, 1, wx.EXPAND, 0)
		htm_panel.SetSizer(sizer)

		return htm_panel

	# Name       : setFontStyle
	# Description: Set a font style for component
	def setFontStyle(self, compnent, size, weight):
		font = compnent.GetFont()
		font.SetWeight(weight)
		font.SetPointSize(size)
		compnent.SetFont(font)

	# Name       : getImageLabel
	# Description: Get a image label
	def getImageLabel(self, parent, file_name):
		logo_image = wx.Image(file_name)
		image_size = logo_image.GetSize()
		image_bitmap = logo_image.ConvertToBitmap()
		label = wx.StaticBitmap(parent, id=wx.ID_ANY, bitmap=image_bitmap)
		label.SetSize(image_size[0], image_size[1])
		return label

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

