# -*- coding: utf-8 -*-

import os
import hashlib
import humanize

#_______________________________________________________________________________
def get_disk_usage(path):
  st = os.statvfs(os.path.realpath(path))
  free = st.f_frsize * st.f_bavail / 1000000000
  used = st.f_frsize * (st.f_blocks - st.f_bfree) / 1000000000
  total = st.f_frsize * st.f_blocks / 1000000000
  usage = float(used) / total
  return free, used, total, usage
#_______________________________________________________________________________
def get_hash(path, algorithm='sha512'):
  if not os.path.isfile(path):
    return None
  hash = hashlib.new(algorithm)
  with open(path, 'rb') as f:
    while True:
      chunk = f.read(2048 * hash.block_size)
      if len(chunk) == 0:
        break
      hash.update(chunk)
  return hash.hexdigest()
#_______________________________________________________________________________
def compare_file_size(src, dest):
  if os.path.isfile(src) and os.path.isfile(dest):
    if os.path.getsize(src) == os.path.getsize(dest):
      return True
  return False
#_______________________________________________________________________________
def natural_size(arg):
  if type(arg) is str:
    if not os.path.isfile(arg):
      return ''
    size = os.path.getsize(arg)
    return humanize.naturalsize(size).replace('Bytes', ' B')
  if type(arg) is int:
    return humanize.naturalsize(arg).replace('Bytes', ' B')
#_______________________________________________________________________________
def file_size(arg):
  if type(arg) is str:
    if os.path.isfile(arg):
      return os.path.getsize(arg)
  return 0
