import time

import TextWindow
import Message

all_msg_win     = TextWindow.TextWindow('Message Window [ALL]')
normal_msg_win  = TextWindow.TextWindow('Message Window [NORMAL]')
control_msg_win = TextWindow.TextWindow('Message Window [CONTROL]')
status_msg_win  = TextWindow.TextWindow('Message Window [STATUS]')
warning_msg_win = TextWindow.TextWindow('Message Window [WARNING]')
error_msg_win   = TextWindow.TextWindow('Message Window [ERROR]')
fatal_msg_win   = TextWindow.TextWindow('Message Window [FATAL]')


def AddMessage(linebuf):
  for line in linebuf:
    now, ms_type, src_id, body = line
  
    #message  = time.strftime('%Y %m/%d %H:%M:%S', time.localtime(now))
    #message += ' [%6d ] ' % src_id
    #message += body
    #message += '\n'

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

    all_msg_win.AddText(message)
  
    if   ms_type == Message.MT_NORMAL  : normal_msg_win.AddText(message)
    elif ms_type == Message.MT_CONTROL : control_msg_win.AddText(message)
    elif ms_type == Message.MT_STATUS  : status_msg_win.AddText(message)
    elif ms_type == Message.MT_WARNING : warning_msg_win.AddText(message)
    elif ms_type == Message.MT_ERROR   : error_msg_win.AddText(message)
    elif ms_type == Message.MT_FATAL   : fatal_msg_win.AddText(message)

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
