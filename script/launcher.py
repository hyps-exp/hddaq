#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys
import os
import signal
import subprocess
from struct import *
from Tkinter import *

#_______________________________________________________________________________
class ChildProc(Frame):
  #___________________________________________________________________________
  def __init__(self, name, cmd, arg):
    Frame.__init__(self)
    self.pack(pady=1)
    self.name   = name
    self.cmd    = cmd
    self.arg    = arg
    self.devnull = open(os.devnull, 'w')
    if len(self.arg) > 0:
      self.execmd = daqtop+self.cmd+' '+self.arg
    else:
      self.execmd = daqtop+self.cmd
    self.__make_button()
  #___________________________________________________________________________
  def __make_button(self):
    font1 = ('Helvetica', -20, '')
    self.name_label = Label(self, text=self.name, font=font1, fg='blue', width=15)
    self.status_label = Label(self, text='DEAD', font=font1, fg='black', width=10)
    self.start_button = Button(self, text='START', command = self.start)
    self.start_button.config(state=DISABLED)
    self.stop_button = Button(self, text='STOP', command = self.stop)
    self.stop_button.config(state=DISABLED)
    self.name_label.grid(row=0, column=0, padx=4)
    self.status_label.grid(row=0, column=1)
    self.start_button.grid(row=0, column=2)
    self.stop_button.grid(row=0, column=3, padx=4)
  #___________________________________________________________________________
  def start(self):
    subprocess.Popen(self.execmd, shell=True,
                     stdout=self.devnull, stderr=self.devnull)
    self.status_label.config(text='RUNNING', fg='green')
    self.start_button.config(state=DISABLED)
    self.stop_button.config(state=NORMAL)
  #___________________________________________________________________________
  def stop(self):
    subprocess.Popen(['pkill','-fx',self.execmd],
                     stdout=self.devnull, stderr=self.devnull)
    self.status_label.config(text='DEAD', fg='black')
    self.start_button.config(state=NORMAL)
    self.stop_button.config(state=DISABLED)
  #___________________________________________________________________________
  def check_process_status(self):
    p = subprocess.Popen(['pgrep','-fx',self.execmd],
                         stdout=subprocess.PIPE)
    out = p.communicate()[0][0:-1]
    if len(out) > 0:
      self.status_label.config(text='RUNNING', fg='green')
      self.start_button.config(state=DISABLED)
      self.stop_button.config(state=NORMAL)
    else:
      self.status_label.config(text='DEAD', fg='black')
      self.start_button.config(state=NORMAL)
      self.stop_button.config(state=DISABLED)

#_______________________________________________________________________________
class ControllerProc(Frame):
  #___________________________________________________________________________
  def __init__(self, name, arg):
    Frame.__init__(self)
    self.pack(pady=1)
    self.name = name
    self.arg = arg
    self.child = None
    self.devnull = open(os.devnull, 'w')
    self.__make_button()
    self.execmd = 'python -B '+daqtop+self.arg
  #___________________________________________________________________________
  def __make_button(self):
    font1 = ('Helvetica', -20, '')
    self.name_label = Label(self, text=self.name, font=font1, fg='blue', width=15)
    self.status_label = Label(self, text='DEAD', font=font1, fg='black', width=10)
    self.start_button = Button(self, text='START', command=self.start)
    self.stop_button = Button(self, text='STOP', command=self.stop)
    self.stop_button.config(state=DISABLED)
    self.name_label.grid(row=0, column=0, padx=4)
    self.status_label.grid(row=0, column=1)
    self.start_button.grid(row=0, column=2)
    self.stop_button.grid(row=0, column=3, padx=4)
  #___________________________________________________________________________
  def start(self):
    self.child = subprocess.Popen(self.execmd, shell=True)
    self.status_label.config(text='RUNNING', fg='green')
    self.start_button.config(state=DISABLED)
    self.stop_button.config(state=NORMAL)
  #___________________________________________________________________________
  def stop(self):
    self.child.send_signal(signal.SIGKILL)
    self.child = None
    self.change_to_dead()
  #___________________________________________________________________________
  def check_process_status(self):
    if self.child is None:
      self.change_to_dead()
    elif self.child.poll() is not None:
      self.change_to_dead()
  #___________________________________________________________________________
  def change_to_dead(self):
    self.status_label.config(text='DEAD', fg='black')
    self.start_button.config(state=NORMAL)
    self.stop_button.config(state=DISABLED)

#_______________________________________________________________________________
class App(Frame):
  #___________________________________________________________________________
  def __init__(self):
    Frame.__init__(self)
    self.master.title('HDDAQ Launcher (' + os.uname()[1] + ')')
    self.pack(fill=BOTH, expand=True)
    self.childlist = []
    self.controllerlist = []
  #___________________________________________________________________________
  def makebuttons(self):
    self.label = Label(self, text='Data Storage Path: '+storage_path+'\n'
                       +'=> '+os.path.realpath(storage_path),
                       fg='red', bg='black', font=('Helvetica', -20, ''))
    self.label.pack(side=TOP, pady=10, fill=X)
  #___________________________________________________________________________
  def entry(self, name, cmd, arg):
    p = ChildProc(name, cmd, arg)
    self.childlist.append(p)
  #___________________________________________________________________________
  def controller_entry(self, name, arg):
    p = ControllerProc(name, arg)
    self.controllerlist.append(p)
  #___________________________________________________________________________
  def updater(self):
    self.label.config(text='Data Storage Path: '+storage_path+'\n'
                      +'=> '+os.path.realpath(storage_path),
                      fg='red', bg='black', font=('Helvetica', -20, ''))
    for p in self.childlist:
      p.check_process_status()
    for p in self.controllerlist:
        p.check_process_status()
    self.after(500, self.updater)

#_______________________________________________________________________________
if __name__ == '__main__':
  argvs = sys.argv
  argc = len(argvs)
  '''check data storage directory'''
  storage_path = './data'
  if argc == 2:
    storage_path = argvs[1]
  if not os.path.isdir(storage_path):
    print('Data Storage Path must be directory : '+storage_path)
    quit()
  '''
  check tmplancher.py and lancher.py
  storage_path=./data    => launcher.py
  storage_path=./tmpdata => tmplauncher.py
  '''
  if storage_path == './data':
    proc = subprocess.Popen(['pgrep','-f','python ./tmplauncher.py'], stdout=subprocess.PIPE)
    proc_out = proc.communicate()[0][0:-1]
    path_list = './datapath.txt'
    if len(proc_out) > 0:
      print 'tmplauncher.py is alreadly running!!'
      sys.exit()
  if storage_path == './tmpdata' :
    proc = subprocess.Popen(['pgrep','-fx','python ./launcher.py'], stdout=subprocess.PIPE)
    proc_out = proc.communicate()[0][0:-1]
    path_list = './tmpdatapath.txt'
    if len(proc_out) > 0:
      print 'launcher.py is alreadly running!!'
      sys.exit()
  '''dir path for HDDAQ programs'''
  daqtop = os.path.dirname(os.path.realpath(__file__))+'/../'

  app = App()
  app.entry('MSGD','Message/bin/msgd', '')
  app.entry('CMSGD', 'Message/bin/cmsgd', 'msgnode.txt')
  app.entry('BUILDER', 'EventBuilder/bin/EventBuilder',
            '--idnumber=90000 datanode.txt')
  app.entry('DISTRIBUTOR', 'EventDistributor/bin/EventDistributor',
            '--node-id=80000')
  # app.entry('RECORDER', 'Recorder/bin/Recorder',
  #           '--node-id=70000 --dir='+storage_path)
  app.entry('RECORDER', 'Recorder/bin/Recorder',
            '--node-id=70000 --compress --dir='+storage_path)
  app.controller_entry('CONTROLLER',
                       'Controller/controller.py \
                        --data-path {0} --data-path-list {1} --mtm-host {2}'
                       .format(storage_path, path_list, '192.168.10.3'))
  app.makebuttons()
  app.updater()
  app.mainloop()
