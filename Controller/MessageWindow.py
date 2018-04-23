# -*- coding: utf-8 -*-

import time

from Tkinter import *
import TextWindow
import Message

#_______________________________________________________________________________
class MessageWindow():
  #_____________________________________________________________________________
  def __init__(self):
    self.all_msg_win = TextWindow.TextWindow('Message Window [ALL]')
    self.normal_msg_win = TextWindow.TextWindow('Message Window [NORMAL]')
    self.control_msg_win = TextWindow.TextWindow('Message Window [CONTROL]')
    self.status_msg_win = TextWindow.TextWindow('Message Window [STATUS]')
    self.warning_msg_win = TextWindow.TextWindow('Message Window [WARNING]')
    self.error_msg_win = TextWindow.TextWindow('Message Window [ERROR]')
    self.fatal_msg_win = TextWindow.TextWindow('Message Window [FATAL]')
    self.ms_normal_flag = BooleanVar()
    self.ms_control_flag = BooleanVar()
    self.ms_status_flag = BooleanVar()
    self.ms_warning_flag = BooleanVar()
    self.ms_error_flag = BooleanVar()
    self.ms_fatal_flag = BooleanVar()
    self.ms_normal_flag.set(True)
    self.ms_control_flag.set(True)
    self.ms_status_flag.set(False)
    self.ms_warning_flag.set(True)
    self.ms_error_flag.set(True)
    self.ms_fatal_flag.set(True)
    menubar = Menu()
    self.all_msg_win.config(menu=menubar)
    menubar.add_checkbutton(label='Normal', onvalue=True, offvalue=False,
                            variable=self.ms_normal_flag)
    menubar.add_checkbutton(label='Control', onvalue=True, offvalue=False,
                            variable=self.ms_control_flag)
    menubar.add_checkbutton(label='Status', onvalue=True, offvalue=False,
                            variable=self.ms_status_flag)
    menubar.add_checkbutton(label='Warning', onvalue=True, offvalue=False,
                            variable=self.ms_warning_flag)
    menubar.add_checkbutton(label='Error', onvalue=True, offvalue=False,
                            variable=self.ms_error_flag)
    menubar.add_checkbutton( label='Fatal', onvalue=True, offvalue=False,
                             variable=self.ms_fatal_flag)
  #_____________________________________________________________________________
  def AddMessage(self, linebuf):
    for line in linebuf:
      now, ms_type, src_id, body = line
      message  = time.strftime('%Y %m/%d %H:%M:%S', time.localtime(now))
      message += ' [%6d,' % src_id
      if   ms_type == Message.MT_NORMAL  : message += '  NORMAL'+' ] '
      elif ms_type == Message.MT_CONTROL : message += ' CONTROL'+' ] '
      elif ms_type == Message.MT_STATUS  : message += '  STATUS'+' ] '
      elif ms_type == Message.MT_WARNING : message += ' WARNING'+' ] '
      elif ms_type == Message.MT_ERROR   : message += '   ERROR'+' ] '
      elif ms_type == Message.MT_FATAL   : message += '   FATAL'+' ] '
      else                               : message += ' UNKNOWN'+' ] '
      message += body
      message += '\n'
      if ms_type == Message.MT_NORMAL:
        self.normal_msg_win.AddText(message)
        if self.ms_normal_flag.get():
          self.all_msg_win.AddText(message)
      elif ms_type == Message.MT_CONTROL:
        self.control_msg_win.AddText(message)
        if self.ms_control_flag.get():
          self.all_msg_win.AddText(message)
      elif ms_type == Message.MT_STATUS:
        self.status_msg_win.AddText(message)
        if self.ms_status_flag.get():
          self.all_msg_win.AddText(message)
      elif ms_type == Message.MT_WARNING:
        self.warning_msg_win.AddText(message)
        if self.ms_warning_flag.get():
          self.all_msg_win.AddText(message)
      elif ms_type == Message.MT_ERROR:
        self.error_msg_win.AddText(message)
        if self.ms_error_flag.get():
          self.all_msg_win.AddText(message)
      elif ms_type == Message.MT_FATAL:
        self.fatal_msg_win.AddText(message)
        if self.ms_error_flag.get():
          self.all_msg_win.AddText(message)

  #_____________________________________________________________________________
  def SaveMessage(self, logfile, linebuf):
    with open(logfile, 'a') as f:
      for line in linebuf:
        now, ms_type, src_id, body = line
        if ms_type == Message.MT_STATUS : continue
        message  = time.strftime('%Y %m/%d %H:%M:%S', time.localtime(now))
        message += ' [%6d,' % src_id
        if   ms_type == Message.MT_NORMAL : message += '  NORMAL'+' ] '
        elif ms_type == Message.MT_CONTROL: message += ' CONTROL'+' ] '
        elif ms_type == Message.MT_STATUS : message += '  STATUS'+' ] '
        elif ms_type == Message.MT_WARNING: message += ' WARNING'+' ] '
        elif ms_type == Message.MT_ERROR  : message += '   ERROR'+' ] '
        elif ms_type == Message.MT_FATAL  : message += '   FATAL'+' ] '
        else                              : message += ' UNKNOWN'+' ] '
        message += body
        message += '\n'
        f.write(message)

  #_____________________________________________________________________________
  def AddSaveMessage(self, logfile, linebuf):
    self.AddMessage(linebuf)
    self.SaveMessage(logfile, linebuf)
