# -*- coding: utf-8 -*-

from Tkinter import *
import os
import stat

#_______________________________________________________________________________
class TextWindow(Toplevel):
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
    '''create a Frame for the Text and Scrollbar'''
    self.txt_frm = Frame(self)
    self.txt_frm.pack(side=TOP, fill=BOTH, expand=True)
    '''create a Text widget'''
    self.txt = Text(self.txt_frm, font=('Courier', -12), height=20, width=109)
    self.txt.config(state=DISABLED)
    self.txt.pack(side=LEFT, fill=BOTH, expand=True)
    '''create a Scrollbar and associate it with txt'''
    self.scrollb = Scrollbar(self.txt_frm, command=self.txt.yview)
    self.txt.config(yscrollcommand=self.scrollb.set)
    self.scrollb.pack(side=LEFT, fill=Y)
  #_____________________________________________________________________________
  def deiconify(self):
    self.txt.see(END)
    Toplevel.deiconify(self)
  #_____________________________________________________________________________
  def AddText(self, line):
    pos = self.scrollb.get()
    self.txt.config(state=NORMAL)
    self.txt.insert(END, line)
    line_end = self.txt.index(END)
    if line_end > '500.0' :
      self.txt.delete(1.0, line_end+'-501lines')
    self.txt.config(state=DISABLED)
    if pos[1] > 0.99:
      self.txt.see(END)
  #_____________________________________________________________________________
  def GetLastText(self):
    return self.txt.get(END + '- 2lines', END +  '- 1lines')

#_______________________________________________________________________________
if __name__ == '__main__':
  root = Tk()
  msgwin = TextWindow('Message Window')

  button = Button(root, text='Message Window', command=msgwin.deiconify)
  button.pack()

  msgwin.AddText("AAAA\n")
  msgwin.AddText("BBB\n")
  msgwin.AddText("CC\n")

  print(msgwin.txt.get(END + '- 2lines', END +  '- 1lines'))

  root.mainloop()



