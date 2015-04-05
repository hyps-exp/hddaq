#!/usr/bin/env python

import sys
import os
import signal
import subprocess
from struct import *
from Tkinter import *

class ChildProc(Frame):
  def __init__(self, name, cmd, arg):
    Frame.__init__(self)
    self.pack(pady=1)

    self.name   = name
    self.cmd    = cmd
    self.arg    = arg
    
    if (len(self.arg) > 0) :
      self.execmd = daqtop+self.cmd+' '+self.arg
    else :
      self.execmd = daqtop+self.cmd

    self.__make_button()
    
  def __make_button(self):
    self.name_label   = Label(self, text = self.name, font=('Helvetica', -20, ''), fg='blue', width=12)
    self.status_label = Label(self, text = 'DEAD', font=('Helvetica', -20, ''), fg='black', width=10)
    self.start_button = Button(self, text = 'START', command = self.start)
    self.start_button.config(state=DISABLED)
    self.stop_button  = Button(self, text = 'STOP', command = self.stop)
    self.stop_button.config(state=DISABLED)

    self.name_label.grid(row=0, column=0, padx=4)
    self.status_label.grid(row=0, column=1)
    self.start_button.grid(row=0, column=2)
    self.stop_button.grid(row=0, column=3, padx=4)
    
  def start(self):
    subprocess.Popen(self.execmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    self.status_label.config(text='RUNNING', fg='green')
    self.start_button.config(state=DISABLED)
    self.stop_button.config(state=NORMAL)
    
  def stop(self):
    subprocess.Popen(['pkill','-fx',self.execmd], stdout=subprocess.PIPE)
    self.status_label.config(text='DEAD', fg='black')
    self.start_button.config(state=NORMAL)
    self.stop_button.config(state=DISABLED)

  def check_process_status(self):
    p = subprocess.Popen(['pgrep','-fx',self.execmd], stdout=subprocess.PIPE)
    out = p.communicate()[0][0:-1]
    if( len(out) > 0 ) : 
      self.status_label.config(text='RUNNING', fg='green')
      self.start_button.config(state=DISABLED)
      self.stop_button.config(state=NORMAL)
    else :
      self.status_label.config(text='DEAD', fg='black')
      self.start_button.config(state=NORMAL)
      self.stop_button.config(state=DISABLED)

class ControllerProc(Frame):
  def __init__(self, name, arg):
    Frame.__init__(self)
    self.pack(pady=1)

    self.name   = name
    self.arg    = arg
    self.child  = None

    self.__make_button()
    
  def __make_button(self):
    self.name_label   = Label(self, text = self.name, font=('Helvetica', -20, ''), fg='blue', width=12)
    self.status_label = Label(self, text = 'DEAD', font=('Helvetica', -20, ''), fg='black', width=10)
    self.start_button = Button(self, text = 'START', command = self.start)
    self.stop_button  = Button(self, text = 'STOP', command = self.stop)
    self.stop_button.config(state=DISABLED)

    self.name_label.grid(row=0, column=0, padx=4)
    self.status_label.grid(row=0, column=1)
    self.start_button.grid(row=0, column=2)
    self.stop_button.grid(row=0, column=3, padx=4)
    
  def start(self):
    execmd = 'python -B '+daqtop+self.arg
    self.child = subprocess.Popen(execmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    self.status_label.config(text='RUNNING', fg='green')
    self.start_button.config(state=DISABLED)
    self.stop_button.config(state=NORMAL)
    
  def stop(self):
    self.child.send_signal(signal.SIGKILL)
    self.child = None
    self.status_label.config(text='DEAD', fg='black')
    self.start_button.config(state=NORMAL)
    self.stop_button.config(state=DISABLED)

  def check_process_status(self):
    if self.child is not None :
      if ( self.child.poll() != None ) :
        self.status_label.config(text='DEAD', fg='black')
        self.start_button.config(state=NORMAL)
        self.stop_button.config(state=DISABLED)

class App(Frame):
  def __init__(self):
    Frame.__init__(self)
    self.master.title('HDDAQ Launcher')
    self.pack(fill=BOTH, expand=True)
    
    self.childlist = []
    
  def makebuttons(self):
    label = Label(self, text='Data Storage Path: '+storage_path+'\n'+'=> '+os.path.realpath(storage_path),
                       fg='red', bg='black', font=('Helvetica', -20, ''))
    label.pack(side=TOP, pady=10, fill=X) 
  
  def entry(self, name, cmd, arg):
    p = ChildProc(name, cmd, arg)
    self.childlist.append(p)

  def controller_entry(self, name, arg):
    self.controller = ControllerProc(name, arg)

  def updater(self):
    for p in self.childlist :
      p.check_process_status()
    
    self.controller.check_process_status()

    self.after(500, self.updater)


if __name__ == "__main__":

  argvs = sys.argv
  argc = len(argvs)

  #check data storage directory
  storage_path = './data'

  if argc == 2 :
    storage_path = argvs[1]

  if not os.path.isdir(storage_path) :
    print 'Data Storage Path='+storage_path+' does not exist!!'
    sys.exit()
  

  # check tmplancher.py and lancher.py
  # storage_path=./data    => launcher.py
  # storage_path=./tmpdata => tmplauncher.py 
  if( storage_path == './data' ) :
    proc = subprocess.Popen(['pgrep','-f','python ./tmplauncher.py'], stdout=subprocess.PIPE)
    proc_out = proc.communicate()[0][0:-1]
    if( len(proc_out) > 0) :
      print 'tmplauncher.py is alreadly running!!'
      sys.exit()

  if( storage_path == './tmpdata') :
    proc = subprocess.Popen(['pgrep','-fx','python ./launcher.py'], stdout=subprocess.PIPE)
    proc_out = proc.communicate()[0][0:-1]
    if( len(proc_out) > 0) :
      print 'launcher.py is alreadly running!!'
      sys.exit()


  #dir path for HDDAQ programs 
  daqtop = os.path.dirname(os.path.realpath(__file__))+'/../'

  app = App()
  app.entry('MSGD','Message/bin/msgd', '')
  app.entry('CMSGD', 'Message/bin/cmsgd', 'msgnode.txt')
  app.entry('BUILDER', 'EventBuilder/bin/EventBuilder', '--idnumber=90000 datanode.txt')
  app.entry('DISTRIBUTOR', 'EventDistributor/bin/EventDistributor', '--node-id=80000')
  app.entry('RECORDER', 'Recorder/bin/Recorder', '--node-id=70000 --compress --dir='+storage_path)
  app.controller_entry('CONTROLLER', 'Controller/controller.py '+storage_path)

  app.makebuttons()
  app.updater()
  app.mainloop()

