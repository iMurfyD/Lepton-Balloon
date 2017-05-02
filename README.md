# Lepton-Balloon
Targeting Raspberry Pi Zero for Horizon Sensor Verification Balloon

Notes:
To disable getty on Serial0 remove console=ttyAMA0,115200 from cmdline.txt fat partition using sd card in external machine
To stop lightdm do: "sudo systemctl set-default multi-user.target"
To restore lightdm do: "sudo systemctl set-default graphical.target"
Also aparently getty is UNKILLABLE JOFISDPOSHGPOS
do "sudo systemctl mask serial-getty@ttyAMA0.service" to pipe it to null.
ensure user is added to video group "sudo usermod -a -G video <username>"
repeated start is required for the melexis so do "echo -n 1 > /sys/module/i2c_bcm2708/parameters/combined"
you will need to be supered for this. (sudo su)
