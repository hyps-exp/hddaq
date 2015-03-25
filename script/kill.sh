#!/bin/bash

pkill -f "python.*controller.py"
if [ $? = 0 ]; then
    echo "# controller.py    : killed"
else
    echo "# controller.py    : no process"
fi

pkill "Recorder"
if [ $? = 0 ]; then
    echo "# Recorder         : killed"
else
    echo "# Recorder         : no process"
fi

pkill "EventDistributor*"
if [ $? = 0 ]; then
    echo "# EventDistributor : killed"
else
    echo "# EventDistributor : no process"
fi

pkill "EventBuilder"
if [ $? = 0 ]; then
    echo "# EventBuilder     : killed"
else
    echo "# EventBuilder     : no process"
fi

pkill "cmsgd"
if [ $? = 0 ]; then
    echo "# cmsgd            : killed"
else
    echo "# cmsgd            : no process"
fi

pkill "msgd"
if [ $? = 0 ]; then
    echo "# msgd             : killed"
else
    echo "# msgd             : no process"
fi

pkill -f "python.*launcher.py"
if [ $? = 0 ]; then
    echo "# launcher.py      : killed"
else
    echo "# launcher.py      : no process"
fi
