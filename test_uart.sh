#!/bin/bash

gcc main.c -o main
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

SESSION="uart_test_session"
PTY1="/tmp/ttyV0"
PTY2="/tmp/ttyV1"

tmux kill-session -t $SESSION 2>/dev/null

tmux new-session -d -s $SESSION -n "socat_bg"
tmux send-keys -t $SESSION:0 "socat PTY,link=$PTY1,raw,echo=0 PTY,link=$PTY2,raw,echo=0" C-m

sleep 1

tmux new-window -t $SESSION -n "testing"

tmux send-keys -t $SESSION:1.0 "cat $PTY2" C-m

tmux split-window -v -t $SESSION:1
tmux send-keys -t $SESSION:1.1 "./main $PTY1" C-m

tmux attach-session -t $SESSION:1
