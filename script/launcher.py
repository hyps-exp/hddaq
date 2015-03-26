#!/usr/bin/env python

import sys
import os
import signal
import select
import subprocess
from struct import *
from Tkinter import *


class MessageWindow(Toplevel):
  def __init__(self, master=None, title=''):
    Toplevel.__init__(self, master)
    self.init(title)

  def init(self, title='Log Window'):
    self.title(title)
    #self.mb = Menubutton(self, text='Control')
    #self.menu = Menu(self.mb)
    #self.menu.add_command(label='Close', command=self.withdraw)
    #self.mb.config(menu=self.menu)

    self.ftext = Frame(self)
    self.textbox = Text(self.ftext, font=('fixed', -12), height=10)
    self.sbtext = Scrollbar(self.ftext, orient=VERTICAL, command=self.textbox.yview)
    self.textbox.config(yscrollcommand=self.sbtext.set)
    self.textbox.pack(side=LEFT, fill=BOTH, expand=True)
    self.sbtext.pack(side=LEFT, fill=Y)

    self.cbutton = Button(self, text='Close', command=self.withdraw) 

    #self.mb.pack(side=TOP, anchor=W)
    self.ftext.pack(side=TOP, fill=BOTH, expand=True)

    self.protocol('WM_DELETE_WINDOW', self.withdraw)
    self.withdraw()

  def quit(self):
    pass

  def dummy(self):
    pass


class ChildProc:
  def __init__(self, name, execfile):
    self.status = 'DEAD'
    self.name = name
    self.execfile = execfile
    self.updated = 0
    self.color = 'green'
    self.modified = 0
    self.logwindow = MessageWindow(None, name)

  def __del__(self):
    print 'Delete ChildProcess'

  def launchchild(self):
    self.child = subprocess.Popen(self.execfile,
                                  shell = True,
                                  stdin = subprocess.PIPE,
                                  stdout = subprocess.PIPE,
                                  stderr = subprocess.STDOUT,
                                  close_fds = True
                                  )
    app.rds.append(self.child.stdout)
    
    self.status = 'RUNNING'
    self.lstatus.config(text='RUNNING', fg='green')
    self.bstart.config(state=DISABLED)
    self.bstop.config(state=NORMAL)

  def killchild(self):
    if self.status == 'RUNNING' :
      self.child.send_signal(signal.SIGKILL)

  def viewlog(self):
    if self.status == 'RUNNING' :
      self.logwindow.textbox.insert(END, '\n---- ')
      self.logwindow.textbox.insert(END, self.status)
      self.logwindow.textbox.insert(END, ' PID: ')
      self.logwindow.textbox.insert(END, self.child.pid)
      self.logwindow.textbox.insert(END, ' ----\n')
    else :
      self.logwindow.textbox.insert(END, '\n---- ')
      self.logwindow.textbox.insert(END, self.status)
      self.logwindow.textbox.insert(END, ' ----\n')

    self.logwindow.textbox.insert(END,'\n')
    self.logwindow.textbox.see(END)
    self.logwindow.deiconify()
      

class App(Frame):
  def init(self):
    self.master.title('DAQ Launcher')
    #self.menu = Menubutton(self, text='Control')
    #menu1 = Menu(self.menu)
    #menu1.add_command(label='Quit', command=self.clean_and_quit)
    #self.menu.config(menu=menu1)
    #self.menu.pack(side=TOP, anchor=W)
    
  def __init__(self, master=None):
    
    self.storage_path='./data'

    self.rds = []
    self.childlist = []

    Frame.__init__(self, master)
    self.pack(fill=BOTH, expand=True)
    self.init()
 
  def set_data_path(self):
    self.argvs = sys.argv
    self.argc = len(self.argvs)
    
    if(self.argc == 2):
      if(os.path.isdir(self.argvs[1])):
        self.storage_path = self.argvs[1]
        return True
      else:
        self.storage_path = self.argvs[1]+' does not exist!!'
        return False
    else:
      if(os.path.isdir(self.storage_path)):
        return True
      else:
        self.storage_path = self.storage_path+' does not exist!!'
        return False

  def makebuttons(self):

    self.label = Label(self, text='Data Storage Path: '+self.storage_path+'\n'+'( => '+os.path.realpath(self.storage_path)+' )', fg='red', font=('Helvetica', -20, ''))
    self.label.pack(side=TOP, pady=10, fill=X) 

    #self.label2 = Label(self, text='')
    #self.label2.pack(side=TOP, fill=X) 

    self.la = []
    self.st = []
    self.startb = []
    self.stopb = []
    self.logb = []
    self.gframe = Frame(self)
    for i in range(len(self.childlist)) :
      self.la.append(Label(self.gframe, text = self.childlist[i].name, font=('Helvetica', -20, ''), fg='blue'))
      self.st.append(Label(self.gframe, text = self.childlist[i].status, font=('Helvetica', -20, ''), fg='black'))
      self.startb.append(Button(self.gframe, text = 'START',
        command=self.childlist[i].launchchild))
      self.stopb.append(Button(self.gframe, text = 'STOP',
        command=self.childlist[i].killchild))
      self.stopb[i].config(state=DISABLED)
      self.logb.append(Button(self.gframe, text = 'LOG',
        command=self.childlist[i].viewlog))
      self.la[i].grid(row=i, column=0)
      self.st[i].grid(row=i, column=1)
      self.childlist[i].lstatus = self.st[i]
      self.startb[i].grid(row=i, column=2)
      self.childlist[i].bstart = self.startb[i]
      self.stopb[i].grid(row=i, column=3)
      self.childlist[i].bstop = self.stopb[i]
      self.logb[i].grid(row=i, column=4)
    self.gframe.pack(side=TOP)

  def clean_and_quit(self):
    for i in self.childlist :
      if i.status == 'RUNNING' :
        i.killchild()
    self.quit()


  def reader(self):
    for i in self.childlist :
      if i.status == 'RUNNING' :
          if i.child.poll() != None :
            i.child.wait()
            self.rds.remove(i.child.stdout)
            i.status = 'DEAD'
            i.lstatus.config(text='DEAD', fg='black')
            i.bstart.config(state=NORMAL)
            i.bstop.config(state=DISABLED)

    rs, ws, es = select.select(self.rds, [], [], 0)
    
    for i in rs :
      is_match = 0
      for j in self.childlist :
         if j.status == 'RUNNING' :
           if j.child.stdout == i :
             lcp = j
             is_match = 1
             break
      if is_match == 1 :
        for k in range(128) :
          lrds = []
          lrds.append(i)
          lrs, lws, les = select.select(lrds, [], [], 0)
          if lrs != [] :
            onechar = i.read(1)
            if len(onechar) > 0 :
              lcp.logwindow.textbox.insert(END, onechar)
              lcp.logwindow.textbox.see(END)
              self.line_end = lcp.logwindow.textbox.index(END)
              if self.line_end > '550.0' :
                lcp.logwindow.textbox.delete(1.0, self.line_end + "- 500 lines")
          else :
            break

    self.after(1, self.reader)

  def watcher(self):
    for i in self.childlist :
      if i.status == 'RUNNING' :
        if i.modified == 1 :
          i.modified = 0
          if i.color == 'green' :
            i.lstatus.config(fg='darkgreen')
            i.color = 'darkgreen'
          else :
            i.lstatus.config(fg='green')
            i.color = 'green'
    self.after(1000, self.watcher)

  def entry(self, name, execfile):
    childp = ChildProc(name, execfile)
    self.childlist.append(childp)


if __name__ == "__main__":

  app = App()

  daqtop = os.path.dirname(os.path.realpath(__file__))+'/../'

  if(app.set_data_path()):
    app.entry('MSGD', daqtop+'Message/bin/msgd')
    app.entry('CMSGD', daqtop+'Message/bin/cmsgd msgnode.txt')
    app.entry('BUILDER', daqtop+'EventBuilder/bin/EventBuilder --idnumber=90000 datanode.txt')
    app.entry('DISTRIBUTOR', daqtop+'EventDistributor/bin/EventDistributor --node-id=80000')
    app.entry('RECORDER', daqtop+'Recorder/bin/Recorder --node-id=70000 --compress --dir='+app.storage_path)
    app.entry('CONTROLLER', 'python -B '+daqtop+'Controller/controller.py '+app.storage_path)
    print 'python -B '+daqtop+'Controller/controller.py '+app.storage_path
    
  app.makebuttons()
  app.reader()
  app.watcher()

  app.mainloop()

  sys.exit()
