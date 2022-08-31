# -*- coding: utf-8 -*-

import time
import TextWindow

comment_win = TextWindow.TextWindow('Comment Window')

#_______________________________________________________________________________
def AddSaveComment(logfile, runno, line):
  comment  = time.strftime('%Y %m/%d %H:%M:%S')
  comment += ' [RUN ' + str(runno).zfill(5) + '] '
  comment += line + '\n'
  comment_win.AddText(comment)
  with open(logfile, 'a') as f:
    f.write(comment)

#_______________________________________________________________________________
def ShowComment(line):
  comment_win.AddText(line)

#_______________________________________________________________________________
def GetLastComment():
  text = comment_win.GetLastText()
  text = text[40:-1]
  return text
