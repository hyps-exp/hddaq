import time
import TextWindow

comment_win = TextWindow.TextWindow('Comment Window')

def AddSaveComment(logfile, runno, line):
    comment  = time.strftime('%Y %m/%d %H:%M:%S')
    comment += ' [RUN ' + str(runno).zfill(5) + '] '
    comment += line + '\n' 

    comment_win.AddText(comment)

    f = open(logfile, 'a')
    f.write(comment)
    f.close()

def ShowComment(line):
    comment_win.AddText(line)

def GetLastComment():
    text = comment_win.GetLastText()
    text = text[40:-1]
    return text
