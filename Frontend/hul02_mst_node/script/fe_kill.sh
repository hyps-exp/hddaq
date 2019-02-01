#!/bin/sh

pkill -f "/bin/sh.*frontend_mst.sh"
if [ $? = 0 ]; then
    echo "# frontend_mst.sh : killed"
else
    echo "# frontend_mst.sh : no process"
fi

pkill -f "frontend_mst($| )"
if [ $? = 0 ]; then
    echo "# frontend_mst    : killed"
else
    echo "# frontend_mst    : no process"
fi

pkill -f "/bin/sh.*message.sh"
if [ $? = 0 ]; then
    echo "# message.sh      : killed"
else
    echo "# message.sh      : no process"
fi

pkill -f "msgd($| )"
if [ $? = 0 ]; then
    echo "# msgd            : killed"
else
    echo "# msgd            : no process"
fi
