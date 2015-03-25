import time
import re
import string

import Message

S_IDLE       = 0
S_RUNNING    = 1
S_TRANSITION = 2

DIST_ID = 80000

class Status:
    def __init__(self):
        self.src_id=0
        self.nickname=''
        self.status=''
        self.info=''
        self.lastupdate=0 
        

class StatusList:
    def __init__(self):
        self.statuslist = []
        self.global_state = S_IDLE
        self.dist_evnum = 0
        
    def __cmp_src_id(self, x, y):
        if x.src_id > y.src_id: return 1
        elif x.src_id == y.src_id: return 0
        else: return -1


    def __change_status(self, istatus, now, mbody):
        istatus.lastupdate = now
        sbody = mbody.split()
        if sbody[0] == 'ENTRY' :
            del sbody[0]
            if len(sbody) > 0:
                istatus.nickname = sbody[0]
        elif sbody[0] in ('INITIAL', 'IDLE', 'RUNNING', 'DISORDER', 'END', 'UNKNOWN'):
            istatus.status = sbody[0]
            del sbody[0]
            if len(sbody) > 0:
                istatus.info = string.join(sbody)
            else :
                istatus.info = ''
        
    def __check_update_interval(self):
        now = time.time()
        for i in self.statuslist:
            diff = now - i.lastupdate
            if diff >  6 :  i.status = 'NOUPDATE'
            if diff > 30 :  i.status = 'DEAD'
               
    def __check_global_status(self):
        is_idle    = 1
        is_running = 1
        for i in self.statuslist:
            if not i.status in ('IDLE', 'UNKNOWN')  : is_idle = 0
            if not i.status in ('RUNNING', 'UNKNOWN') : is_running = 0
            
        if  (is_idle==1)    : self.global_state = S_IDLE
        elif(is_running==1) : self.global_state = S_RUNNING
        else                : self.global_state = S_TRANSITION

    def cleanup_list(self):
        self.statuslist = []

    def make_statusline(self, istatus):        
        tmp      = istatus.nickname[0:10] 
        nickname = tmp + ' ' * (10 - len(tmp))
        ssrcid = ' ' * (9 - len(str(istatus.src_id))) + str(istatus.src_id)
        sstatus = '    ' + istatus.status + ' ' * (10 - len(istatus.status))
        
        return string.join([nickname, ssrcid, sstatus, istatus.info, '\n'])
    
    def update_list(self, linebuf):
        for line in linebuf:
            now, ms_type, src_id, body = line
            
            if(ms_type != Message.MT_STATUS) : continue
            
            is_match = 0
            for i in self.statuslist:
                if i.src_id == src_id:
                    self.__change_status(i, now, body)
                    is_match = 1
                    break

            if is_match == 0:
                newentry = Status()
                newentry.src_id = src_id
                newentry.nickname = '--'
                newentry.status = 'UNKNOWN'
                self.__change_status(newentry, now, body) 
                self.statuslist.append(newentry)
                self.statuslist.sort(self.__cmp_src_id)

            if src_id == DIST_ID:  #Event Distributor
                sbody = body.split()
                m = re.findall(r'[0-9]+',sbody[1])          
                self.dist_evnum = m[0]

        self.__check_update_interval()
        self.__check_global_status()

        
    def check_null_nickname(self):
        for i in self.statuslist:
            if (i.nickname == '--'): return True
        return False
