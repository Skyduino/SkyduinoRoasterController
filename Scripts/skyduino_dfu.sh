#!/usr/bin/expect -f

if {[llength $argv] != 2} {
    send_error "incorrect number of parameters. Expected 2, got [llength $argv]"
    exit 1
}

set DEVICE [lindex $argv 0]
set SPEED [lindex $argv 1]

send_user "Setting '$DEVICE' device speed to $SPEED\n"
exec stty -F $DEVICE $SPEED

send_user "Connecting to '$DEVICE'\n"
spawn -open [open $DEVICE r+]

send "DFU\n"
expect -re {DFU\schallenge\:\s(\d+)} {
    send "DFU;$expect_out(1,string)\n"
}