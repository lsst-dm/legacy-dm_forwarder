# dm_Forwarder
Forwarder application for transferring fitsfiles

## How to set redis password
1. Manually Change Configuration File
   * In your configuration file uncomment the line that says #requirepass foobared
   * change foobared to password of your choosing
   * You can use the redis.conf file included in the root directory of the Redis source code distribution as a template
    to write your configuration file if you do not already have one.
   * In order to start Redis with a configuration file use the full path of the configuration file as first argument, 
   like in the following example: `redis-server /etc/redis.conf`
2. Command Line
   * After starting the redis server (with or without config file) you can set the password at the command line
    with this command: `redis-cli config set requirepass <password>`