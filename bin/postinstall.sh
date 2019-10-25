# add user if it does not exist
id -u iip &> /dev/null || useradd iip

# create directory for logging
mkdir -p /var/log/iip

# give permission to iip user
chown -R iip:iip /var/log/iip

# reload systemd
systemctl daemon-reload

# enable dm_forwarder.service
systemctl enable dm_forwarder.service
