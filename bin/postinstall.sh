# add user if it does not exist
id -u iip &> /dev/null || useradd iip

# create directory for logging
mkdir -p /var/log/iip
chown -R iip:iip /var/log/iip

# create directory for work dir
mkdir -p /var/tmp/data
chown -R iip:iip /var/tmp/data

# reload systemd
systemctl daemon-reload

# enable dm_forwarder.service
systemctl enable dm_forwarder.service
