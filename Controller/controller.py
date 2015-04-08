from Tkinter import *

import os
import sys
import time
import threading

import MTMController
import MessageHandler
import MessageWindow
import CommentWindow
import StatusList

class Controller(Frame):
    def __init__(self, path):
        Frame.__init__(self)
        self.master.title('HDDAQ Controller')
        self.master.resizable(0,1)
        self.pack(fill=Y, expand=True)
        
        #make widgets
        self.__make_menu()
        self.__make_labels()
        self.__make_comment_input()
        self.__make_buttons()
        self.__make_status_view()

        #check directory tree
        self.message_dir = path+'/Messages'
        if(os.path.isdir(self.message_dir) == False):
            os.mkdir(self.message_dir)

        misc_dir = path+'/misc'
        if(os.path.isdir(misc_dir) == False):
            os.mkdir(misc_dir)
            
        self.comment_file = misc_dir+'/comment.txt'
        if(os.path.isfile(self.comment_file) == False):
            f=open(self.comment_file,'w')
            f.write('')
            f.close

        self.runno_file =  misc_dir+'/runno.txt'
        if(os.path.isfile(self.runno_file) == False):
            self.set_runno(0)

        self.maxevent_file =  misc_dir+'/maxevent.txt'
        if(os.path.isfile(self.maxevent_file) == False):
            self.set_maxevent(10000000)
            
        self.trig_file =  misc_dir+'/trig.txt'
        if(os.path.isfile(self.trig_file) == False):
            self.set_trig_state('OFF')

        self.starttime_file =  misc_dir+'/starttime.txt'
        if(os.path.isfile(self.starttime_file) == False):
            self.set_starttime('')
            
        #control flags
        self.daq_state =-1
        self.daq_start_flag = 0
        self.daq_auto_restart_flag = 0
        self.master_controller_flag = 0

        #time stamps
        self.comment_timestamp =  time.time()
        self.runno_timestamp =  time.time()
        self.maxevent_timestamp =  time.time()
        self.trig_timestamp =  time.time()
        self.starttime_timestamp =  time.time()
        self.undertransition_timestamp =  time.time()

    def __make_menu(self):
        menubar= Menu(self)    
        self.master.config(menu=menubar)

        menu1 = Menu(menubar, tearoff=0)
        menu2 = Menu(menubar, tearoff=0)
        menu3 = Menu(menubar, tearoff=0)
        menu4 = Menu(menubar, tearoff=0)
        menu5 = Menu(menubar, tearoff=0)
        menu6 = Menu(menubar, tearoff=0)
        menubar.add_cascade(label='Control',menu=menu1)
        menubar.add_cascade(label='Comment',menu=menu2)
        menubar.add_cascade(label='Message',menu=menu3)
        menubar.add_cascade(label='DAQ mode',menu=menu4)
        menubar.add_cascade(label='Options',menu=menu5)
               #space
        menubar.add_cascade(state=DISABLED,label='                                         ')
        menubar.add_cascade(state=DISABLED,label='                                          ')
               #space
        menubar.add_cascade(label='Force control',menu=menu6)

        #control
        menu1.add_command(label='Clean List', command=self.clean_list_command)
        menu1.add_separator()
        menu1.add_command(label='Restart Frontend', command=lambda:msgh.send_message('fe_end\0'))
        menu1.add_separator()
        menu1.add_command(label='Quit', command=self.master.quit)
    
        #force control
        menu6.add_command(label='Force Restart Frontend', command=lambda:msgh.send_message('fe_exit\0'))
        menu6.add_separator()
        menu6.add_command(label='Force Start', command=self.start_command)
        menu6.add_command(label='Force Stop', command=self.stop_command)
        menu6.add_separator()
        menu6.add_command(label='Force Trig. ON', command=self.trigon_command)
        menu6.add_command(label='Force Trig. OFF', command=self.trigoff_command)
        menu6.add_command(label='Force MTM reset', command=MTMController.mtm_reset)

        #comment
        menu2.add_command(label='Write Comment', command=self.write_run_comment)
        menu2.add_command(label='Load Last Comment', command=self.load_last_comment)
        menu2.add_separator()
        menu2.add_command(label='Comment Window', command=CommentWindow.comment_win.deiconify)

        #window
        menu3.add_command(label='Message Window [ALL]', command=MessageWindow.all_msg_win.deiconify)
        menu3.add_command(label='Message Window [NORMAL]', command=MessageWindow.normal_msg_win.deiconify)
        menu3.add_command(label='Message Window [CONTROL]', command=MessageWindow.control_msg_win.deiconify)
        menu3.add_command(label='Message Window [STATUS]', command=MessageWindow.status_msg_win.deiconify)
        menu3.add_command(label='Message Window [WARNING]', command=MessageWindow.warning_msg_win.deiconify)
        menu3.add_command(label='Message Window [ERROR]', command=MessageWindow.error_msg_win.deiconify)
        menu3.add_command(label='Message Window [FATAL]', command=MessageWindow.fatal_msg_win.deiconify)

        #daq mode
        menu4.add_command(label='Normal', command=lambda:msgh.send_message('normal_mode\0'))
        menu4.add_command(label='Dummy', command=lambda:msgh.send_message('dummy_mode\0'))
        
        #options
        self.auto_trig_on = IntVar()
        self.auto_trig_on.set(0)
        self.auto_restart = IntVar()
        self.auto_restart.set(0)
        menu5.add_checkbutton(label='Auto Trig. ON', onvalue=1, offvalue=0, variable=self.auto_trig_on)
        menu5.add_checkbutton(label='Auto Restart', onvalue=1, offvalue=0, variable=self.auto_restart)
        
    def __make_labels(self):
        #DAQ label
        self.label = Label(self, text='DAQ: Idle', bg='black', fg='blue', font=('Helvetica', -24,'bold'))
        self.label.pack(side=TOP, fill=X)
        #Last Run Start Time:
        self.lasttime = Label(self, text='Last Run Start Time:')
        self.lasttime.pack(side=TOP,pady=10)

    def __make_comment_input(self):
        fcomment = Frame(self)
        fcomment.pack(side=TOP)
        lcomment = Label(fcomment, text='Comment:')
        lcomment.pack(side=LEFT)
        self.comment = Entry(fcomment, width=86)
        self.comment.pack(side=LEFT)

    def __make_buttons(self):
        fbuttons = Frame(self)
        fbuttons.pack(side=TOP)

        self.bstart = Button(fbuttons, text='Start', font=('Helvetica', -24, 'bold'), command=self.start_command)
        self.bstart.config(state=DISABLED)
        self.bstop = Button(fbuttons, text='Stop', font=('Helvetica', -24, 'bold'), command=self.stop_command)
        self.bstop.config(state=DISABLED)
        self.btrigon = Button(fbuttons, text="Trig. ON", font=('Helvetica', -16, 'normal'), command=self.trigon_command)
        self.btrigon.config(state=DISABLED)
        self.btrigoff = Button(fbuttons, text="Trig. OFF", font=('Helvetica', -16, 'normal'), command=self.trigoff_command)
        self.btrigoff.config(state=DISABLED)
        
        frunno = Frame(fbuttons)
        lrunno = Label(frunno, text='Run Number')
        lrunno.pack()
        self.runno_e = Entry(frunno, justify=RIGHT, width=12, disabledbackground='white', disabledforeground='black' )
        self.runno_e.config(state=DISABLED)
        self.runno_e.pack()
        
        fmaxevent = Frame(fbuttons)
        lmaxevent = Label(fmaxevent, text='Maximun Events')
        lmaxevent.pack()
        self.maxevent_e = Entry(fmaxevent, justify=RIGHT, width=12, disabledbackground='white', disabledforeground='black' )
        self.maxevent_e.insert(0,'10000000')
        self.maxevent_e.pack()
        
        fevent = Frame(fbuttons)
        levent = Label(fevent, text='Event Number')
        levent.pack()
        self.event_e = Entry(fevent, justify=RIGHT, width=12, disabledbackground='white', disabledforeground='black' )
        self.event_e.insert(0,'0')
        self.event_e.config(state=DISABLED)
        self.event_e.pack()

        self.bstart.pack(side=LEFT, padx=5, pady=15)
        self.bstop.pack(side=LEFT, padx=5)
        self.btrigon.pack(side=LEFT, padx=5)
        self.btrigoff.pack(side=LEFT, padx=5)
        hspace1 = Label(fbuttons, width=1)
        hspace1.pack(side=LEFT)
        frunno.pack(side=LEFT, padx=12)
        fevent.pack(side=LEFT, padx=12)
        fmaxevent.pack(side=LEFT, padx=12)

    def __make_status_view(self):
        lstatus = Label(self, anchor='w', text='Nick Name     Node ID    Status     Information')
        lstatus.pack(side=TOP, fill=X)
        lstatus.config(font=('Courier', -12, 'bold'))    
        fsttext = Frame(self)
        fsttext.pack(side=TOP, fill=BOTH, expand=True)

        self.sttext = Text(fsttext, font=('Courier', -12), height=10)
        self.sttext.config(state=DISABLED)
        self.sttext.tag_config('normal', foreground='black')
        self.sttext.tag_config('warning', foreground='yellow')
        self.sttext.tag_config('fatal', foreground='red')
        self.sttext.pack(side=LEFT, fill=BOTH, expand=True)

        sbsttext = Scrollbar(fsttext, command=self.sttext.yview)
        self.sttext.config(yscrollcommand=sbsttext.set)
        sbsttext.pack(side=LEFT, fill=Y)

# Button Command definitions
    def start_command(self):
        self.bstart.config(state=DISABLED)
        self.increment_runno()
        msgh.send_message('run ' + str(self.get_runno()) + '\0')
        msgh.send_message('maxevent ' + str(999999999) + '\0') # for backword compatibility
        msgh.send_message('start\0')
        
        maxevent = self.maxevent_e.get()
        self.set_maxevent(maxevent)

        self.write_run_comment('START ')
        self.set_starttime( time.strftime('%Y %m/%d %H:%M:%S') )
        
        MTMController.mtm_reset()
        self.daq_start_flag = 1
        self.master_controller_flag = 1

    def stop_command(self):
        self.btrigon.config(state=DISABLED)
        self.bstop.config(state=DISABLED)
        self.trigoff_command()
        time.sleep(1)
        msgh.send_message('stop\0')
        self.write_run_comment('STOP  ')
        

    def trigon_command(self):
        self.btrigon.config(fg='black', bg='green', state=DISABLED)
        MTMController.trig_on()
        self.write_run_comment('G_ON  ')
        self.set_trig_state('ON')
        
    def trigoff_command(self):
        self.btrigoff.config(state=DISABLED)
        MTMController.trig_off()
        self.write_run_comment('G_OFF ')
        self.set_trig_state('OFF')
        
    def clean_list_command(self):
        status.cleanup_list()
        self.sttext.config(state=NORMAL)
        self.sttext.delete(1.0, END)
        self.sttext.config(state=DISABLED)
        msgh.send_message('anyone\0')        

# Misc Command definitions
    def write_run_comment(self, command='      '):
        comment = self.comment.get()
        if(command=='      ' and comment=='') : return
        runno = self.get_runno()
        CommentWindow.AddSaveComment( self.comment_file, runno, command+': '+comment)
        self.comment_timestamp = os.stat(self.comment_file).st_mtime
        
    def load_last_comment(self):
        last = CommentWindow.GetLastComment()
        self.comment.delete(0, END)
        self.comment.insert(0, last) 
            
    def get_runno(self):
        f=open(self.runno_file, 'r')
        runno = f.readline()
        f.close()
        return int(runno)
    
    def set_runno(self, runno):
        f=open(self.runno_file, 'w')
        f.write( str(runno) )
        f.close()
        
    def increment_runno(self):
        runno = self.get_runno()
        runno += 1
        self.set_runno(runno)

    def get_maxevent(self):
        f=open(self.maxevent_file, 'r')
        maxevent = f.readline()
        f.close()
        return int(maxevent)
    
    def set_maxevent(self, maxevent):
        f=open(self.maxevent_file, 'w')
        f.write( str(maxevent) )
        f.close()

    def get_trig_state(self):
        f=open(self.trig_file, 'r')
        state = f.readline()
        f.close()
        return state
    
    def set_trig_state(self, state):
        f=open(self.trig_file, 'w')
        f.write( state )
        f.close()

    def get_starttime(self):
        f=open(self.starttime_file, 'r')
        starttime = f.readline()
        f.close()
        return starttime
    
    def set_starttime(self, starttime):
        f=open(self.starttime_file, 'w')
        f.write( starttime )
        f.close()

    def beep_sound(self):
        os.system(sound_command)

# Update Command definitions
    def update_starttime_window(self):
        stat = os.stat(self.starttime_file).st_mtime
        if(self.starttime_timestamp != stat):
            self.starttime_timestamp = stat
            print 'update_startime_window'
            starttime = self.get_starttime()
            self.lasttime.configure(text='Last Run Start Time: ' + starttime)

    def update_comment_window(self):    
        stat = os.stat(self.comment_file).st_mtime
        if(self.comment_timestamp != stat):
            self.comment_timestamp = stat
            print 'update_comment_window'
            f = open(self.comment_file, 'r')
            CommentWindow.ShowComment( f.read() )
            f.close()
            self.load_last_comment()
            
    def update_runno_window(self):
        stat = os.stat(self.runno_file).st_mtime
        if(self.runno_timestamp != stat):
            self.runno_timestamp = stat
            print 'update_runno_window'
            runno = self.get_runno()
            self.runno_e.config(state=NORMAL)
            self.runno_e.delete(0,END)
            self.runno_e.insert(0, str(runno))
            self.runno_e.config(state=DISABLED)

    def update_maxevent_window(self):
        stat = os.stat(self.maxevent_file).st_mtime
        if(self.maxevent_timestamp != stat):
            self.maxevent_timestamp = stat
            print 'update_maxevent_window'
            maxevent = self.get_maxevent()
            self.maxevent_e.delete(0,END)
            self.maxevent_e.insert(0, str(maxevent))            

    def update_trig_status(self):
        stat = os.stat(self.trig_file).st_mtime
        if(self.trig_timestamp != stat):
            self.trig_timestamp = stat
            print 'update_trig_status'
            state = self.get_trig_state()
            if(state == 'ON'):
                self.btrigon.config(fg='black', bg='green', state=DISABLED)
                self.btrigoff.config(state=NORMAL)
            elif(state == 'OFF'):
                self.btrigon.config(fg='black', bg='#d9d9d9', state=NORMAL)
                self.btrigoff.config(state=DISABLED)

    def update_status_window(self):
        self.sttext.config(state=NORMAL)
        self.sttext.delete(1.0, END)
        for i in status.statuslist:
            statusline = status.make_statusline(i)
            sttag = 'normal'
            if i.status in ('NOUPDATE', 'DEAD') : sttag = 'fatal'
            self.sttext.insert(END, statusline, sttag)
        self.sttext.config(state=DISABLED)

    def update_evnum_window(self):
        if( self.daq_state == StatusList.S_RUNNING ):
            self.event_e.config(state=NORMAL)
            self.event_e.delete(0, END)
            self.event_e.insert(0, status.dist_evnum)
            self.event_e.config(state=DISABLED)

    def update_global_state(self):
        gstate = status.global_state
        if( gstate == self.daq_state ) : return
        else : self.daq_state = gstate
        print 'update_global_status'

        # IDLE state
        if  ( self.daq_state == StatusList.S_IDLE ):
            self.label.config(text='DAQ: Idle', fg='blue', bg='black')
            self.bstart.config(state=NORMAL)
            self.bstop.config(state=DISABLED)
            self.btrigon.config(state=DISABLED)
            self.btrigoff.config(state=DISABLED)
            self.maxevent_e.config(state=NORMAL)
            #master flag is changed here for logging messages which come after stop command 
            if( self.master_controller_flag == 1 ) : self.master_controller_flag = 0

        # RUNNING state
        elif( self.daq_state == StatusList.S_RUNNING ):
            if( self.master_controller_flag == 1 ):
                self.label.config(text='DAQ: RUNNING [MASTER]', fg='green', bg='black')
            else:
                self.label.config(text='DAQ: RUNNING [SLAVE]', fg='green', bg='black')
            self.bstart.config(state=DISABLED)
            self.bstop.config(state=NORMAL)
            self.maxevent_e.config(state=DISABLED)

            if( self.daq_start_flag == 1 ):
                self.daq_start_flag = 0
                if (self.auto_trig_on.get() == 1):
                    self.trigon_command()
                else:
                    self.set_trig_state('OFF')
                    self.btrigon.config(state=NORMAL)
            else:
                state = self.get_trig_state()
                if(state == 'ON'):
                    self.btrigon.config(fg='black', bg='green', state=DISABLED)
                    self.btrigoff.config(state=NORMAL)
                elif(state == 'OFF'):
                    self.btrigon.config(fg='black', bg='#d9d9d9', state=NORMAL)
                    self.btrigoff.config(state=DISABLED)
                
        # TRANSITION state
        else :
            self.label.config(text='DAQ: under Transition', fg='yellow', bg='red')
            self.bstart.config(state=DISABLED)
            self.bstop.config(state=DISABLED)
            self.btrigon.config(state=DISABLED)
            self.btrigoff.config(state=DISABLED)
            self.maxevent_e.config(state=DISABLED)
            

    def under_transition_checker(self):
        if not self.daq_state in (StatusList.S_IDLE, StatusList.S_RUNNING):
            if( self.master_controller_flag == 1 ):
                now = time.time()
                diff = now - self.undertransition_timestamp
                if diff > 4 :
                    self.undertransition_timestamp = now
                    thread = threading.Thread(target=self.beep_sound)
                    thread.setDaemon(True)
                    thread.start()
                
                
    def updater(self):
        #Message
        linebuf = msgh.get_message()

        if ( self.master_controller_flag == 1 ):
            msgfile = self.message_dir + '/msglog_run' + str(self.get_runno()).zfill(5) + '.txt'
            MessageWindow.AddSaveMessage(msgfile,linebuf)
        else:
            MessageWindow.AddMessage(linebuf)
        
        #Status
        status.update_list(linebuf)
        self.under_transition_checker()
        self.update_trig_status()
        self.update_status_window()
        self.update_evnum_window()
        self.update_maxevent_window()
        self.update_runno_window()
        self.update_comment_window()
        self.update_starttime_window()
        self.update_global_state()
        if status.check_null_nickname() : msgh.send_message('anyone\0')
        
        #Check event number stop and auto restart flag
        #(only for the master controller that issued start command)
        if( self.daq_state == StatusList.S_RUNNING and self.master_controller_flag == 1 ):
            if ( int(status.dist_evnum) >= int(self.maxevent_e.get()) ):
                self.stop_command()
                if( self.auto_restart.get()==1 ): self.daq_auto_restart_flag=1
                    
        #Auto restart
        if( self.daq_state == StatusList.S_IDLE ):
            if( self.daq_auto_restart_flag==1 ):
                self.daq_auto_restart_flag=0
                self.start_command()

        #500ms repetition
        self.after(500, self.updater)

# End of class Controller(Frame)
    

if __name__ == "__main__":

    argvs = sys.argv
    argc  = len(argvs)

    storage_dir = ''
    
    if argc == 2 :
        storage_dir = argvs[1]
    
    if ( len(storage_dir) == 0) : quit()

    app = Controller(storage_dir)
    msgh = MessageHandler.MessageHandler('127.0.0.1', 8882)

    status = StatusList.StatusList()

    #play sound comman while under stansition state
    #usage:  os.system(sound_command)
    sound_file =  os.path.abspath(os.path.dirname(__file__))+'/sound/under_transition.wav'
    sound_command = 'aplay '+sound_file
    

    app.updater()
    app.mainloop()
    
    
    
