README.md
=========

## Update

- 2017.07.11 --- S.H. Hayakawa
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
    $ ./launcher.py <data directory> (default is "./data")

#### GUI tools

#### kill

    $ ./kill.sh

#### Frontend

    $ cd <hddaq>/Frontend/script

change nickname, nodeid, frontend values in **frontend.sh**

start frontend

    $ ./fe_start.sh

kill frontend

    $ ./fe_kill.sh
