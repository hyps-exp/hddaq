README.md
=========

## Update

- 2017.03.11 --- S.H. Hayakawa
- 2015.03.25 --- K. Hosomi

## Install

#### HDDAQ

    $ cd hddaq
    $ make

#### Frontend

    $ cd hddaq/Frontend/skeleton_node
    $ make

#### Work directory

    $ mkdir daq
    $ cd daq
    $ <hddaq>/script/makeworkdir.sh

**launcher.py** and **tmplauncher.py** needs data directory

    $ ln -s <data directory> data
    $ ln -s <tmpdata directory> tmpdata

## RUN

#### Main DAQ

    $ cd <work directory>
    $ sh launcher.py <data directory>

#### GUI tools

#### kill

    $ sh kill.sh

#### Frontend

    $ cd <hddaq>/Frontend/script

change nickname, nodeid, frontend values in **frontend.sh**

start frontend

    $ sh fe_start.sh

kill frontend

    $ sh fe_kill.sh
