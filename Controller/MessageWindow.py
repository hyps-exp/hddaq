import time

from Tkinter import *
import TextWindow
import Message

normal_msg_win  = TextWindow.TextWindow('Message Window [NORMAL]')
control_msg_win = TextWindow.TextWindow('Message Window [CONTROL]')
status_msg_win  = TextWindow.TextWindow('Message Window [STATUS]')
warning_msg_win = TextWindow.TextWindow('Message Window [WARNING]')
error_msg_win   = TextWindow.TextWindow('Message Window [ERROR]')
fatal_msg_win   = TextWindow.TextWindow('Message Window [FATAL]')

all_msg_win     = TextWindow.TextWindow('Message Window [ALL]')
ms_normal_flag = BooleanVar()
ms_normal_flag.set(True)
ms_control_flag = BooleanVar()
ms_control_flag.set(True)
ms_status_flag = BooleanVar()
ms_status_flag.set(True)
ms_warning_flag = BooleanVar()
ms_warning_flag.set(True)
ms_error_flag = BooleanVar()
ms_error_flag.set(True)
ms_fatal_flag = BooleanVar()
ms_fatal_flag.set(True)
menubar = Menu()
all_msg_win.config(menu=menubar)
menubar.add_checkbutton(label='Normal',onvalue=True,offvalue=False,variable=ms_normal_flag)
menubar.add_checkbutton(label='Control',onvalue=True,offvalue=False,variable=ms_control_flag)
menubar.add_checkbutton(label='Status',onvalue=True,offvalue=False,variable=ms_status_flag)
menubar.add_checkbutton(label='Warning',onvalue=True,offvalue=False,variable=ms_warning_flag)
menubar.add_checkbutton(label='Error',onvalue=True,offvalue=False,variable=ms_error_flag)
menubar.add_checkbutton(label='Fatal',onvalue=True,offvalue=False,variable=ms_fatal_flag)

def AddMessage(linebuf):
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

    if   ms_type == Message.MT_NORMAL  :
      normal_msg_win.AddText(message)
      if ms_normal_flag.get() : all_msg_win.AddText(message)

    elif ms_type == Message.MT_CONTROL :
      control_msg_win.AddText(message)
      if ms_control_flag.get() : all_msg_win.AddText(message)

    elif ms_type == Message.MT_STATUS  :
      status_msg_win.AddText(message)
      if ms_status_flag.get()  : all_msg_win.AddText(message)

    elif ms_type == Message.MT_WARNING :
      warning_msg_win.AddText(message)
      if ms_warning_flag.get()  : all_msg_win.AddText(message)

    elif ms_type == Message.MT_ERROR   :
      error_msg_win.AddText(message)
      if ms_error_flag.get()  : all_msg_win.AddText(message)

    elif ms_type == Message.MT_FATAL   :
      fatal_msg_win.AddText(message)
      if ms_error_flag.get()  : all_msg_win.AddText(message)

def SaveMessage(logfile, linebuf):
  f = open(logfile, 'a')
  
  for line in linebuf:
    now, ms_type, src_id, body = line

    if ms_type == Message.MT_STATUS : continue
    
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
    f.write(message)

  f.close()  

def AddSaveMessage(logfile, linebuf):
  AddMessage(linebuf)
  SaveMessage(logfile, linebuf)
