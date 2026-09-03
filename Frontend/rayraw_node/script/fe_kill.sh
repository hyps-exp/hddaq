#!/bin/sh

pkill -f "/bin/sh.*frontend_rayraw.sh"
if [ $? = 0 ]; then
    echo "# frontend_rayraw.sh : killed"
else
    echo "# frontend_rayraw.sh : no process"
fi

pkill -f "frontend_rayraw($| )"
if [ $? = 0 ]; then
    echo "# frontend_rayraw    : killed"
else
    echo "# frontend_rayraw    : no process"
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
