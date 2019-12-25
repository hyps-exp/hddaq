# -*- coding: utf-8 -*-

import os
import datetime
import glob
import signal
import time
import threading
import Queue
import subprocess

from Tkinter import *
import TextWindow
import Message
import FileSystemUtility

#_______________________________________________________________________________
class DSyncController(Toplevel):
  #_____________________________________________________________________________
  def __init__(self, title):
    Toplevel.__init__(self)
    self.title(title)
    self.resizable(0,1)
    self.__make_layout()
    self.protocol('WM_DELETE_WINDOW', self.withdraw)
    self.withdraw()
  #_____________________________________________________________________________
  def __make_layout(self):
    font = ('Courier', -12)
    self.status_frame = Frame(self)
    self.status_frame.pack(side=TOP, fill=BOTH, expand=True)
    self.status_text = Text(self.status_frame, font=font, height=10, width=109)
    self.status_text.config(state=DISABLED)
    self.status_text.pack(side=LEFT, fill=BOTH, expand=True)
    self.status_scrollb = Scrollbar(self.status_frame,
                                    command=self.status_text.yview)
    self.status_text.config(yscrollcommand=self.status_scrollb.set)
    self.status_scrollb.pack(side=LEFT, fill=Y)
    self.log_frame = Frame(self)
    self.log_frame.pack(side=TOP, fill=BOTH, expand=True)
    self.log_text = Text(self.log_frame, font=font, height=10, width=109)
    self.log_text.config(state=DISABLED)
    self.log_text.pack(side=LEFT, fill=BOTH, expand=True)
    self.log_scrollb = Scrollbar(self.log_frame, command=self.log_text.yview)
    self.log_text.config(yscrollcommand=self.log_scrollb.set)
    self.log_scrollb.pack(side=LEFT, fill=Y)
  #_____________________________________________________________________________
  def deiconify(self):
    self.log_text.see(END)
    Toplevel.deiconify(self)
  #_____________________________________________________________________________
  def AddText(self, line):
    pos = self.log_scrollb.get()
    self.log_text.config(state=NORMAL)
    self.log_text.insert(END, line)
    line_end = self.log_text.index(END)
    if line_end > '500.0' :
      self.log_text.delete(1.0, line_end+'-501lines')
    self.log_text.config(state=DISABLED)
    if pos[1] > 0.99:
      self.log_text.see(END)

#_______________________________________________________________________________
class RsyncProcess():
  #_____________________________________________________________________________
  def __init__(self, src, dest):
    self.src = src
    self.dest = dest
    self.size = os.path.getsize(src)
    self.hsize = FileSystemUtility.natural_size(self.size)
    self.start = time.time()
    self.command = ('exec ionice -c 3 nice -n 19 rsync -a --partial --inplace '
                    + src + ' ' + dest)
    self.__lock = os.path.dirname(dest) + '/dsync.lock'
    self.__devnull = open(os.devnull, 'w')
    if (not os.path.isfile(self.__lock) and
        not FileSystemUtility.compare_file_size(src, dest)):
      open(self.__lock, 'w')
    else:
      self.command = 'exit 1'
    self.__process = subprocess.Popen(self.command, shell=True,
                                      stdout=self.__devnull,
                                      stderr=self.__devnull)
  #_____________________________________________________________________________
  def __del__(self):
    if 'rsync' in self.command and os.path.isfile(self.__lock):
      os.remove(self.__lock)
  #_____________________________________________________________________________
  def kill(self):
    self.__process.send_signal(signal.SIGKILL)
  #_____________________________________________________________________________
  def poll(self):
    return self.__process.poll()
  #_____________________________________________________________________________
  def is_synced(self):
    return (self.poll() == 0 and
            FileSystemUtility.compare_file_size(self.src, self.dest))

#_______________________________________________________________________________
class DataSynchronizer():
  #_____________________________________________________________________________
  def __init__(self, path, primary_path_list, secondary_path_list):
    self.msg_win = DSyncController('Data Synchronizer')
    self.menubar = Menu()
    self.msg_win.config(menu=self.menubar)
    self.menubar.add_command(label='Start', command=self.__start)
    self.menubar.add_command(label='Stop', state=DISABLED)
    self.menubar.add_command(label=' ' * 1, state=DISABLED)
    self.erase_flag = IntVar()
    self.erase_flag.set(0)
    self.menubar.add_checkbutton(label='Erase', onvalue=1, offvalue=0,
                                 variable=self.erase_flag)
    self.menubar.add_command(label=' ' * 3, state=DISABLED)
    self.menubar.add_command(label='IDLE',
                             background='#d9d9d9',
                             activebackground='#d9d9d9',
                             foreground='blue',
                             activeforeground='blue')
    self.menubar.add_command(label=' '*108, state=DISABLED)
    menu_force = Menu(self.menubar, tearoff=0)
    self.menubar.add_cascade(label='Force control', menu=menu_force)
    menu_force.add_command(label='Force erase dsync.lock',
                           command=self.__force_erase_lock)
    self.path = path
    self.primary_path_list = primary_path_list
    self.secondary_path_list = secondary_path_list
    self.message_q = Queue.Queue()
    self.proc = []
    self.max_process = len(self.secondary_path_list)
    self.state = 'IDLE'
    if len(self.secondary_path_list) == 0:
      self.menubar.entryconfig('Start', state=DISABLED)
      self.menubar.entryconfig('Stop', state=DISABLED)
  #_____________________________________________________________________________
  def __check(self, check_hash=False):
    n_sync = 0
    n_wait = 0
    remain_size = 0
    for src in self.get_data_list():
      r = int(src.replace('.dat', '').replace('.gz', '')[-5:])
      dest = (self.secondary_path_list[r % len(self.secondary_path_list)]
              + '/' + os.path.basename(src))
      if FileSystemUtility.compare_file_size(src, dest):
        n_sync += 1
        if self.erase == 1:
          if check_hash:
            shash = FileSystemUtility.get_hash(src)
            dhash = FileSystemUtility.get_hash(dest)
            if shash == dhash and self.state == 'RUNNING':
              self.__put('erased {0}'.format(src))
          else:
            self.__put('erased {0}'.format(src))
            os.remove(src)
      else:
        n_wait += 1
        remain_size += os.path.getsize(src)
    return n_sync, n_wait, remain_size
  #_____________________________________________________________________________
  def __force_erase_lock(self):
    self.__put('Force erase dsync.lock')
    for p in self.secondary_path_list:
      l = p + '/dsync.lock'
      if os.path.isfile(l):
        self.__put('erased {}'.format(l))
        os.remove(l)
      else:
        self.__put('no lock file in {}'.format(p))
  #_____________________________________________________________________________
  def __put(self, message):
    message = (time.strftime('%Y %m/%d %H:%M:%S')
               + ' ' + message.rstrip() + '\n')
    self.message_q.put(message)
  #_____________________________________________________________________________
  def __rsync(self):
    for i, src in enumerate(self.get_data_list()):
      if (self.state != 'RUNNING' or
          os.path.dirname(src) == os.path.realpath(self.path)):
        break
      while len(self.proc) >= self.max_process:
        for j in reversed(xrange(len(self.proc))):
          if self.proc[j].poll() is not None:
            if self.proc[j].is_synced():
              self.__put('synced {0:38} -> {1:20} {2:>12}\n'
                         .format(self.proc[j].src,
                                 os.path.dirname(self.proc[j].dest),
                                 self.proc[j].size))
            del self.proc[j]
      r = int(src.replace('.dat', '').replace('.gz', '')[-5:])
      dest = (self.secondary_path_list[r % len(self.secondary_path_list)]
              + '/' + os.path.basename(src))
      self.proc.append(RsyncProcess(src, dest))
      size = FileSystemUtility.natural_size(src)
    while len(self.proc) != 0:
      for j in reversed(xrange(len(self.proc))):
        if self.state != 'RUNNING':
          self.proc[j].kill()
        if self.proc[j].poll() is not None:
          if self.proc[j].is_synced():
            self.__put('synced {0:38} -> {1:20} {2:>12}\n'
                       .format(self.proc[j].src,
                               os.path.dirname(self.proc[j].dest),
                               self.proc[j].size))
          del self.proc[j]
  #_____________________________________________________________________________
  def __run(self):
    self.__put('run_thread start')
    while self.state == 'RUNNING':
      n_sync, n_wait, remain_size = self.__check()
      if n_wait > 0:
        self.__rsync()
      else:
        time.sleep(0.5)
    self.__put('run_thread stop')
  #_____________________________________________________________________________
  def __start(self):
    if self.state == 'IDLE':
      self.menubar.entryconfig('Start', state=DISABLED)
      self.menubar.entryconfig('Stop', state=NORMAL, command=self.__stop)
      self.state = 'RUNNING'
      self.menubar.insert_command('IDLE', label='RUNNING',
                          background='#d9d9d9',
                          activebackground='#d9d9d9',
                          foreground='red',
                          activeforeground='red')
      self.menubar.delete('IDLE')
      self.menubar.insert_command('Force control', label=' '*101, state=DISABLED)
      self.menubar.delete(' '*108)
      self.run_thread = threading.Thread(target=self.__run)
      self.run_thread.setDaemon(True)
      self.run_thread.start()
  #_____________________________________________________________________________
  def __stop(self):
    if self.state == 'RUNNING':
      self.menubar.entryconfig('Start', state=NORMAL, command=self.__start)
      self.menubar.entryconfig('Stop', state=DISABLED)
      self.state = 'IDLE'
      self.menubar.insert_command('RUNNING', label='IDLE',
                                  background='#d9d9d9',
                                  activebackground='#d9d9d9',
                                  foreground='blue',
                                  activeforeground='blue')
      self.menubar.delete('RUNNING')
      self.menubar.insert_command('Force control', label=' '*108, state=DISABLED)
      self.menubar.delete(' '*101)
      self.run_thread.join()
  #_____________________________________________________________________________
  def get_message(self):
    linebuf = []
    while not self.message_q.empty():
      linebuf.append(self.message_q.get())
    return linebuf
  #_____________________________________________________________________________
  def get_data_list(self):
    data_list = []
    for p in self.primary_path_list:
      if os.path.realpath(self.path) == p:
          continue
      data_list += glob.glob(p + '/*.dat*')
    data_list.sort()
    return data_list
  #_____________________________________________________________________________
  def update(self):
    self.erase = self.erase_flag.get()
    message = time.strftime('%Y %m/%d %H:%M:%S')
    message += ('{0:>28} {1:>8} {2:>8} {3:>7} {4:>6} {5:>10}\n'
                .format('Free(GB)', 'Used(GB)',
                        'Usage', 'Sync', 'Wait', 'RSize'))
    max_usage_p = 0
    max_usage_s = 0
    for p in self.primary_path_list:
      n_sync = 0
      n_wait = 0
      remain_size = 0
      data_list = glob.glob(p + '/*.dat*')
      data_list.sort()
      for i, f in enumerate(data_list):
        r = int(f.replace('.dat', '').replace('.gz', '')[-5:])
        dest = (self.secondary_path_list[r % len(self.secondary_path_list)]
                + '/' + os.path.basename(f))
        if FileSystemUtility.compare_file_size(f, dest):
          n_sync += 1
        else:
          n_wait += 1
          remain_size += os.path.getsize(f)
      free, used, total, usage = FileSystemUtility.get_disk_usage(p)
      max_usage_p = max(max_usage_p, usage)
      message += 'Primary   '
      message += ('{0:28} {1:8} {2:8}    {3:5.1%}'
                  .format(p, free, used, usage))
      message += ('{0:8} {1:6} {2:>10}'
                  .format(n_sync, n_wait,
                          FileSystemUtility.natural_size(remain_size)))
      if os.path.realpath(self.path) == p:
        message += '  @@@ CURRENT @@@'
      message += '\n'
    for p in self.secondary_path_list:
      free, used, total, usage = FileSystemUtility.get_disk_usage(p)
      max_usage_s = max(max_usage_s, usage)
      message += 'Secondary '
      message += ('{0:28} {1:8} {2:8}    {3:5.1%}'
                  .format(p, free, used, usage))
      message += ('{0:8} {1:6} {2:>10}'
                  .format(len(glob.glob(p + '/*.dat*')), '', ''))
      if os.path.isfile(p + '/dsync.lock'):
        message += '  %%% SYNCING %%%'
      message += '\n'
    message += '{0:>3} Processes running\n'.format(len(self.proc))
    for p in self.proc:
      elapsed_time = time.time() - p.start
      if not 'rsync' in p.command:
        continue
      src = p.command.split()[-2]
      dest = p.command.split()[-1]
      src_size = FileSystemUtility.natural_size(src)
      dest_size = FileSystemUtility.natural_size(dest)
      speed = FileSystemUtility.file_size(dest) / 1e6 / elapsed_time
      message += ('Running ... {0:38} -> {1:12} {2:>9}/{3:>9} ({4:.1f} MB/s)'
                  .format(src, os.path.dirname(dest),
                          dest_size, src_size, speed))
      message += '\n'
    if max_usage_p > 0.50 or max_usage_s > 0.75:
      fg_color = 'yellow'
      bg_color = 'black'
      font = ('Courier', -12, 'bold')
      flush = True
    elif max_usage_p > 0.75 or max_usage_s > 0.90:
      fg_color = 'red'
      bg_color = 'black'
      font = ('Courier', -12, 'bold')
      flush = True
    else:
      fg_color = 'black'
      bg_color = 'white'
      font = ('Courier', -12)
      flush = False
    fg = self.msg_win.status_text.cget('fg')
    bg = self.msg_win.status_text.cget('bg')
    if flush and fg == fg_color and bg == bg_color:
      fg_color = bg
      bg_color = fg
    self.msg_win.status_text.config(state=NORMAL, fg=fg_color, bg=bg_color,
                                    font=font)
    self.msg_win.status_text.delete(1.0, END)
    message = message.rstrip()
    for line in message:
      if not line:
        continue
      self.msg_win.status_text.insert(END, line)
    self.msg_win.status_text.config(state=DISABLED)
    self.msg_win.status_text.see(1.0)
  #_____________________________________________________________________________
  def AddMessage(self, linebuf):
    for line in linebuf:
      self.msg_win.AddText(line)
  #_____________________________________________________________________________
  def SaveMessage(self, logfile, linebuf):
    with open(logfile, 'a') as f:
      for line in linebuf:
        if not line:
          continue
        f.write(line)
  #_____________________________________________________________________________
  def AddSaveMessage(self, logfile, linebuf):
    self.AddMessage(linebuf)
    self.SaveMessage(logfile, linebuf)
