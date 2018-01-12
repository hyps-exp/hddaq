#!/bin/sh

pkill -f "/bin/sh.*frontend_hrtdc.sh"
if [ $? = 0 ]; then
    echo "# frontend_hrtdc.sh : killed"
else
    echo "# frontend_hrtdc.sh : no process"
fi

pkill -f "frontend_hrtdc($| )"
if [ $? = 0 ]; then
    echo "# frontend_hrtdc    : killed"
else
    echo "# frontend_hrtdc    : no process"
fi

pkill -f "/bin/sh.*message.sh"
if [ $? = 0 ]; then
    echo "# message.sh        : killed"
else
    echo "# message.sh        : no process"
fi

pkill -f "msgd($| )"
if [ $? = 0 ]; then
    echo "# msgd              : killed"
else
    echo "# msgd              : no process"
fi
